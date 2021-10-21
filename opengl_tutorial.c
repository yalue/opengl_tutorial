// This defines the main executable implementing the OpenGL tutorial from
// learnopengl.com.
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
  glDeleteProgram(s->shader_program);
  glDeleteTextures(1, &(s->texture));
  glDeleteVertexArrays(1, &(s->vertex_array_object));
  glDeleteBuffers(1, &(s->element_buffer_object));
  glDeleteBuffers(1, &(s->vertex_buffer_object));
  memset(s, 0, sizeof(*s));
  free(s);
}

uint8_t* ReadFullFile(const char *path) {
  long size = 0;
  uint8_t *to_return = NULL;
  FILE *f = fopen(path, "rb");
  if (!f) {
    printf("Failed opening %s: %s\n", path, strerror(errno));
    return NULL;
  }
  if (fseek(f, 0, SEEK_END) != 0) {
    printf("Failed seeking end of %s: %s\n", path, strerror(errno));
    fclose(f);
    return NULL;
  }
  size = ftell(f);
  if (size < 0) {
    printf("Failed getting size of %s: %s\n", path, strerror(errno));
    fclose(f);
    return NULL;
  }
  if ((size + 1) < 0) {
    printf("File %s too big.\n", path);
    fclose(f);
    return NULL;
  }
  if (fseek(f, 0, SEEK_SET) != 0) {
    printf("Failed rewinding to start of %s: %s\n", path, strerror(errno));
    fclose(f);
    return NULL;
  }
  // Use size + 1 to null-terminate the data.
  to_return = (uint8_t *) calloc(1, size + 1);
  if (!to_return) {
    printf("Failed allocating buffer to hold contents of %s.\n", path);
    fclose(f);
    return NULL;
  }
  if (fread(to_return, size, 1, f) < 1) {
    printf("Failed reading %s: %s\n", path, strerror(errno));
    fclose(f);
    free(to_return);
    return NULL;
  }
  fclose(f);
  return to_return;
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

// Creates the GLFWwindow. Returns 0 on error.
static int SetupWindow(ApplicationState *s) {
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

// Allocates the vertex buffer. Returns 0 on error.
static int SetupVertexBuffer(ApplicationState *s) {
  // Each row:
  //  - First three values: position (x, y, z)
  //  - Second three values: color (r, g, b)
  //  - Next two values: texture coordinate (u, v)
  float vertices[] = {
    0.5, 0.5, 0, 1.0, 0.0, 0.0, 1.0, 1.0,
    0.5, -0.5, 0, 0.0, 1.0, 0.0, 1.0, 0.0,
    -0.5, -0.5, 0, 0.0, 0.0, 1.0, 0.0, 0.0,
    -0.5, 0.5, 0, 0.5, 0.5, 0.5, 0.0, 1.0,
  };
  GLuint indices[] = {
    0, 1, 2,
    2, 3, 0,
  };
  glGenVertexArrays(1, &(s->vertex_array_object));
  glBindVertexArray(s->vertex_array_object);
  glGenBuffers(1, &(s->element_buffer_object));
  glGenBuffers(1, &(s->vertex_buffer_object));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->element_buffer_object);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, s->vertex_buffer_object);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set up the positions and colors to interleave:
  //  - Each has 3 values, so a stride of 6.
  //  - Colors are at attribute 1, and start at the fourth float in the array.
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
    (void *) (3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
    (void *) (6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  return CheckGLErrors();
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

// Loads any image texture(s) needed by the application. Returns 0 on error.
static int LoadTextures(ApplicationState *s) {
  int width, height, channels;
  unsigned char *image_data = stbi_load("container.jpg", &width, &height,
    &channels, 3);
  if (!image_data) {
    printf("Failed loading container.jpg.\n");
    return 0;
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
    GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenTextures(1, &(s->texture));
  glBindTexture(GL_TEXTURE_2D, s->texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
    GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(image_data);
  image_data = NULL;
  return CheckGLErrors();
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
  // Uncomment to render in wireframe mode.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(s->window)) {
    if (!ProcessInputs(s->window)) {
      printf("Error processing inputs.\n");
      return 0;
    }

    glClearColor(0.3f, 0.05f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(s->shader_program);
    glBindTexture(GL_TEXTURE_2D, s->texture);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s->element_buffer_object);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(s->window);
    glfwPollEvents();
    if (!CheckGLErrors()) return 0;
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
  glViewport(0, 0, 800, 600);
  glfwSetFramebufferSizeCallback(s->window, FramebufferResizedCallback);
  if (!SetupVertexBuffer(s)) {
    printf("Failed setting up vertex buffer.\n");
    to_return = 1;
    goto cleanup;
  }
  if (!LoadTextures(s)) {
    printf("Failed loading textures.\n");
    to_return = 1;
    goto cleanup;
  }
  if (!SetupShaderProgram(s)) {
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

