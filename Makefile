# This makefile is intended to be used on my Linux machine.
.PHONY: all clean

GLFW_DIR := /storage/other/glfw/install
GLFW_CFLAGS := -I$(GLFW_DIR)/include -L$(GLFW_DIR)/lib -lglfw3 -ldl -lm -lpthread

all: opengl_tutorial

parse_obj.o: parse_obj.c parse_obj.h
	gcc -Wall -Werror -O3 -c -o parse_obj.o parse_obj.c

opengl_tutorial: opengl_tutorial.c opengl_tutorial.h parse_obj.o
	gcc -Wall -Werror -O3 -o opengl_tutorial opengl_tutorial.c glad/src/glad.c \
		-Iglad/include -I cglm/include $(GLFW_CFLAGS)

clean:
	rm -f parse_obj.o
	rm -f opengl_tutorial

