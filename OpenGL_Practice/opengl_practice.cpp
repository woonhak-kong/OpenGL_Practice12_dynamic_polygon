//***************************************************************************
// GAME2012_A2_LastFirst.cpp by by Woonhak Kong - 101300258 (C) 2020 All Rights Reserved.
//
// Assignment 2 submission.
//
// Description:
// This source file is for 3D Programming Assigmnet 1.
// This assignment is for animation, perspective, and camera movement.
//*****************************************************************************


using namespace std;

#include "stdlib.h"
#include "time.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <string>
#include <fstream>
#include <array>


// Prototypes
int setShader(char* shaderType, char* shaderFile);
char* readShader(std::string fileName);
void timer(int);
void createModel(int n);

#define BUFFER_OFFSET(x)  ((const void*) (x))
#define FPS 60
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,0.9,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)


static unsigned int
program,
vertexShaderId,
fragmentShaderId;

glm::mat4 MVP, View, Projection;
GLuint vao, points_vbo, colors_vbo, modelID, indices_vbo;
int randomColorSeed[10];

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
float cameraSpeed = 0.2;

float scale = 1.0f, inc = -0.05f, angle1 = 0.0f, angle2 = 0.0f;


static float R = 2.0; // Radius of circle.
static float X = 0.0; // X-coordinate of center of circle.
static float Y = 0.0; // Y-coordinate of center of circle.
const int MaxNumVertices = 500; // Number of vertices on circle.
static int numVertices = 10;
#define PI 3.14159265358979324

float theta = 0.0f;

std::array<glm::vec3, MaxNumVertices> vertices = {};
std::array<glm::vec3, MaxNumVertices> colors = {};

GLfloat shape_vertices[MaxNumVertices][3] = { 0.0f, };
GLfloat shape_colors[MaxNumVertices][3] = { 0.0f, };
GLuint shape_indices[MaxNumVertices] = { 0, };

void init(void)
{
	// Create shader program executable.
	vertexShaderId = setShader((char*)"vertex", (char*)"basic.vert");
	fragmentShaderId = setShader((char*)"fragment", (char*)"basic.frag");
	program = glCreateProgram();
	glAttachShader(program, vertexShaderId);
	glAttachShader(program, fragmentShaderId);
	glLinkProgram(program);
	glUseProgram(program);

	modelID = glGetUniformLocation(program, "MVP");

	// Projection matrix : 45¡Ä Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	//Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	Projection = glm::ortho(-3.0f, 3.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	View = glm::lookAt(
		glm::vec3(0, 0, 3), // Origin. Camera is at (0,0,3), in World Space
		glm::vec3(0, 0, 0),	  // Look target. Looks at the origin
		glm::vec3(0, 1, 0)   // Up vector. Head is up (set to 0,-1,0 to look upside-down)
	);


	// Create and set-up the vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);



	// vertex
	glGenBuffers(1, &points_vbo);
	// Populate the position buffer
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	// don't need to do now
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cube_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	// Enable the vertex attribute arrays
	glEnableVertexAttribArray(0); // for Vertex position

	// color
	glGenBuffers(1, &colors_vbo);
	// Populate the color buffer
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	// don't need to do now
	//glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte*)NULL);
	glEnableVertexAttribArray(1); // for Vertex color



	glGenBuffers(1, &indices_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);


	glEnable(GL_DEPTH_TEST);
	timer(0);
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(float scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, glm::vec3(scale));

	View = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	//View = glm::lookAt(
	//	glm::vec3(xVal, yVal, 5), // Origin. Camera is at (0,0,3), in World Space
	//	glm::vec3(0, 0, -4),	  // Look target. Looks at the origin
	//	glm::vec3(0, 1, 0)   // Up vector. Head is up (set to 0,-1,0 to look upside-down)
	//);
	MVP = Projection * View * Model;
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &MVP[0][0]);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);

	transformObject(1.0f, XY_AXIS, angle1 += 0.f, glm::vec3(0.0f, 0.0f, 0.0f));

	createModel(numVertices);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_vertices), shape_vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shape_colors), shape_colors, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(shape_indices), shape_indices, GL_STATIC_DRAW);

	// by array
	//glDrawArrays(GL_TRIANGLE_FAN, 0, numVertices);

	// by element array
	glDrawElements(GL_TRIANGLE_FAN, numVertices, GL_UNSIGNED_INT, NULL);


	glutSwapBuffers(); // Now for a potentially smoother render.
}

