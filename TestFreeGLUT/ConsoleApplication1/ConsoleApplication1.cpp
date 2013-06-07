#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl\glew.h>
#include <freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "FileReader.h"

void onDisplay();
bool initResources(void);
void freeResources();
void printLog(GLuint object);
GLuint createShader(const char* filename, GLenum type);
void enableOpenGLTransparency();
bool createShaderProgram();
bool registerShaderAttributes();
void createTriangleAttributes();
void onIdle();

GLuint program;
GLuint vbo_triangle, vbo_triangle_colors;
GLint attribute_coord3d, attribute_v_color;
GLint uniform_fade;
GLint uniform_m_transform;

struct attributes {
	GLfloat coord2d[3];
	GLfloat v_color[3];
};

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(640, 480);
	glutCreateWindow("My First Triangle");

	GLenum glewStatus = glewInit();
	if (glewStatus != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewStatus));
		return EXIT_FAILURE;
	}

	bool successfullyInitializedResources = initResources();
	if (successfullyInitializedResources)
	{
		glutDisplayFunc(onDisplay);
		glutIdleFunc(onIdle);
		enableOpenGLTransparency();
		glutMainLoop();
	}
	freeResources();

	std::string input = "";
	std::cout << "Terminating..." << std::endl;
	std::cin >> input;

	return 0;
}

void onIdle()
{
	float move = 0;//sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.14) / 5); // -1<->+1 every 5 seconds
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0 * 45;  // 45° per second
	glm::vec3 axis_z(0, 0, 1);
	glm::mat4 m_transform = glm::translate(glm::mat4(1.0f), glm::vec3(move, 0.0, 0.0)) * glm::rotate(glm::mat4(1.0f), angle, axis_z);
	glUniformMatrix4fv(uniform_m_transform, 1, GL_FALSE, glm::value_ptr(m_transform));

	float cur_fade = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0 * (2*3.1459) / 5) / 2 + 0.5; // 0->1->0 every 5 seconds
	glUniform1f(uniform_fade, cur_fade);

	glUseProgram(program);
	glutPostRedisplay();
}

void enableOpenGLTransparency()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void onDisplay()
{
	/* Clear the background as white */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);

	glEnableVertexAttribArray(attribute_coord3d);
	glEnableVertexAttribArray(attribute_v_color);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glVertexAttribPointer(
		attribute_coord3d,   // attribute
		3,                   // number of elements per vertex, here (x,y,z)
		GL_FLOAT,            // the type of each element
		GL_FALSE,            // take our values as-is
		sizeof(struct attributes),  // next coord3d appears every 6 floats
		0                    // offset of first element
		);
	glVertexAttribPointer(
		attribute_v_color,      // attribute
		3,                      // number of elements per vertex, here (r,g,b)
		GL_FLOAT,               // the type of each element
		GL_FALSE,               // take our values as-is
		sizeof(struct attributes),  // stride
		//(GLvoid*) (2 * sizeof(GLfloat))     // offset of first element
		(GLvoid*) offsetof(struct attributes, v_color)  // offset
		);

	/* Push each element in buffer_vertices to the vertex shader */
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_v_color);
	glutSwapBuffers();
}

bool initResources(void)
{
	const char* version = (const char*)glGetString(GL_VERSION);
	std::cout << version << std::endl;

	if (!createShaderProgram())
	{
		return false;
	}

	if (!registerShaderAttributes())
	{
		return false;
	}

	createTriangleAttributes();

	return true;
}

bool createShaderProgram()
{
	GLuint vs = createShader("triangle.vs.glsl", GL_VERTEX_SHADER);
	if (vs == 0)
	{
		return false;
	}

	GLuint fs = createShader("triangle.fs.glsl", GL_FRAGMENT_SHADER);
	if (fs == 0) 
	{
		return false;
	}

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	GLint link_ok = GL_FALSE;
	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
	if (!link_ok) {
		fprintf(stderr, "glLinkProgram:");
		printLog(program);
		return false;
	}

	return true;
}

bool registerShaderAttributes() 
{
	const char* attribute_name = "coord3d";
	attribute_coord3d = glGetAttribLocation(program, attribute_name);
	if (attribute_coord3d == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return 0;
	}

	attribute_name = "v_color";
	attribute_v_color = glGetAttribLocation(program, attribute_name);
	if (attribute_v_color == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return false;
	}

	const char* uniform_name = "fade";
	uniform_fade = glGetUniformLocation(program, uniform_name);
	if (uniform_fade == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		return false;
	}

	uniform_name = "m_transform";
	uniform_m_transform = glGetUniformLocation(program, uniform_name);
	if (uniform_m_transform == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
		return 0;
	}

	return true;
}

void createTriangleAttributes()
{
	struct attributes triangle_attributes[] = {
		{{ 0.0,  0.8, 0.0}, {1.0, 1.0, 0.0}},
		{{-0.8, -0.8, 0.0}, {0.0, 0.0, 1.0}},
		{{ 0.8, -0.8, 0.0}, {1.0, 0.0, 0.0}}
	};
	glGenBuffers(1, &vbo_triangle);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_triangle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);
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