// This file includes the application state and other necessary definitions for
// following the tutorial at https://learnopengl.com.
#ifndef OPENGL_TUTORIAL_H
#define OPENGL_TUTORIAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow *window;
  GLuint vertex_buffer_object;
} ApplicationState;

// Allocates an ApplicationState struct and initializes its values to 0.
// Returns NULL on error. The returned state must be cleaned up using
// FreeApplicationState when no longer needed.
ApplicationState* AllocateApplicationState(void);

// Cleans up and frees the given ApplicationState struct.
void FreeApplicationState(ApplicationState *s);

// Checks OpenGL errors. Prints a message and returns 0 if one was detected.
// Otherwise returns 1.
int CheckGLErrors(void);

// Sets up the GLFW window. Returns 0 on error.
int SetupWindow(ApplicationState *s);

// Sets up our vertex buffer object. Returns 0 on error.
int SetupVertexBuffer(ApplicationState *s);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_H