void idle() // Not even called.
{
	glutPostRedisplay();
}

void timer(int) {
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, timer, 0);
}


// Keyboard input processing routine.
void keyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
			exit(0);
			break;
		case 'w':
			cameraPos += cameraSpeed * cameraFront;
			break;
		case 's':
			cameraPos -= cameraSpeed * cameraFront;
			break;
		case 'r':
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			break;
		case 'f':
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraRight)) * cameraSpeed;
			break;
		case 'a':
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case 'd':
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		case '+':
			numVertices++;
			break;
		default:
			break;
	}
}

//---------------------------------------------------------------------
//
// main
//

int main(int argc, char** argv)
{
	//Before we can open a window, theremust be interaction between the windowing systemand OpenGL.In GLUT, this interaction is initiated by the following function call :
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

	//if you comment out this line, a window is created with a default size
	glutInitWindowSize(800, 800);

	//the top-left corner of the display
	glutInitWindowPosition(0, 0);

	glutCreateWindow("GAME2012_A2_KongWoonhak, 101300258");

	// background color is white
	glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init(); // Our own custom function.

	//If there are events in the queue, our program responds to them through functions
	//called callbacks.A callback function is associated with a specific type of event.
	//A display callback is generated when the application programm or the
	//operating system determines that the graphics in a window need to be redrawn.
	glutDisplayFunc(display); // Output.
	//glutIdleFunc(idle);

	glutKeyboardFunc(keyDown); // Input.

	//begin an event-processing loop
	glutMainLoop();
}

// Function to initialize shaders.
int setShader(char* shaderType, char* shaderFile)
{
	int shaderId;
	char* shader = readShader(shaderFile);

	if (shaderType == "vertex") shaderId = glCreateShader(GL_VERTEX_SHADER);
	if (shaderType == "tessControl") shaderId = glCreateShader(GL_TESS_CONTROL_SHADER);
	if (shaderType == "tessEvaluation") shaderId = glCreateShader(GL_TESS_EVALUATION_SHADER);
	if (shaderType == "geometry") shaderId = glCreateShader(GL_GEOMETRY_SHADER);
	if (shaderType == "fragment") shaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(shaderId, 1, (const char**)&shader, NULL);
	glCompileShader(shaderId);

	return shaderId;
}
// Function to read external shader file.
char* readShader(std::string fileName)
{
	// Initialize input stream.
	std::ifstream inFile(fileName.c_str(), std::ios::binary);

	// Determine shader file length and reserve space to read it in.
	inFile.seekg(0, std::ios::end);
	int fileLength = inFile.tellg();
	char* fileContent = (char*)malloc((fileLength + 1) * sizeof(char));

	// Read in shader file, set last character to NUL, close input stream.
	inFile.seekg(0, std::ios::beg);
	inFile.read(fileContent, fileLength);
	fileContent[fileLength] = '\0';
	inFile.close();

	return fileContent;
}

void createModel(int n)
{
	theta = 0.0f;
	for (int i = 0; i < n; ++i)
	{

		vertices[i] = glm::vec3(X + R * cos(theta), Y + R * sin(theta), 0.0);
		colors[i] = glm::vec3((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX);

		theta += 2 * PI / n;
	}

	for (int i = 0; i < n; ++i) {

		shape_vertices[i][0] = vertices[i].x;
		shape_vertices[i][1] = vertices[i].y;
		shape_vertices[i][2] = vertices[i].z;

		shape_colors[i][0] = colors[i][0];
		shape_colors[i][1] = colors[i][1];
		shape_colors[i][2] = colors[i][2];

		// for indices
		shape_indices[i] = i;
	}


}