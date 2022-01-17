// This defines the main executable implementing the OpenGL tutorial from
// learnopengl.com.
#include <errno.h>
#include <math.h>
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

// The number of instances of the model to render.
#define MODEL_INSTANCES (100)

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
  DestroyMesh(s->mesh);
  free(s->transforms);
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

// Updates the view matrix.
static void UpdateView(ApplicationState *s, mat4 view) {
  vec3 position, target, up;
  float tmp;
  glm_mat4_identity(view);
  glm_vec3_zero(position);
  glm_vec3_zero(target);
  glm_vec3_zero(up);
  up[1] = 1.0;
  tmp = glfwGetTime() / 2.0;
  position[0] = sin(tmp) * 10.0;
  position[2] = cos(tmp) * 10.0;
  glm_lookat(position, target, up, view);
}

// Updates the mesh_transforms matrices.
static void UpdateModelTransforms(ApplicationState *s) {
  int i;
  MeshTransformConfiguration *t = NULL;
  float angle;
  for (i = 0; i < s->instance_count; i++) {
    t = s->transforms + i;
    glm_mat4_identity(s->transform_matrices[i]);
    glm_translate(s->transform_matrices[i], t->position);
    angle = t->start_angle - (glfwGetTime() * t->rotation_speed);
    glm_rotate(s->transform_matrices[i], angle, t->axis);
  }
  // Copy the new data to the instanced vertex buffer.
  glBindBuffer(GL_ARRAY_BUFFER, s->mesh->instanced_vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, s->instance_count * sizeof(mat4),
    s->transform_matrices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// Processes window inputs. Returns 0 on error.
static int ProcessInputs(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, 1);
  }
  return 1;
}

// Runs the main window loop. Returns 0 on error.
static int RunMainLoop(ApplicationState *s) {
  mat4 view, projection;
  GLuint view_uniform;
  int i = 0;
  GetViewAndProjection(s, view, projection);

  glUseProgram(s->mesh->shader_program->shader_program);
  view_uniform = s->mesh->shader_program->view_uniform;
  // For now, we only need to set the projection uniform once.
  glUniformMatrix4fv(view_uniform, 1, GL_FALSE, (float *) view);
  glUniformMatrix4fv(s->mesh->shader_program->projection_uniform, 1, GL_FALSE,
    (float *) projection);

  // Put the textures in their respective slots.
  for (i = 0; i < s->mesh->texture_count; i++) {
    glUniform1i(s->mesh->shader_program->texture_uniform_indices[i], i);
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, s->mesh->textures[i]);
  }

  // Uncomment to render in wireframe mode.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  while (!glfwWindowShouldClose(s->window)) {
    if (!ProcessInputs(s->window)) {
      printf("Error processing inputs.\n");
      return 0;
    }

    glClearColor(0.3f, 0.05f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Update the model transforms
    UpdateModelTransforms(s);

    // Update the camera position
    UpdateView(s, view);
    glUniformMatrix4fv(view_uniform, 1, GL_FALSE, (float *) view);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(s->mesh->vertex_array);
    glDrawElementsInstanced(GL_TRIANGLES, s->mesh->element_count,
      GL_UNSIGNED_INT, 0, s->instance_count);

    glfwSwapBuffers(s->window);
    glfwPollEvents();
    if (!CheckGLErrors()) return 0;
  }
  return 1;
}

static float RandomFloat(void) {
  return ((float) rand()) / ((float) RAND_MAX);
}

// Loads the 3D model to render. Returns 0 on error. Also determines the mesh
// positions.
static int Setup3DModels(ApplicationState *s) {
  int i;
  MeshTransformConfiguration *t = NULL;
  s->mesh = LoadMesh("cube.obj", 2, "container.jpg", "awesomeface.png");
  if (!s->mesh) return 0;
  if (!SetShaderProgram(s->mesh, "shader.vert", "shader.frag")) return 0;
  s->instance_count = MODEL_INSTANCES;
  s->transforms = (MeshTransformConfiguration *) calloc(s->instance_count,
    sizeof(MeshTransformConfiguration));
  if (!s->transforms) {
    printf("Failed allocating memory for mesh positions.\n");
    return 0;
  }
  s->transform_matrices = (mat4 *) calloc(s->instance_count, sizeof(mat4));
  if (!s->transform_matrices) {
    printf("Failed allocating memory for model transform matrices.\n");
    return 0;
  }
  for (i = 0; i < s->instance_count; i++) {
    t = s->transforms + i;
    // x position: -8 to +8
    t->position[0] = RandomFloat() * 16.0 - 8.0;
    // y position: -4 to +4
    t->position[1] = RandomFloat() * 8.0 - 4.0;
    // z position: -4 to +4
    t->position[2] = RandomFloat() * 8.0 - 4.0;
    // Rotate about a random axis
    t->axis[0] = RandomFloat() * 2.0 - 1.0;
    t->axis[1] = RandomFloat() * 2.0 - 1.0;
    t->axis[2] = RandomFloat() * 2.0 - 1.0;
    t->start_angle = RandomFloat() * 2.0 * 3.1415926535;
    t->rotation_speed = RandomFloat() * 3.0;
  }
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
  if (!Setup3DModels(s)) {
    to_return = 1;
    goto cleanup;
  }
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

