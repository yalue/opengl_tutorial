// This defines the main executable implementing the OpenGL tutorial from
// learnopengl.com.
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "model.h"
#include "parse_obj.h"
#include "utilities.h"
#include "opengl_tutorial.h"

// The default window width and height
#define DEFAULT_WINDOW_WIDTH (800)
#define DEFAULT_WINDOW_HEIGHT (600)

ApplicationState* AllocateApplicationState(void) {
  ApplicationState *to_return = NULL;
  to_return = calloc(1, sizeof(*to_return));
  if (!to_return) return NULL;
  to_return->window_width = DEFAULT_WINDOW_WIDTH;
  to_return->window_height = DEFAULT_WINDOW_HEIGHT;
  to_return->aspect_ratio = ((float) to_return->window_width) /
    ((float) to_return->window_height);
  return to_return;
}

void FreeApplicationState(ApplicationState *s) {
  if (!s) return;
  if (s->window) glfwDestroyWindow(s->window);
  glDeleteProgram(s->shader_program);
  DestroyMesh(s->mesh);
  memset(s, 0, sizeof(*s));
  free(s);
}

static void FramebufferResizedCallback(GLFWwindow *window, int width,
    int height) {
  ApplicationState *s = (ApplicationState *) glfwGetWindowUserPointer(window);
  s->window_width = width;
  s->window_height = height;
  s->aspect_ratio = ((float) s->window_width) / ((float) s->window_height);
  glViewport(0, 0, width, height);
}

