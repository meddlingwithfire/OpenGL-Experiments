
// Left off here: http://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_06

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <gl\glew.h>
#include <freeglut.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "FileReader.h"
#include "TargaReader.h"
#include "SOIL.h"

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
void onResize(int width, int height);
void loadTexture();

int screen_width=800, screen_height=600;

GLuint program;

GLuint vbo_cube_vertices, vbo_cube_texcoords, vbo_cube_colors;
GLuint ibo_cube_elements;

GLint attribute_coord3d, attribute_v_color, attribute_texcoord, uniform_mvp, uniform_fade;

GLuint texture_id;
GLint uniform_mytexture;

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
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
		glutReshapeFunc(onResize);
		glutIdleFunc(onIdle);

		enableOpenGLTransparency();

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	float angle = glutGet(GLUT_ELAPSED_TIME) / 1000.0f * 45.0f;  // 45° per second
	glm::vec3 axis_y(0, 1, 0);
	glm::mat4 anim = glm::rotate(glm::mat4(1.0f), angle, axis_y);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -4.0));
	glm::mat4 view = glm::lookAt(glm::vec3(0.0, 2.0, 0.0), glm::vec3(0.0, 0.0, -4.0), glm::vec3(0.0, 1.0, 0.0));
	glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);

	glm::mat4 mvp = projection * view * model * anim;

	float sine = sinf(glutGet(GLUT_ELAPSED_TIME) / 1000.0f * (2.0f * 3.14f) / 5.0f);
	float fade = 0.75f + (sine * 0.25f);

	glUseProgram(program);
	glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
	glUniform1f(uniform_fade, fade);
	glutPostRedisplay();
}

void enableOpenGLTransparency()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void onDisplay()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(uniform_mytexture, /*GL_TEXTURE*/0);

	glEnableVertexAttribArray(attribute_coord3d);
	// Describe our vertices array to OpenGL (it can't guess its format automatically)
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glVertexAttribPointer(
		attribute_coord3d, // attribute
		3,                 // number of elements per vertex, here (x,y,z)
		GL_FLOAT,          // the type of each element
		GL_FALSE,          // take our values as-is
		0,                 // no extra data between each position
		0                  // offset of first element
	);

	glEnableVertexAttribArray(attribute_texcoord);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
	glVertexAttribPointer(
		attribute_texcoord, // attribute
		2,                  // number of elements per vertex, here (x,y)
		GL_FLOAT,           // the type of each element
		GL_FALSE,           // take our values as-is
		0,                  // no extra data between each position
		0                   // offset of first element
	);

	/* Push each element in buffer_vertices to the vertex shader */
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	int size;  glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	glDrawElements(GL_TRIANGLES, size/sizeof(GLushort), GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(attribute_coord3d);
	glDisableVertexAttribArray(attribute_texcoord);
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

	loadTexture();

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
		return false;
	}

	attribute_name = "texcoord";
	attribute_texcoord = glGetAttribLocation(program, attribute_name);
	if (attribute_texcoord == -1) {
		fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
		return false;
	}

	attribute_name = "mvp";
	uniform_mvp = glGetUniformLocation(program, attribute_name);
	if (uniform_mvp == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", attribute_name);
		return false;
	}

	attribute_name = "mytexture";
	uniform_mytexture = glGetUniformLocation(program, attribute_name);
	if (uniform_mytexture == -1) {
		fprintf(stderr, "Could not bind uniform %s\n", attribute_name);
		return false;
	}

	return true;
}

void createTriangleAttributes()
{
	GLfloat cube_vertices[] = {
		// front
		-1.0, -1.0,  1.0,
		1.0, -1.0,  1.0,
		1.0,  1.0,  1.0,
		-1.0,  1.0,  1.0,
		// top
		-1.0,  1.0,  1.0,
		1.0,  1.0,  1.0,
		1.0,  1.0, -1.0,
		-1.0,  1.0, -1.0,
		// back
		1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0,  1.0, -1.0,
		1.0,  1.0, -1.0,
		// bottom
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, -1.0,  1.0,
		-1.0, -1.0,  1.0,
		// left
		-1.0, -1.0, -1.0,
		-1.0, -1.0,  1.0,
		-1.0,  1.0,  1.0,
		-1.0,  1.0, -1.0,
		// right
		1.0, -1.0,  1.0,
		1.0, -1.0, -1.0,
		1.0,  1.0, -1.0,
		1.0,  1.0,  1.0
	};
	glGenBuffers(1, &vbo_cube_vertices);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_vertices);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	GLfloat cube_texcoords[2*4*6] = {
		// front
		0.0, 0.0,
		1.0, 0.0,
		1.0, 1.0,
		0.0, 1.0
	};
	for (int i = 1; i < 6; i++)
	{
		memcpy(&cube_texcoords[i*4*2], &cube_texcoords[0], 2*4*sizeof(GLfloat));
	}
	glGenBuffers(1, &vbo_cube_texcoords);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cube_texcoords);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoords), cube_texcoords, GL_STATIC_DRAW);

	GLushort cube_elements[] = {
	// front
	0,  1,  2,
	2,  3,  0,
	// top
	4,  5,  6,
	6,  7,  4,
	// back
	8,  9, 10,
	10, 11,  8,
	// bottom
	12, 13, 14,
	14, 15, 12,
	// left
	16, 17, 18,
	18, 19, 16,
	// right
	20, 21, 22,
	22, 23, 20,
	};
	glGenBuffers(1, &ibo_cube_elements);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_cube_elements);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);
}

void freeResources()
{
	glDeleteProgram(program);
	glDeleteBuffers(1, &vbo_cube_vertices);
	glDeleteBuffers(1, &vbo_cube_colors);
	glDeleteBuffers(1, &ibo_cube_elements);

	glDeleteTextures(1, &texture_id);
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

void onResize(int width, int height) {
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}

void loadTexture() {
	texture_id = SOIL_load_OGL_texture
	(
		"OpenGL-Tutorial-Texture.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if( 0 == texture_id )
	{
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
	}

	//glGenTextures(1, &texture_id);
	//glBindTexture(GL_TEXTURE_2D, texture_id);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//glTexImage2D(GL_TEXTURE_2D, // target
	//		0,  // level, 0 = base, no minimap,
	//		GL_RGB, // internalformat
	//		256,  // width
	//		256,  // height
	//		0,  // border, always 0 in OpenGL ES
	//		GL_RGB,  // format
	//		GL_UNSIGNED_BYTE, // type
	//		textureBits);
}