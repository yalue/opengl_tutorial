gcc -Wall -Werror -O3 -o opengl_tutorial opengl_tutorial.c ^
  parse_obj.c ^
  model.c ^
  shader_program.c ^
  utilities.c ^
  scapegoat_tree.c ^
  glad\src\glad.c ^
  -I cglm\include ^
  -I glad\include ^
  -I C:\bin\glfw-3.3.3\include ^
  -L C:\bin\glfw-3.3.3\lib-static-ucrt ^
  -lglfw3dll ^
  -lm