// Creates the GLFWwindow. Returns 0 on error.
static int SetupWindow(ApplicationState *s) {
  GLFWwindow *window = NULL;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(s->window_width, s->window_height,
    "OpenGL tutorial", NULL, NULL);
  if (!window) {
    printf("Failed creating GLFW window.\n");
    glfwTerminate();
    return 0;
  }
  glfwSetWindowUserPointer(window, s);
  glfwMakeContextCurrent(window);
  s->window = window;
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

// Links the OpenGL shaders. Returns 0 on error.
static int SetupShaderProgram(ApplicationState *s) {
  GLint link_result = 0;
  GLchar link_log[512];
  GLuint vertex_shader, fragment_shader;
  vertex_shader = LoadShader("./shader.vert", GL_VERTEX_SHADER);
  if (!vertex_shader) {
    printf("Couldn't load vertex shader.\n");
    return 0;
  }
  fragment_shader = LoadShader("./shader.frag", GL_FRAGMENT_SHADER);
  if (!fragment_shader) {
    printf("Couldn't load fragment shader.\n");
    return 0;
  }

  s->shader_program = glCreateProgram();
  glAttachShader(s->shader_program, vertex_shader);
  glAttachShader(s->shader_program, fragment_shader);
  glLinkProgram(s->shader_program);

  // The loaded shaders aren't needed after linking.
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  // Check link result
  memset(link_log, 0, sizeof(link_log));
  glGetProgramiv(s->shader_program, GL_LINK_STATUS, &link_result);
  if (link_result != GL_TRUE) {
    glGetProgramInfoLog(s->shader_program, sizeof(link_log) - 1, NULL,
      link_log);
    printf("GL program link error:\n%s\n", link_log);
    return 0;
  }
  glUseProgram(s->shader_program);
  return CheckGLErrors();
}

// Gets the view and projection matrices for the scene.
static void GetViewAndProjection(ApplicationState *s, mat4 view,
    mat4 projection) {
  vec3 view_translate;
  glm_mat4_identity(view);
  glm_mat4_identity(projection);
  glm_vec3_zero(view_translate);
  // Move our viewpoint 3 units "back" from the origin (along the z axis)
  view_translate[2] = -3.0;
  glm_translate(view, view_translate);
  // Create a perspective, with 45 degree FOV
  glm_perspective(45.0, s->aspect_ratio, 0.01, 100.0, projection);
}

// Sets the given matrix to the model's transform matrix.
static void GetModelTransform(ApplicationState *s, mat4 transform) {
  vec3 axis;
  glm_vec3_zero(axis);
  glm_mat4_identity(transform);
  // Rotate over time around the x axis.
  axis[0] = 0.5;
  axis[1] = 1.0;
  glm_rotate(transform, -glfwGetTime(), axis);
}

// Processes window inputs. Returns 0 on error.
static int ProcessInputs(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  return 1;
}

// Sets *index to the index of the named uniform in s->shader_program. Returns
// 0 and prints a message on error.
static int UniformIndex(ApplicationState *s, const char *name, GLint *index) {
  *index = glGetUniformLocation(s->shader_program, name);
  if (*index < 0) {
    printf("Failed getting location of uniform %s.\n", name);
    return 0;
  }
  return 1;
}

// Runs the main window loop. Returns 0 on error.
static int RunMainLoop(ApplicationState *s) {
  mat4 model_transform, view, projection;
  GLint box_tex_uniform, face_tex_uniform, model_uniform, view_uniform,
    projection_uniform;
  int i = 0;
  if (!UniformIndex(s, "box_texture", &box_tex_uniform)) return 0;
  if (!UniformIndex(s, "face_texture", &face_tex_uniform)) return 0;
  if (!UniformIndex(s, "model_transform", &model_uniform)) return 0;
  if (!UniformIndex(s, "view_transform", &view_uniform)) return 0;
  if (!UniformIndex(s, "projection_transform", &projection_uniform)) return 0;
  GetModelTransform(s, model_transform);
  GetViewAndProjection(s, view, projection);
  if (s->mesh->texture_count != 2) {
    printf("Internal error: we must have a mesh with 2 textures for now.\n");
    return 0;
  }

  glUseProgram(s->shader_program);
  // For now, we only need to set the view and projection uniforms once.
  glUniformMatrix4fv(view_uniform, 1, GL_FALSE, (float *) view);
  glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, (float *) projection);

  // We will put the box and face textures in GL_TEXTURE0 and GL_TEXTURE1,
  // respectively.
  glUniform1i(box_tex_uniform, 0);
  glUniform1i(face_tex_uniform, 1);

  // Uncomment to render in wireframe mode.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);

  while (!glfwWindowShouldClose(s->window)) {
    if (!ProcessInputs(s->window)) {
      printf("Error processing inputs.\n");
      return 0;
    }

    glClearColor(0.3f, 0.05f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Start using the shader program
    glUseProgram(s->shader_program);

    // Update the model transform
    GetModelTransform(s, model_transform);
    glUniformMatrix4fv(model_uniform, 1, GL_FALSE, (float *) model_transform);
    // TODO: Update the view and projection uniforms if needed, too.

    // Set the textures.
    for (i = 0; i < s->mesh->texture_count; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
      glBindTexture(GL_TEXTURE_2D, s->mesh->textures[i]);
    }

    // Bind the vertex array and draw.
    glBindVertexArray(s->mesh->vertex_array);
    glDrawElements(GL_TRIANGLES, s->mesh->element_count, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(s->window);
    glfwPollEvents();
    if (!CheckGLErrors()) return 0;
  }
  return 1;
}

// Loads the 3D model to render. Returns 0 on error.
static int Setup3DModel(ApplicationState *s) {
  s->mesh = LoadMesh("pyramid.obj", 2, "container.jpg", "awesomeface.png");
  if (!s->mesh) return 0;
  return 1;
}

int main(int argc, char **argv) {
  int to_return = 0;
  ApplicationState *s = NULL;
  if (!glfwInit()) {
    printf("Failed glfwInit().\n");
    return 1;
  }
  s = AllocateApplicationState();
  if (!s) {
    printf("Failed allocating application state.\n");
    return 1;
  }
  if (!SetupWindow(s)) {
    printf("Failed setting up window.\n");
    FreeApplicationState(s);
    return 1;
  }
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    printf("Failed initializing GLAD.\n");
    to_return = 1;
    goto cleanup;
  }
  glViewport(0, 0, s->window_width, s->window_height);
  glfwSetFramebufferSizeCallback(s->window, FramebufferResizedCallback);
  if (!SetupShaderProgram(s)) {
    to_return = 1;
    goto cleanup;
  }
  if (!Setup3DModel(s)) {
    to_return = 1;
    goto cleanup;
  }
  // TODO (next): Continue with https://learnopengl.com/Getting-started/Coordinate-Systems
  if (!CheckGLErrors()) {
    printf("OpenGL errors detected during initialization.\n");
    to_return = 1;
    goto cleanup;
  }
  if (!RunMainLoop(s)) {
    printf("Application ended with an error.\n");
    to_return = 1;
  } else {
    printf("Everything done OK.\n");
  }
cleanup:
  FreeApplicationState(s);
  glfwTerminate();
  return to_return;
}

