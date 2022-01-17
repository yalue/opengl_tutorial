#ifndef OPENGL_TUTORIAL_SHADER_PROGRAM_H
#define OPENGL_TUTORIAL_SHADER_PROGRAM_H
#ifdef __cplusplus
extern "C" {
#endif
#include <glad/glad.h>

// The maximum number of textures (named texture0 through textureN) we support
// in a shader.
#define MAX_TEXTURES (16)

// Holds information about a shader program, including some uniform indices.
typedef struct {
  // The actual handle to the shader program.
  GLuint shader_program;
  // Texture uniform indices. Texture uniforms must be named "texture0" through
  // "textureN" in shader source code. Contains texture_count entries.
  GLint *texture_uniform_indices;
  // The number of textures used in the shaders. See the comment on
  // texture_uniform_indices for a note on what the texture uniforms must be
  // named.
  int texture_count;
  // This is the index of the "view_transform" uniform, or -1 if no such
  // uniform exists in source code.
  GLint view_uniform;
  // This is the index of the "projection_transform" uniform, or -1 if no such
  // uniform exists in source code.
  GLint projection_uniform;
} ShaderProgram;

// Takes paths to the shader source files. Allocates and returns a
// ShaderProgram struct, or NULL if any error occurs loading or setting up the
// shader program. The caller is responsible for destroying the returned
// program using DestroyShaderProgram(...) when no longer needed.
ShaderProgram* SetupShaderProgram(const char *vertex_src,
    const char *fragment_src, int texture_count);

// Cleans up any resources associated with the given shader program. The
// pointer p is invalid after calling this function.
void DestroyShaderProgram(ShaderProgram *p);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_SHADER_PROGRAM_H
