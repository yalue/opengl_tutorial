#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_obj.h"

typedef struct {
  // Contains each vertex's location coordinate, as specified by "v" lines in
  // the file. Contains 3 * location_count floats.
  float *locations;
  uint32_t location_count;
  // The number of locations we've already seen, if we're parsing the file.
  uint32_t locations_read;
  // Contains each normal vector, as specified by "vn" lines in the file.
  // Contains 3 * normal_count floats.
  float *normals;
  uint32_t normal_count;
  // The number of normals we've already seen, if we're parsing the file.
  uint32_t normals_read;
  // Contains each UV coordinate, as specified by the "vt" lines in the file.
  // Contains 2 * uv_coord_count floats.
  float *uv_coords;
  uint32_t uv_coord_count;
  // The number of UV coords we've already seen, if we're parsing the file.
  uint32_t uv_coords_read;
  // Contains each vertex index in the file. Each index contains three ints:
  // specifying the location, UV coordinate, and normal in their respective
  // arrays. So this contains index_count * 3 uint32_t values. Additionally,
  // the index_count must be divisible by 3, since we're expecting triangles.
  uint32_t *indices;
  uint32_t index_count;
  // The number of indices we've already seen, if we're parsing the file.
  uint32_t indices_read;
} InternalObjectFile;

// Returns the pointer to the next character after in that is non-whitespace.
// This does *not* count newlines as whitespace.
static const char* SkipSpaces(const char *in) {
  while ((*in == ' ') || (*in == '\t')) in++;
  return in;
}

// Returns the number of indices on a line starting with "f ". Returns a
// negative value on error.
static int CountIndicesOnLine(const char *l) {
  int to_return;
  to_return = 0;
  char c;

  // We basically count the number of indices here by scanning the number of
  // space-separated tokens.
  while (*l) {
    c = *l;
    if (c == '\n') break;
    if ((c == ' ') || (c == '\t')) {
      to_return++;
      l = SkipSpaces(l);
      continue;
    }
    // We're not at a newline or a whitespace.
    l++;
  }
  return to_return;
}

// Returns 1 if string a starts with string b.
static int StartsWith(const char *a, const char *b) {
  if (strstr(a, b) == a) return 1;
  return 0;
}

// Takes a string and skips past the following newline. If a null character is
// encountered before the next newline, stops at the null instead.
static const char* SkipLine(const char *in) {
  // Move to the next newline character.
  while (*in != '\n') {
    // Stop early if we encounter the null character.
    if (*in == 0) return in;
    in++;
  }
  // Skip the newline.
  in++;
  return in;
}

// Counts the number of vertices, normals, texture coordinates, and indices in
// the file so we can allocate the arrays to hold them. Returns 0 on error,
// including if the file contains too many objects or the file uses
// non-triangular faces.
static int CountVerticesAndIndices(const char *content,
    InternalObjectFile *o) {
  int object_count = 0;
  char c = 0;
  o->location_count = 0;
  o->normal_count = 0;
  o->uv_coord_count = 0;
  o->index_count = 0;

  while (*content) {
    // Skip any leading whitespace.
    content = SkipSpaces(content);
    c = *content;
    // Skip blank lines.
    if (c == '\n') {
      content++;
      continue;
    }
    // Skip comments.
    if (c == '#') {
      content = SkipLine(content);
      continue;
    }
    // We're looking at a new object.
    if (StartsWith(content, "o ")) {
      object_count++;
      if (object_count > 1) {
        printf("The obj file contains too many objects.\n");
        return 0;
      }
      content = SkipLine(content);
      continue;
    }
    if (StartsWith(content, "v ")) {
      o->location_count++;
      content = SkipLine(content);
      continue;
    }
    if (StartsWith(content, "vt ")) {
      o->uv_coord_count++;
      content = SkipLine(content);
      continue;
    }
    if (StartsWith(content, "vn ")) {
      o->normal_count++;
      content = SkipLine(content);
      continue;
    }
    if (StartsWith(content, "f ")) {
      if (CountIndicesOnLine(content) != 3) {
        printf("Found a non-triangular face.\n");
        return 0;
      }
      o->index_count += 3;
      content = SkipLine(content);
      continue;
    }
    // Fallback for unrecognized line. Skip it.
    content = SkipLine(content);
  }
  return 1;
}

