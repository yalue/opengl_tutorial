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
  DestroyMesh(s->floor);
  DestroyMesh(s->lamp.mesh);
  free(s->transforms);
  free(s->transform_matrices);
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

// Sets the contents of the normal mat3 to the correct normal matrix for the
// given model matrix.
static void ModelToNormalMatrix(mat4 model, mat3 normal) {
  mat4 dst;
  glm_mat4_inv(model, dst);
  glm_mat4_transpose(dst);
  glm_mat4_pick3(dst, normal);
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
  tmp = glfwGetTime() / 4.0;
  // Uncomment to disable the camera movement.
  // tmp = 0;
  position[0] = sin(tmp) * 15.0;
  position[2] = cos(tmp) * 15.0;
  glm_lookat(position, target, up, view);
}

static void UpdateLampPosition(ApplicationState *s) {
  ModelAndNormal transform;
  float tmp;
  s->lamp.position[1] = 2.0;
  tmp = glfwGetTime() / 2.0;
  s->lamp.position[0] = sin(tmp) * 8.0;
  s->lamp.position[2] = cos(tmp) * 8.0;
  // Update the lamp mesh's location
  glm_mat4_identity(transform.model);
  glm_translate(transform.model, s->lamp.position);
  glm_scale_uni(transform.model, 0.5);
  ModelToNormalMatrix(transform.model, transform.normal);
  SetInstanceTransforms(s->lamp.mesh, 1, &transform);
}

// Updates the mesh_transforms matrices.
static void UpdateModelTransforms(ApplicationState *s) {
  int i;
  MeshTransformConfiguration *t = NULL;
  ModelAndNormal *m = NULL;
  float angle;
  for (i = 0; i < s->instance_count; i++) {
    t = s->transforms + i;
    m = s->transform_matrices + i;
    glm_mat4_identity(m->model);
    glm_translate(m->model, t->position);
    angle = t->start_angle - (glfwGetTime() * t->rotation_speed);
    glm_rotate(m->model, angle, t->axis);
    ModelToNormalMatrix(m->model, m->normal);
  }
  // Copy the new data to the instanced vertex buffer.
  SetInstanceTransforms(s->mesh, s->instance_count, s->transform_matrices);
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
  GetViewAndProjection(s, view, projection);
  vec3 ambient_color;
  ambient_color[0] = 1.0;
  ambient_color[1] = 1.0;
  ambient_color[2] = 1.0;


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

    // Move around the lamp
    UpdateLampPosition(s);

    // TODO (next): Implement specular lighting, following the tutorial at
    // https://learnopengl.com/Lighting/Basic-Lighting

    if (!DrawMesh(s->lamp.mesh, (float *) view, (float *) projection, 0.4,
      ambient_color, s->lamp.color, s->lamp.position)) {
      return 0;
    }
    if (!DrawMesh(s->floor, (float *) view, (float *) projection, 0.4,
      ambient_color, s->lamp.color, s->lamp.position)) {
      return 0;
    }
    if (!DrawMesh(s->mesh, (float *) view, (float *) projection, 0.4,
      ambient_color, s->lamp.color, s->lamp.position)) {
      return 0;
    }
    glfwSwapBuffers(s->window);
    glfwPollEvents();
    if (!CheckGLErrors()) return 0;
  }
  return 1;
}

static float RandomFloat(void) {
  return ((float) rand()) / ((float) RAND_MAX);
}

// Loads and initializes the floor plane mesh. Returns 0 on error.
static int SetupFloorPlane(ApplicationState *s) {
  ModelAndNormal floor_transform;

  s->floor = LoadMesh("plane.obj", 1, "floor_texture.png");
  if (!s->floor) {
    printf("Failed loading floor plane mesh.\n");
    return 0;
  }
  if (!SetShaderProgram(s->floor, "basic_vertices.vert",
    "single_texture_shader.frag")) {
    printf("Failed loading floor shaders.\n");
    return 0;
  }

  glm_mat4_identity(floor_transform.model);
  glm_translate_y(floor_transform.model, -5.0);
  // Flip the plane to face upwards.
  glm_rotate_x(floor_transform.model, 3.1415926536, floor_transform.model);
  glm_scale_uni(floor_transform.model, 20.0);
  ModelToNormalMatrix(floor_transform.model, floor_transform.normal);
  if (!SetInstanceTransforms(s->floor, 1, &floor_transform)) {
    printf("Failed setting floor size and position.\n");
    return 0;
  }
  return 1;
}

// Sets up the box meshes, including randomizing their positions, rotations,
// etc. Returns 0 on error.
static int SetupBoxMeshes(ApplicationState *s) {
  int i;
  MeshTransformConfiguration *t = NULL;
  s->mesh = LoadMesh("cube.obj", 2, "container.jpg", "awesomeface.png");
  if (!s->mesh) return 0;
  if (!SetShaderProgram(s->mesh, "basic_vertices.vert",
    "two_texture_shader.frag")) {
    return 0;
  }
  s->instance_count = MODEL_INSTANCES;
  s->transforms = (MeshTransformConfiguration *) calloc(s->instance_count,
    sizeof(MeshTransformConfiguration));
  if (!s->transforms) {
    printf("Failed allocating memory for mesh positions.\n");
    return 0;
  }
  s->transform_matrices = (ModelAndNormal *) calloc(s->instance_count,
    sizeof(ModelAndNormal));
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

// Sets up the light position, color, etc. Returns 0 on error.
static int SetupLamp(ApplicationState *s) {
  ModelAndNormal lamp_transform;
  s->lamp.mesh = LoadMesh("pyramid.obj", 0);
  if (!s->lamp.mesh) {
    printf("Failed loading lamp mesh.\n");
    return 0;
  }
  if (!SetShaderProgram(s->lamp.mesh, "basic_vertices.vert",
    "lamp_object_shader.frag")) {
    printf("Failed loading lamp shaders.\n");
    return 0;
  }

  // Put the lamp at 0, 3, 6, make it small.
  s->lamp.position[0] = 0;
  s->lamp.position[1] = 3.0;
  s->lamp.position[2] = 6.0;
  glm_vec3_one(s->lamp.color);
  glm_mat4_identity(lamp_transform.model);
  glm_translate(lamp_transform.model, s->lamp.position);
  glm_scale_uni(lamp_transform.model, 0.5);
  ModelToNormalMatrix(lamp_transform.model, lamp_transform.normal);
  if (!SetInstanceTransforms(s->lamp.mesh, 1, &lamp_transform)) {
    printf("Failed setting lamp size and position.\n");
    return 0;
  }
  return 1;
}

// Loads the 3D models to render. Returns 0 on error.
static int Setup3DModels(ApplicationState *s) {
  if (!SetupFloorPlane(s)) return 0;
  if (!SetupBoxMeshes(s)) return 0;
  if (!SetupLamp(s)) return 0;
  return 1;
}

int main(int argc, char **argv) {
  int to_return = 0;
  ApplicationState *s = NULL;
  printf("Size of ModelAndNormal: %d\n", (int) sizeof(ModelAndNormal));
  printf("Offset of model: %d\n", (int) offsetof(ModelAndNormal, model));
  printf("Offset of normal: %d\n", (int) offsetof(ModelAndNormal, normal));
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

