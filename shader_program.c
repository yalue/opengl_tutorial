#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glad/glad.h>
#include "shader_program.h"
#include "utilities.h"

void DestroyShaderProgram(ShaderProgram *p) {
  if (!p) return;
  glDeleteProgram(p->shader_program);
  if (p->texture_count > 0) {
    free(p->texture_uniform_indices);
  }
  memset(p, 0, sizeof(*p));
  free(p);
}

// Sets *index to the index of the named uniform in s->shader_program. Returns
// 0 and prints a message on error.
static int UniformIndex(GLuint program, const char *name, GLint *index) {
  *index = glGetUniformLocation(program, name);
  if (*index < 0) {
    printf("Failed getting location of uniform %s.\n", name);
    return 0;
  }
  return 1;
}

// Loads the indices for the various uniforms in the shader program. Returns 0
// on error.
static int GetUniformIndices(ShaderProgram *p) {
  char uniform_name[128];
  GLuint block_index;
  int i;
  for (i = 0; i < p->texture_count; i++) {
    snprintf(uniform_name, sizeof(uniform_name), "texture%d", i);
    if (!UniformIndex(p->shader_program, uniform_name,
      p->texture_uniform_indices + i)) {
      return 0;
    }
  }
  block_index = glGetUniformBlockIndex(p->shader_program, "SharedUniforms");
  if (block_index == GL_INVALID_INDEX) {
    printf("Failed getting index of shared uniform block.\n");
    CheckGLErrors();
    return 0;
  }
  glUniformBlockBinding(p->shader_program, block_index,
    SHARED_UNIFORMS_BINDING);
  return CheckGLErrors();
}

// Loads and compiles a shader from the given file path. Returns the GLuint
// handle to the shader. Returns 0 on error.
static GLuint LoadShader(const char *path, GLenum shader_type) {
  GLuint to_return = 0;
  GLint compile_result = 0;
  GLchar shader_log[512];
  char *shader_src_orig = NULL;
  char *shared_uniform_code = NULL;
  char *final_src = NULL;

  // First do some preprocessing to insert the common uniform definitions in
  // place of the special comment in the main shader source.
  shader_src_orig = ReadFullFile(path);
  if (!shader_src_orig) return 0;
  shared_uniform_code = ReadFullFile("./shared_uniforms.glsl");
  if (!shared_uniform_code) {
    free(shader_src_orig);
    return 0;
  }
  final_src = StringReplace(shader_src_orig, "//INCLUDE_SHARED_UNIFORMS\n",
    shared_uniform_code);
  if (!final_src) {
    printf("Failed preprocessing shader source code.\n");
    free(shader_src_orig);
    free(shared_uniform_code);
    return 0;
  }
  free(shader_src_orig);
  free(shared_uniform_code);
  shader_src_orig = NULL;
  shared_uniform_code = NULL;

  to_return = glCreateShader(shader_type);
  glShaderSource(to_return, 1, (const char**) &final_src, NULL);
  glCompileShader(to_return);
  free(final_src);
  final_src = NULL;

  // Check compilation success.
  memset(shader_log, 0, sizeof(shader_log));
  glGetShaderiv(to_return, GL_COMPILE_STATUS, &compile_result);
  if (compile_result != GL_TRUE) {
    glGetShaderInfoLog(to_return, sizeof(shader_log) - 1, NULL,
      shader_log);
    printf("Shader %s compile error:\n%s\n", path, shader_log);
    glDeleteShader(to_return);
    return 0;
  }
  if (!CheckGLErrors()) {
    glDeleteShader(to_return);
    return 0;
  }
  return to_return;
}

ShaderProgram* SetupShaderProgram(const char *vertex_src,
    const char *fragment_src, int texture_count) {
  GLchar link_log[512];
  GLint link_result = 0;
  GLuint vertex_shader, fragment_shader, shader_program;
  ShaderProgram *to_return = NULL;

  // The limit of 16 is arbitrary for now.
  if ((texture_count < 0) || (texture_count > 16)) {
    printf("Invalid or unsupported number of shader textures.\n");
    return NULL;
  }

  vertex_shader = LoadShader(vertex_src, GL_VERTEX_SHADER);
  if (!vertex_shader) {
    printf("Couldn't load vertex shader.\n");
    return NULL;
  }
  fragment_shader = LoadShader(fragment_src, GL_FRAGMENT_SHADER);
  if (!fragment_shader) {
    printf("Couldn't load fragment shader.\n");
    glDeleteShader(vertex_shader);
    return NULL;
  }

  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  // The loaded shaders aren't needed after linking.
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Check link result
  memset(link_log, 0, sizeof(link_log));
  glGetProgramiv(shader_program, GL_LINK_STATUS, &link_result);
  if (link_result != GL_TRUE) {
    glGetProgramInfoLog(shader_program, sizeof(link_log) - 1, NULL, link_log);
    printf("GL program link error:\n%s\n", link_log);
    glDeleteProgram(shader_program);
    return NULL;
  }
  glUseProgram(shader_program);
  if (!CheckGLErrors()) {
    glDeleteProgram(shader_program);
    return NULL;
  }
  to_return = (ShaderProgram *) calloc(1, sizeof(*to_return));
  if (!to_return) {
    printf("Failed allocating ShaderProgram struct.\n");
    glDeleteProgram(shader_program);
    return NULL;
  }
  to_return->shader_program = shader_program;
  to_return->texture_count = texture_count;
  to_return->texture_uniform_indices = (GLint *) calloc(texture_count,
      sizeof(GLint));
  if (!to_return->texture_uniform_indices) {
    free(to_return);
    glDeleteProgram(shader_program);
    return NULL;
  }
  if (!GetUniformIndices(to_return)) {
    DestroyShaderProgram(to_return);
    return NULL;
  }
  return to_return;
}
