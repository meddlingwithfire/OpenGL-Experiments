#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl\glew.h>
#include <freeglut.h>

#include "FileReader.h"

void onDisplay();
int initResources(void);
void freeResources();
void printLog(GLuint object);
GLuint createShader(const char* filename, GLenum type);

GLuint program;
GLint attribute_coord2d;
GLuint vbo_triangle;

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

	std::string input = "";
	std::cin >> input;

	return 0;
}

void onDisplay()
{
	/* Clear the background as white */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glEnableVertexAttribArray(attribute_coord2d);
	/* Describe our vertices array to OpenGL (it can't guess its format automatically) */
	glVertexAttribPointer(
		attribute_coord2d, // attribute
		2,                 // number of elements per vertex, here (x,y)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
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

	GLuint vs = createShader("triangle.vs.glsl", GL_VERTEX_SHADER);
	if (vs == 0)
	{
		return 0;
	}

	GLuint fs = createShader("triangle.fs.glsl", GL_FRAGMENT_SHADER);
	if (fs == 0) 
	{
		return 0;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		printLog(program);
		return 0;
	}

	const char* attribute_name = "coord2d";
	attribute_coord2d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord2d == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}

	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	GLfloat triangle_vertices[] = {
		0.0,  0.8,
		-0.8, -0.8,
		0.8, -0.8,
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_vertices), triangle_vertices, GL_STATIC_DRAW);

	return 1;
}

void freeResources()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_triangle);
}

GLuint createShader(const char* filename, GLenum type)
{
	meddlingwithfire::FileReader* fileReader = new meddlingwithfire::FileReader();
	const GLchar* source = fileReader->readAllText(filename);
	delete fileReader;

	if (source == NULL) {
		fprintf(stderr, "Error opening %s: ", filename); perror("");
		return 0;
	}
	GLuint res = glCreateShader(type);
	const GLchar* sources[1] = { source };
	int size = sizeof(sources);
	glShaderSource(res, 1, sources, NULL);
	free((void*)source);

	glCompileShader(res);
	GLint compile_ok = GL_FALSE;
	glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);
	if (compile_ok == GL_FALSE) {
		fprintf(stderr, "%s:", filename);
		printLog(res);
		glDeleteShader(res);
		return 0;
	}

	return res;
}

void printLog(GLuint object)
{
	GLint log_length = 0;
	if (glIsShader(object))
	{
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else if (glIsProgram(object))
	{
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	}
	else 
	{
		fprintf(stderr, "printlog: Not a shader or a program\n");
		return;
	}

	char* log = (char*)malloc(log_length);

	if (glIsShader(object))
	{
		glGetShaderInfoLog(object, log_length, NULL, log);
	}
	else if (glIsProgram(object))
	{
		glGetProgramInfoLog(object, log_length, NULL, log);
	}

	fprintf(stderr, "%s", log);
	free(log);
}