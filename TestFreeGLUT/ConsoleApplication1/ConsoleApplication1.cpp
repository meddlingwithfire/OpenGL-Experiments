#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl\glew.h>
#include <freeglut.h>

void onDisplay();
int initResources(void);
void freeResources();

GLuint program;
GLint attribute_coord2d;

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("My First Triangle");

	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewStatus));
		return EXIT_FAILURE;
	}

	if (1 == initResources())
	{
		glutDisplayFunc(onDisplay);
		glutMainLoop();
	}
	freeResources();

	return 0;
}

void onDisplay()
{
  /* Clear the background as white */
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
 
  glUseProgram(program);
  glEnableVertexAttribArray(attribute_coord2d);
  GLfloat triangle_vertices[] = {
     0.0,  0.8,
    -0.8, -0.8,
     0.8, -0.8,
  };
  /* Describe our vertices array to OpenGL (it can't guess its format automatically) */
  glVertexAttribPointer(
    attribute_coord2d, // attribute
    2,                 // number of elements per vertex, here (x,y)
    GL_FLOAT,          // the type of each element
    GL_FALSE,          // take our values as-is
    0,                 // no extra data between each position
    triangle_vertices  // pointer to the C array
  );
 
  /* Push each element in buffer_vertices to the vertex shader */
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glDisableVertexAttribArray(attribute_coord2d);
 
  /* Display the result */
  glutSwapBuffers();
}

int initResources(void)
{
	GLint compile_ok = GL_FALSE, link_ok = GL_FALSE;

	const char* version = (const char*)glGetString(GL_VERSION);
	std::cout << version << std::endl;

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const char *vs_source = 
	"#version 120\n"
	"attribute vec2 coord2d;\n"
	"void main(void) {\n"
	"	gl_Position = vec4(coord2d, 0.0, 1.0);\n"
	"}";
	glShaderSource(vs, 1, &vs_source, NULL);
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &compile_ok);

	if (0 == compile_ok)
	{
		fprintf(stderr, "Error in vertex shader\n");
		return 0;
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	const char *fs_source =
		"#version 120\n"
		"void main(void) {\n"
		"	gl_FragColor[0] = gl_FragCoord.x/640.0;\n"
		"	gl_FragColor[1] = gl_FragCoord.y/480.0;\n"
		"	gl_FragColor[2] = 0.5;\n"
		/*"	gl_FragColor[0] = 0.0;\n"
		"	gl_FragColor[1] = 0.0;\n"
		"	gl_FragColor[2] = 1.0;\n"*/
		"}";
	glShaderSource(fs, 1, &fs_source, NULL);
	glCompileShader(fs);
	glGetShaderiv(fs, GL_COMPILE_STATUS, &compile_ok);
	if (!compile_ok) {
		fprintf(stderr, "Error in fragment shader\n");
		return 0;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		return 0;
	}

	const char* attribute_name = "coord2d";
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord2d == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}
 
	return 1;
}

void freeResources()
{
	glDeleteProgram(program);
}