# This makefile is intended to be used on my Linux machine.
.PHONY: all clean

GLFW_DIR := /storage/other/glfw/install
GLFW_CFLAGS := -I$(GLFW_DIR)/include -L$(GLFW_DIR)/lib -lglfw3 -ldl -lm -lpthread

all: opengl_tutorial

opengl_tutorial: opengl_tutorial.c opengl_tutorial.h
	gcc -Wall -Werror -O3 -o opengl_tutorial opengl_tutorial.c glad/src/glad.c \
		-Iglad/include $(GLFW_CFLAGS)

