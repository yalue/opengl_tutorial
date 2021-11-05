// This file includes the application state and other necessary definitions for
// following the tutorial at https://learnopengl.com.
#ifndef OPENGL_TUTORIAL_H
#define OPENGL_TUTORIAL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model.h"

typedef struct {
  GLFWwindow *window;
  GLuint shader_program;
  Mesh *mesh;
  int window_width;
  int window_height;
  float aspect_ratio;
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
