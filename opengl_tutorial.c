// This defines the main executable implementing the OpenGL tutorial from
// learnopengl.com.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "opengl_tutorial.h"

ApplicationState* AllocateApplicationState(void) {
  ApplicationState *to_return = NULL;
  to_return = calloc(1, sizeof(*to_return));
  if (!to_return) return NULL;
  return to_return;
}

void FreeApplicationState(ApplicationState *s) {
  if (!s) return;
  if (s->window) glfwDestroyWindow(s->window);
  memset(s, 0, sizeof(*s));
  free(s);
}

static void PrintGLErrorString(GLenum error) {
  switch (error) {
  case GL_NO_ERROR:
    printf("No OpenGL error");
    return;
  case GL_INVALID_ENUM:
    printf("Invalid enum");
    return;
  case GL_INVALID_VALUE:
    printf("Invalid value");
    return;
  case GL_INVALID_OPERATION:
    printf("Invalid operation");
    return;
  // GL_STACK_OVERFLOW is undefined; bug in GLAD
  case 0x503:
    printf("Stack overflow");
    return;
  // GL_STACK_UNDERFLOW is undefined; bug in GLAD
  case 0x504:
    printf("Stack underflow");
    return;
  case GL_OUT_OF_MEMORY:
    printf("Out of memory");
    return;
  default:
    break;
  }
  printf("Unknown OpenGL error: %d", (int) error);
}

int CheckGLErrors(void) {
  GLenum error = glGetError();
  if (error == GL_NO_ERROR) return 1;
  while (error != GL_NO_ERROR) {
    printf("Got OpenGL error: ");
    PrintGLErrorString(error);
    printf("\n");
    error = glGetError();
  }
  return 0;
}

static void FramebufferResizedCallback(GLFWwindow *window, int width,
    int height) {
  glViewport(0, 0, 800, 600);
}

int SetupWindow(ApplicationState *s) {
  GLFWwindow *window = NULL;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(800, 600, "OpenGL tutorial", NULL, NULL);
  if (!window) {
    printf("Failed creating GLFW window.\n");
    glfwTerminate();
    return 0;
  }
  glfwMakeContextCurrent(window);
  s->window = window;
  return 1;
}

int SetupVertexBuffer(ApplicationState *s) {
  // TODO (next): Continue here.
  //  - Use the vertex_buffer_object field in s.
  //  - https://learnopengl.com/Getting-started/Hello-Triangle
  return 1;
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
  while (!glfwWindowShouldClose(s->window)) {
    if (!ProcessInputs(s->window)) {
      printf("Error processing inputs.\n");
      return 0;
    }

    glClearColor(0.3f, 0.05f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(s->window);
    glfwPollEvents();
    if (!CheckGLErrors()) return 0;
  }
  return 1;
}

int main(int argc, char **argv) {
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
    FreeApplicationState(s);
    glfwTerminate();
    return 1;
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(s->window, FramebufferResizedCallback);
  if (!CheckGLErrors()) {
    printf("OpenGL errors detected during initialization.\n");
    FreeApplicationState(s);
    glfwTerminate();
    return 0;
  }
  if (!RunMainLoop(s)) {
    printf("Application ended with an error.\n");
  } else {
    printf("Everything done OK.\n");
  }
  FreeApplicationState(s);
  glfwTerminate();
  return 0;
}

