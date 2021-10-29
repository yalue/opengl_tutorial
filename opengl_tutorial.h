// This file includes the application state and other necessary definitions for
// following the tutorial at https://learnopengl.com.
#ifndef OPENGL_TUTORIAL_H
#define OPENGL_TUTORIAL_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct {
  GLFWwindow *window;
  GLuint vertex_array_object;
  GLuint vertex_buffer_object;
  GLuint element_buffer_object;
  GLuint shader_program;
  GLuint box_texture;
  GLuint face_texture;
  GLuint elements_to_draw;
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

// Checks OpenGL errors. Prints a message and returns 0 if one was detected.
// Otherwise returns 1.
int CheckGLErrors(void);

// Reads the file with the entire given name to a NULL-terminated buffer of
// bytes. Returns NULL on error. The caller is responsible for freeing the
// returned buffer when it's no longer needed.
uint8_t* ReadFullFile(const char *path);

#ifdef __cplusplus
}  // extern "C"
#endif
#endif  // OPENGL_TUTORIAL_H
