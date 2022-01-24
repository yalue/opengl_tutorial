// This file includes the application state and other necessary definitions for
// following the tutorial at https://learnopengl.com.
#ifndef OPENGL_TUTORIAL_H
#define OPENGL_TUTORIAL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model.h"

// Mimics the layout of the Matrices uniform block.
typedef struct {
  mat4 projection;
  mat4 view;
} MatricesUniformBlock;

// Mimics the layout of the Lighting uniform block.
typedef struct {
  // Use vec4's rather than vec3's for alignment reasons.
  vec4 position;
  vec4 color;
  vec4 ambient_color;
  float ambient_power;
  float pad[3];
} LightingUniformBlock;

// Maintains data indicating how a single mesh should be transformed.
typedef struct {
  // The position (translation) of the mesh relative to the origin.
  vec3 position;
  // The axis about which the mesh rotates.
  vec3 axis;
  // The initial rotation about the axis.
  float start_angle;
  // The speed at which the model spins.
  float rotation_speed;
} MeshTransformConfiguration;

// Overall application state.
typedef struct {
  GLFWwindow *window;
  Mesh *mesh;
  Mesh *floor;
  Mesh *lamp;
  // The number of times to draw the mesh.
  int instance_count;
  // Used to determine the position, rotation, etc, of each instance.
  MeshTransformConfiguration *transforms;
  // The transform matrices. Computed from transforms. One per instance.
  ModelAndNormal *transform_matrices;
  int window_width;
  int window_height;
  float aspect_ratio;
  // Holds the shared ubo for shared transform matrices and lighting.
  GLuint uniform_buffer;
  // The CPU-side copy of the matrices and lighting uniform data.
  MatricesUniformBlock matrices_uniform;
  LightingUniformBlock lighting_uniform;
} ApplicationState;

// Allocates an ApplicationState struct and initializes its values to 0.
// Returns NULL on error. The returned state must be cleaned up using
// FreeApplicationState when no longer needed.
ApplicationState* AllocateApplicationState(void);

// Cleans up and frees the given ApplicationState struct.
void FreeApplicationState(ApplicationState *s);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_H
