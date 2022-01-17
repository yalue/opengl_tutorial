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
  int i;
  for (i = 0; i < p->texture_count; i++) {
    snprintf(uniform_name, sizeof(uniform_name), "texture%d", i);
    if (!UniformIndex(p->shader_program, uniform_name,
      p->texture_uniform_indices + i)) {
      return 0;
    }
  }
  if (!UniformIndex(p->shader_program, "view_transform", &(p->view_uniform))) {
    return 0;
  }
  if (!UniformIndex(p->shader_program, "projection_transform",
    &(p->projection_uniform))) {
    return 0;
  }
  return 1;
}

// Loads and compiles a shader from the given file path. Returns the GLuint
// handle to the shader. Returns 0 on error.
static GLuint LoadShader(const char *path, GLenum shader_type) {
  GLuint to_return = 0;
  GLint compile_result = 0;
  GLchar shader_log[512];
  uint8_t *shader_src = ReadFullFile(path);
  if (!shader_src) return 0;
  to_return = glCreateShader(shader_type);
  glShaderSource(to_return, 1, (const char **) &shader_src, NULL);
  glCompileShader(to_return);
  free(shader_src);
  shader_src = NULL;

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
    glGetProgramInfoLog(shader_program, sizeof(link_log) - 1, NULL,
      link_log);
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
