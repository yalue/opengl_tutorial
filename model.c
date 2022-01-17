#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include "parse_obj.h"
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader_program.h"
#include "utilities.h"

#include "model.h"

int SetShaderProgram(Mesh *m, const char *vert_src, const char *frag_src) {
  if (m->shader_program) {
    printf("The mesh already had a shader program. This one must be destroyed "
      "before setting a new one.\n");
    return 0;
  }
  m->shader_program = SetupShaderProgram(vert_src, frag_src, m->texture_count);
  if (!m->shader_program) {
    printf("Failed loading shader program.\n");
    return 0;
  }
  return 1;
}

// Loads a texture, returning the ID of the new OpenGL texture. Returns 0 on
// error. If this returns nonzero, then the texture should be destroyed by the
// caller.
static GLuint LoadTexture(const char *filename) {
  GLuint to_return = 0;
  int width, height, channels;
  unsigned char *image_data = stbi_load(filename, &width, &height, &channels,
    4);
  if (!image_data) {
    printf("Failed loading image %s\n", filename);
    return 0;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenTextures(1, &to_return);
  glBindTexture(GL_TEXTURE_2D, to_return);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
    GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(image_data);
  image_data = NULL;
  if (!CheckGLErrors()) {
    printf("Couldn't create texture from %s\n", filename);
    glDeleteTextures(1, &to_return);
    return 0;
  }
  return to_return;
}

Mesh* LoadMesh(const char *object_file_path, int texture_count, ...) {
  va_list args;
  ObjectFileInfo *object = NULL;
  GLuint *textures = NULL;
  GLuint vao = 0, vbo = 0, ebo = 0, instanced_vbo = 0;
  const char *image_path = NULL;
  char *object_file_content = NULL;
  int i = 0;
  Mesh *to_return = NULL;
  object_file_content = (char *) ReadFullFile(object_file_path);
  if (!object_file_content) {
    printf("Failed object file from %s\n", object_file_path);
    return NULL;
  }
  object = ParseObjFile(object_file_content);
  free(object_file_content);
  object_file_content = NULL;
  if (!object) {
    printf("Failed parsing object file %s\n", object_file_path);
    return NULL;
  }
  // Next try loading the textures.
  textures = (GLuint *) calloc(texture_count, sizeof(GLuint));
  if (!textures) {
    printf("Failed allocating textures handle buffer.\n");
    FreeObjectFileInfo(object);
    return NULL;
  }
  va_start(args, texture_count);
  for (i = 0; i < texture_count; i++) {
    image_path = va_arg(args, const char *);
    textures[i] = LoadTexture(image_path);
    if (!textures[i]) goto error_cleanup;
  }
  va_end(args);
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  // Set up the element buffer.
  glGenBuffers(1, &ebo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, object->index_count * sizeof(GLuint),
    object->indices, GL_STATIC_DRAW);
  // Set up the vertex buffer.
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, object->vertex_count *
    sizeof(ObjectFileVertex), object->vertices, GL_STATIC_DRAW);
  // Set up the instanced transform buffer.
  glGenBuffers(1, &instanced_vbo);

  if (!CheckGLErrors()) {
    printf("Failed setting up element and vertex buffers.\n");
    goto error_cleanup;
  }
  to_return = (Mesh *) calloc(1, sizeof(Mesh));
  if (!to_return) {
    printf("Failed allocating mesh struct.\n");
    goto error_cleanup;
  }

  // Set up the position, normal, and texture coordinate attributes.
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectFileVertex),
    (void *) offsetof(ObjectFileVertex, location));
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ObjectFileVertex),
    (void *) offsetof(ObjectFileVertex, normal));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ObjectFileVertex),
    (void *) offsetof(ObjectFileVertex, uv));
  glEnableVertexAttribArray(2);
  if (!CheckGLErrors()) {
    printf("Error setting up mesh attributes.\n");
    goto error_cleanup;
  }

  // Set up the instanced vertex buffer. A mat4 uses four attribute locations.
  glBindBuffer(GL_ARRAY_BUFFER, instanced_vbo);
  for (i = 0; i < 4; i++) {
    glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, 16 * sizeof(float),
      (void *) (i * 4 * sizeof(float)));
    glEnableVertexAttribArray(3 + i);
    glVertexAttribDivisor(3 + i, 1);
  }
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  if (!CheckGLErrors()) {
    printf("Error setting up instanced vertex buffer.\n");
    goto error_cleanup;
  }

  to_return->textures = textures;
  to_return->texture_count = texture_count;
  to_return->vertex_array = vao;
  to_return->vertex_buffer = vbo;
  to_return->instanced_vertex_buffer = instanced_vbo;
  to_return->element_buffer = ebo;
  to_return->element_count = object->index_count;
  FreeObjectFileInfo(object);
  return to_return;

error_cleanup:
  glDeleteVertexArrays(1, &vao);
  glDeleteBuffers(1, &ebo);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &instanced_vbo);
  free(to_return);
  if (textures) {
    glDeleteTextures(texture_count, textures);
    free(textures);
  }
  FreeObjectFileInfo(object);
  return NULL;
}

void DestroyMesh(Mesh *mesh) {
  if (!mesh) return;
  glDeleteTextures(mesh->texture_count, mesh->textures);
  free(mesh->textures);
  glDeleteBuffers(1, &(mesh->element_buffer));
  glDeleteBuffers(1, &(mesh->vertex_buffer));
  glDeleteBuffers(1, &(mesh->instanced_vertex_buffer));
  glDeleteVertexArrays(1, &(mesh->vertex_array));
  memset(mesh, 0, sizeof(*mesh));
  free(mesh);
}
