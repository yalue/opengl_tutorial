#ifndef OPENGL_TUTORIAL_MODEL_H
#define OPENGL_TUTORIAL_MODEL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdarg.h>
#include <glad/glad.h>
#include "shader_program.h"

// Holds a single 3D model along with its associated textures. The contents of
// this struct should not be modified by the user.
typedef struct {
  // Contains the OpenGL textures associated with this mesh.
  GLuint *textures;
  // The number of textures associated with the mesh.
  int texture_count;
  // The shader used to draw this mesh.
  ShaderProgram *shader_program;
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

// Sets up the shader program used by this mesh. Requires paths to the vertex
// and fragment shader sources. See shader_program.h for some notes about this;
// some uniforms are expected to follow certain naming conventions. This
// function returns 0 on error.
int SetShaderProgram(Mesh *m, const char *vert_src, const char *frag_src);

// Frees any resources associated with the mesh, along with the mesh struct
// itself. The mesh pointer is invalid after passing it to this.
void DestroyMesh(Mesh *mesh);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_MODEL_H

