#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void FramebufferResizedCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, 800, 600);
}

// Following the OpenGL tutorial at https://learnopengl.com/Getting-started/Hello-Window
int main(int argc, char **argv) {
  GLFWwindow *window = NULL;
  if (!glfwInit()) {
    printf("Failed glfwInit().\n");
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window = glfwCreateWindow(800, 600, "OpenGL tutorial", NULL, NULL);
  if (!window) {
    printf("Failed creating GLFW window.\n");
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
    printf("Failed initializing GLAD.\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return 1;
  }
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(window, FramebufferResizedCallback);
  // TODO: Check for errors.
  while (!glfwWindowShouldClose(window)) {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  printf("Everything done OK.\n");
  glfwTerminate();
 return 0;
}