// Call this after counting vertices, etc, to allocate the buffers in o. If any
// allocation fails, this returns 0 and o is unchanged. Buffers are filled with
// zero.
static int AllocateTemporaryBuffers(InternalObjectFile *o) {
  float *locations = NULL, *normals = NULL, *uv_coords = NULL;
  uint32_t *indices = NULL;
  locations = (float *) calloc(sizeof(float), 3 * o->location_count);
  if (!locations) return 0;
  normals = (float *) calloc(sizeof(float), 3 * o->normal_count);
  if (!normals) {
    free(locations);
    return 0;
  }
  uv_coords = (float *) calloc(sizeof(float), 2 * o->uv_coord_count);
  if (!uv_coords) {
    free(locations);
    free(normals);
    return 0;
  }
  indices = (uint32_t *) calloc(sizeof(uint32_t), 3 * o->index_count);
  if (!indices) {
    free(locations);
    free(normals);
    free(uv_coords);
    return 0;
  }
  o->locations = locations;
  o->normals = normals;
  o->uv_coords = uv_coords;
  o->indices = indices;
  return 1;
}

// Frees any memory associated with the given InternalObjectFile.
static void CleanupInternalObjectFile(InternalObjectFile *o) {
  free(o->locations);
  free(o->normals);
  free(o->uv_coords);
  free(o->indices);
  memset(o, 0, sizeof(*o));
}

// Reads a single floating-point value from s, writing it to *out. Returns a
// pointer to the first char after the float. Returns NULL on error. Skips
// any leading whitespace in s, apart from newlines.
static const char* ParseNextFloat(const char *s, float *out) {
  char *end = NULL;
  float result;
  s = SkipSpaces(s);
  result = strtof(s, &end);
  if (end == s) {
    printf("String doesn't start with a float.\n");
    return NULL;
  }
  *out = result;
  return end;
}

// Reads n consecutive whitespace-separated floats on the given line. The
// floats must be separated by whitespace, and the line may start with
// whitespace. The out array must have enough capacity for n floats. Returns 0
// on error.
static int ParseFloats(int n, const char *line, float *out) {
  int i;
  float tmp;
  for (i = 0; i < n; i++) {
    line = ParseNextFloat(line, &tmp);
    if (!line) {
      printf("Failed parsing float %d/%d on line.\n", i + 1, n);
      return 0;
    }
    out[i] = tmp;
  }
  return 1;
}

// Reads the next 3 '/'-separated indices from s. s may have leading
// whitepsace. out must have space to store the 3 indices. Returns the first
// character after the third index. Returns NULL on error.
static const char* ParseNext3Indices(const char *s, uint32_t *out) {
  int i;
  unsigned long tmp;
  char *end = NULL;
  s = SkipSpaces(s);
  for (i = 0; i < 3; i++) {
    tmp = strtoul(s, &end, 10);
    if (s == end) {
      printf("String doesn't start with a number.\n");
      return NULL;
    }
    // Advance past the number.
    s = end;
    if ((i < 2) && (*s != '/')) {
      printf("Didn't find 3 indices separated by '/'.\n");
      return NULL;
    }
    out[i] = tmp;
    // Skip the '/' for the first two numbers.
    if (i < 2) s++;
  }
  return s;
}

// Reads 9 indices in a single face: 3 space-separated sets of 3 '/'-separated
// indices. Returns 0 on error. The out buffer must have space for 9 indces.
// The line may have leading whitespace.
static int ParseFace(const char *line, uint32_t *out) {
  int i;
  uint32_t indices[3];
  for (i = 0; i < 3; i++) {
    line = ParseNext3Indices(line, indices);
    if (line == NULL) {
      printf("Failed parsing 3 indices group %d/3.\n", i);
      return 0;
    }
    memcpy(out + (3 * i), indices, sizeof(indices));
  }
  return 1;
}

// Parses a single line in the object file, returning a pointer to the start
// of the next line, or NULL on error. Updates the content of o with the
// line's content.
static const char* ParseLine(const char *line, InternalObjectFile *o) {
  float parsed_floats[3];
  uint32_t parsed_indices[9];
  line = SkipSpaces(line);

  // Skip comments.
  if (StartsWith(line, "#")) return SkipLine(line);

  // Parse a vertex location.
  if (StartsWith(line, "v ")) {
    line += 1;
    if (!ParseFloats(3, line, parsed_floats)) {
      printf("Failed parsing vertex location.\n");
      return NULL;
    }
    if (o->locations_read >= o->location_count) {
      printf("Internal error: read more than expected number of locations.\n");
      return NULL;
    }
    memcpy(o->locations + (3 * o->locations_read), parsed_floats,
      3 * sizeof(float));
    o->locations_read++;
    return SkipLine(line);
  }

  // Parse a normal.
  if (StartsWith(line, "vn ")) {
    line += 2;
    if (!ParseFloats(3, line, parsed_floats)) {
      printf("Failed parsing normal.\n");
      return NULL;
    }
    if (o->normals_read >= o->normal_count) {
      printf("Internal error: read more than expected number of normals.\n");
      return NULL;
    }
    memcpy(o->normals + (3 * o->normals_read), parsed_floats,
      3 * sizeof(float));
    o->normals_read++;
    return SkipLine(line);
  }

  if (StartsWith(line, "vt ")) {
    line += 2;
    if (!ParseFloats(2, line, parsed_floats)) {
      printf("Failed parsing UV coords.\n");
      return NULL;
    }
    if (o->uv_coords_read >= o->uv_coord_count) {
      printf("Internal error: read more than expected number of UV coords.\n");
      return NULL;
    }
    memcpy(o->uv_coords + (2 * o->uv_coords_read), parsed_floats,
      2 * sizeof(float));
    o->uv_coords_read++;
    return SkipLine(line);
  }

  if (StartsWith(line, "f ")) {
    line += 1;
    if (!ParseFace(line, parsed_indices)) {
      printf("Failed parsing face coord indices.\n");
      return NULL;
    }
    if (o->indices_read >= o->index_count) {
      printf("Internal error: read more than expected number of indices.\n");
      return NULL;
    }
    memcpy(o->indices + (3 * o->indices_read), parsed_indices,
      9 * sizeof(uint32_t));
    o->indices_read += 3;
    return SkipLine(line);
  }

  // Skip any unknown lines.
  return SkipLine(line);
}

