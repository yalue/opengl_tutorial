#ifndef OPENGL_TUTORIAL_MODEL_H
#define OPENGL_TUTORIAL_MODEL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>
#include <glad/glad.h>

// Holds a single 3D model along with its associated textures. The contents of
// this struct should not be modified by the user.
typedef struct {
  // Contains the OpenGL textures associated with this mesh.
  GLuint *textures;
  // The number of textures associated with the mesh.
  int texture_count;
  // Used for rendering the object.
  GLuint vertex_array;
  GLuint vertex_buffer;
  GLuint instanced_vertex_buffer;
  GLuint element_buffer;
  GLuint element_count;
} Mesh;

// Creates a mesh from the given object file. Also takes the number of textures
// and the corresponding number of paths to texture images. Returns NULL on
// error. The returned mesh must be passed to DestroyMesh when no longer
// needed.
Mesh* LoadMesh(const char *object_file_path, int texture_count, ...);

// Frees any resources associated with the mesh, along with the mesh struct
// itself. The mesh pointer is invalid after passing it to this.
void DestroyMesh(Mesh *mesh);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_MODEL_H

