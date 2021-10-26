// Defines a very simple library for parsing Wavefront .obj 3D files. This only
// supports files containing triangulated faces and a single object.

#ifndef PARSE_OBJ_H
#define PARSE_OBJ_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

// Holds a single vertex, keeping track of the location, normal, and UV
// coordinates, respectively. Must contain exactly 8 floats.
typedef union {
  struct {
    float location[3];
    float normal[3];
    float uv[2];
  };
  float data[8];
} ObjectFileVertex;

// Holds information from a parsed object file. Allocated and initialized by
// the ParseObjFile function.
typedef struct {
  // A list of vertices in the object file. Merges normals, UV coords, etc,
  // into new vertices, even if vertices have the same location.
  ObjectFileVertex *vertices;
  uint32_t vertex_count;
  // Holds the list of indices to form the triangles.
  uint32_t *indices;
  // The number of indices. Will always be divisible by 3.
  uint32_t index_count;
} ObjectFileInfo;

// Parses an object file. Only ever returns the first object in the file.
// The object must only have triangular faces. Requires the full file content
// followed by a null character. Allocates and returns an ObjectFileInfo struct
// with the data. The returned struct must be freed by the caller, using
// FreeObjectFileInfo, when no longer needed.
ObjectFileInfo* ParseObjFile(const char *file_content);

// Frees any memory used by the given ObjectFileInfo struct along with the
// struct itself. (The pointer will be invalid after calling this.)
void FreeObjectFileInfo(ObjectFileInfo *o);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // PARSE_OBJ_H