// Populates the buffers in o with the content from the object file. Expects
// the buffers to have been allocated already.
static int ParseInternalObjectFile(const char *content,
    InternalObjectFile *o) {
  const char *current = content;
  while (*current) {
    current = ParseLine(current, o);
    if (!current) {
      printf("Error parsing obj file line.\n");
      return 0;
    }
  }
  return 1;
}

// Converts the data collected in o to the format in the ObjectFileInfo struct.
// Returns 0 on error.
static int ConvertInternalObjectFile(InternalObjectFile *o,
    ObjectFileInfo *out) {
  printf("Object file info:\n");
  printf("  # of vertex locations: %d\n", (int) o->location_count);
  printf("  # of normals: %d\n", (int) o->normal_count);
  printf("  # of UV coordinates: %d\n", (int) o->uv_coord_count);
  printf("  # of indices: %d\n", (int) o->index_count);

  // Do some sanity checks to make sure the second pass was OK.
  if (o->location_count != o->locations_read) {
    printf("Expected to parse %d locations, got %d.\n",
      (int) o->location_count, (int) o->locations_read);
    return 0;
  }
  if (o->normal_count != o->normals_read) {
    printf("Expected to parse %d normals, got %d.\n",
      (int) o->normal_count, (int) o->normals_read);
    return 0;
  }
  if (o->uv_coord_count != o->uv_coords_read) {
    printf("Exected to parse %d UV coords, got %d.\n",
      (int) o->uv_coord_count, (int) o->uv_coords_read);
    return 0;
  }
  if (o->index_count != o->indices_read) {
    printf("Expected to parse %d indices, got %d.\n", (int) o->index_count,
      (int) o->indices_read);
    return 0;
  }
  // TODO (next): Implement ConvertInternalObjectFile.
  printf("Converting object file not implemented yet.\n");
  return 0;
}

ObjectFileInfo* ParseObjFile(const char *content) {
  InternalObjectFile o;
  ObjectFileInfo *to_return = NULL;
  if (sizeof(ObjectFileVertex) != (sizeof(float) * 8)) {
    printf("Internal error: expected exactly 8 floats per vertex struct.\n");
    return NULL;
  }
  if (!content) {
    printf("Got NULL in place of .obj file content.\n");
    return NULL;
  }
  memset(&o, 0, sizeof(o));
  if (!CountVerticesAndIndices(content, &o)) {
    printf("Failed initial pass over object file.\n");
    return NULL;
  }
  if (!AllocateTemporaryBuffers(&o)) {
    printf("Failed allocating temporary buffer to hold obj content.\n");
    return NULL;
  }
  if (!ParseInternalObjectFile(content, &o)) {
    printf("Failed parsing object file content.\n");
    CleanupInternalObjectFile(&o);
    return NULL;
  }
  to_return = (ObjectFileInfo *) calloc(sizeof(ObjectFileInfo), 1);
  if (!to_return) {
    printf("Failed allocating object file.\n");
    CleanupInternalObjectFile(&o);
    return NULL;
  }
  if (!ConvertInternalObjectFile(&o, to_return)) {
    printf("Failed generating ObjectFileInfo struct.\n");
    CleanupInternalObjectFile(&o);
    free(to_return);
    return NULL;
  }
  CleanupInternalObjectFile(&o);
  return to_return;
}

void FreeObjectFileInfo(ObjectFileInfo *o) {
  free(o->vertices);
  free(o->indices);
  memset(o, 0, sizeof(*o));
  free(o);
}
