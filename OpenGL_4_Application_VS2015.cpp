//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "SkyBox.hpp"
#include "Camera.hpp"
#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"



int glWindowWidth = 640;
int glWindowHeight = 480;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
GLuint lightPointLoc;

GLuint camDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 roz = glm::vec3(1.0f, 0.0f, 1.0f);
glm::vec3 galben = glm::vec3(1.0f,1.0f, 0.0f);
GLuint colorLoc;
GLuint colorLoc2;
GLuint ok1Loc;
GLuint ok2Loc;
GLuint ok3Loc;

glm::vec3 lightPoint[] = {
		glm::vec3(1.0f,  25.0f,  10.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
};

gps::Camera myCamera(glm::vec3(0.0f, 1.0f, 2.5f), glm::vec3(0.0f, 0.0f, 0.0f));
GLfloat cameraSpeed = 0.1f;

bool pressedKeys[1024];
GLfloat angle;
GLfloat lightAngle ;

gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D bus;
gps::Model3D corp;
gps::Model3D coada;
gps::Model3D avion;
gps::Model3D copac;
gps::Model3D lightCube;

gps::Shader myCustomShader;
//gps::Shader lightingShader;
gps::Shader lightShader;
gps::Shader lightShader2;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

//bool ok1 = false, ok2 = false , ok3 = false;
int ok1 = 1, ok2 = 0, ok3 = 0;

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	lightShader2.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

GLboolean firstMouse = true;
float lastX = 320, lastY = 340;
float yaw, pitch;
int nr = 0;
int nr2 = 0;
int okRain = 0;
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.05;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	myCamera.rotate(pitch, yaw);
	/*glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);*/
}
float fov = 45.0f;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
int okf = 1;
bool okRotate = false;
void processMovement()
{

	if (pressedKeys[GLFW_KEY_Q]) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}
	if (pressedKeys[GLFW_KEY_R]) {
		if (okRotate == false) okRotate = true;
		else okRotate = false;
	}
	if (pressedKeys[GLFW_KEY_E]) {
		angle -= 1.0f;
		if (angle < 0.0f)
			angle += 360.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}
	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_X])
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_C]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_SMOOTH);
	}
	if (pressedKeys[GLFW_KEY_P]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
	if (pressedKeys[GLFW_KEY_R]) {
		okRain = 1;
	}
	if (pressedKeys[GLFW_KEY_J]) {
	//	printf("DA %f",lightAngle);
		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		myCustomShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}
	if (pressedKeys[GLFW_KEY_M])
	{
		if (okf == 0) {
			glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			okf = 1;
		}
		else {
			glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			okf = 0;
		}
	}
	if (pressedKeys[GLFW_KEY_V])
	{
		if (ok1 == 0) ok1 = 1;
		else ok1 = 0;
		printf("%d\n", ok1);
		glDisable(GL_LIGHT0);
	}
	if (pressedKeys[GLFW_KEY_B])
	{
		if (ok2 == 0) ok2 = 1;
		else ok2 = 0;
		printf("%d\n", ok2);
	}
	if (pressedKeys[GLFW_KEY_N])
	{
		if (ok3 == 0) ok3 = 1;
		else ok3 = 0;
		printf("%d\n", ok3);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetScrollCallback(glWindow, scroll_callback);

	return true;
}

void initOpenGLState()
{
	/*glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise*/
	glClearColor(0.3, 0.3, 0.3, 1.0);
	glViewport(0, 0, retina_width, retina_height);
	//!!!!
	glEnable(GL_FRAMEBUFFER_SRGB);
	//!!!!
	//glEnable(GL_BLEND);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	//glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	//myModel = gps::Model3D("objects/nanosuit/nanosuit.obj", "objects/nanosuit/");
	myModel = gps::Model3D("objects/oras/oras12.obj", "");
	ground = gps::Model3D("objects/oras/ground.obj", "");
	lightCube = gps::Model3D("objects/cube/cube.obj", "objects/cube/");
	bus = gps::Model3D("objects/bus/bus.obj", "objects/bus/");
	//avion = gps::Model3D("objects/piper_pa18_obj/piper_pa18.obj", "objects/piper_pa18_obj/");
	avion = gps::Model3D("objects/avion/SC_Private_0012.obj", "objects/avion/");
	corp = gps::Model3D("objects/Horse/corp.obj", "objects/Horse/");
	coada = gps::Model3D("objects/Horse/coada.obj", "objects/Horse/");
	copac = gps::Model3D("objects/EU43_Castanea_sativa_Chestnut_obj/copaciii.obj", "objects/EU43_Castanea_sativa_Chestnut_obj/");

}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader2.loadShader("shaders/lightCube2.vert", "shaders/lightCube2.frag");

	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
	//myCustomShader2.loadShader("shaders/shaderStartPoint.vert", "shaders/shaderStartPoint.frag");
	//myCustomShader2.useShaderProgram();
	//lightingShader.loadShader("shaders/lighting.vert", "shaders/lighting.frag");

	std::vector<const GLchar*> faces;

	faces.push_back("textures/skybox/right.tga");
	faces.push_back("textures/skybox/left.tga");
	faces.push_back("textures/skybox/top.tga");
	faces.push_back("textures/skybox/bottom.tga");
	faces.push_back("textures/skybox/back.tga");
	faces.push_back("textures/skybox/front.tga");

	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
}

void initUniforms()
{
	myCustomShader.useShaderProgram();

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 2.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));


	lightPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPoint0");
	glUniform3fv(lightPointLoc, 4, glm::value_ptr(lightPoint[0]));

	lightPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPoint1");
	glUniform3fv(lightPointLoc, 4, glm::value_ptr(lightPoint[1]));

	lightPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPoint2");
	glUniform3fv(lightPointLoc, 4, glm::value_ptr(lightPoint[2]));

	lightPointLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPoint3");
	glUniform3fv(lightPointLoc, 4, glm::value_ptr(lightPoint[3]));

	camDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "camDir");
	glUniform3fv(camDirLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));

	
	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

/*
	myCustomShader2.useShaderProgram();///aici pt a ne sigura ca e activ
	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader2.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	*/
	/*
	lightingShader.useShaderProgram();
	GLint lightPosLoc = glGetUniformLocation(lightingShader.shaderProgram, "light.position");
	GLint lightSpotdirLoc = glGetUniformLocation(lightingShader.shaderProgram, "light.direction");
	GLint lightSpotCutOffLoc = glGetUniformLocation(lightingShader.shaderProgram, "light.cutOff");
	GLint lightSpotOuterCutOffLoc = glGetUniformLocation(lightingShader.shaderProgram, "light.outerCutOff");
	GLint viewPosLoc = glGetUniformLocation(lightingShader.shaderProgram, "viewPos");
	glUniform3f(lightPosLoc, myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z);
	glUniform3f(lightSpotdirLoc, myCamera.getCameraDirection().x, myCamera.getCameraDirection().y, myCamera.getCameraDirection().z);
	glUniform1f(lightSpotCutOffLoc, glm::cos(glm::radians(12.5f)));
	glUniform1f(lightSpotOuterCutOffLoc, glm::cos(glm::radians(17.5f)));
	glUniform3f(viewPosLoc, myCamera.getCameraPosition().x, myCamera.getCameraPosition().y, myCamera.getCameraPosition().z);
	// Set lights properties
	glUniform3f(glGetUniformLocation(lightingShader.shaderProgram, "light.ambient"), 0.1f, 0.1f, 0.1f);
	// We set the diffuse intensity a bit higher; note that the right lighting conditions differ with each lighting method and environment.
	// Each environment and lighting type requires some tweaking of these variables to get the best out of your environment.
	glUniform3f(glGetUniformLocation(lightingShader.shaderProgram, "light.diffuse"), 0.8f, 0.8f, 0.8f);
	glUniform3f(glGetUniformLocation(lightingShader.shaderProgram, "light.specular"), 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.shaderProgram, "light.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(lightingShader.shaderProgram, "light.linear"), 0.09);
	glUniform1f(glGetUniformLocation(lightingShader.shaderProgram, "light.quadratic"), 0.032);
	// Set material properties
	glUniform1f(glGetUniformLocation(lightingShader.shaderProgram, "material.shininess"), 32.0f);
	*/
	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	colorLoc = glGetUniformLocation(lightShader.shaderProgram, "color");
	glUniform3fv(colorLoc, 1, glm::value_ptr(galben));

	lightShader2.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	colorLoc2 = glGetUniformLocation(lightShader2.shaderProgram, "color");
	glUniform3fv(colorLoc2, 1, glm::value_ptr(roz));
}
float angleCam = 0;
int ok = 1;
float busx = -5.0f;
float avionx = -500.0f;
float coadaangle = 0.0f;
int plus = 1;

int i = 0;
bool inceput = true;
void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();
	
	myCamera.setCameraPosition(glm::vec3(myCamera.getCameraPosition().x, 0.7f, myCamera.getCameraPosition().z));
	if (inceput) {
		myCamera.rotate(0.0f, 10.0f);
		for(int j=0;j<10;j++)
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		inceput = false;
	}
	
	if (i != 360) {
		i++;
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
		
	}
	if (okRotate == true) {
		angle += 1.0f;
		if (angle > 360.0f)
			angle -= 360.0f;
	}
	//render the scene to the depth buffer (first pass)

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	myModel.Draw(depthMapShader);
	depthMapShader.useShaderProgram();

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	model = glm::translate(model, glm::vec3(busx, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	bus.Draw(depthMapShader);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	model = glm::translate(model, glm::vec3(-20.0f, 0.45f, -7.2f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	corp.Draw(depthMapShader);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	model = glm::translate(model, glm::vec3(-20.0f, 0.45f, -7.2f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	coada.Draw(depthMapShader);

	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//model = glm::translate(model, glm::vec3(0.0f, -2.6f, 0.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	copac.Draw(depthMapShader);

	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	ground.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();
	//myCustomShader2.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	myModel.Draw(myCustomShader);
	//myModel.Draw(myCustomShader2);

	//create model matrix for ground
	//model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	ground.Draw(myCustomShader);
	//ground.Draw(myCustomShader2);

	//BUS
		//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(2.0f, 2.0f, 2.0f));
	model = glm::translate(model, glm::vec3(busx, -0.025f, -0.2f));
	model = glm::rotate(model, glm::radians(50.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	bus.Draw(myCustomShader);
	//bus.Draw(myCustomShader2);
	if (i >= 360) if (busx < 3.5f) busx += 0.05f;
	
	//AVION
		//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
	model = glm::translate(model, glm::vec3(avionx, 200.0f, 20.0f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	avion.Draw(myCustomShader);
	if (avionx < 500.0f) avionx += 0.4f;
	else avionx = -500.0f;

	//HORSE
		//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix

	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	model = glm::translate(model, glm::vec3(-20.0f, 0.45f, -7.2f));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	corp.Draw(myCustomShader);
//	corp.Draw(myCustomShader2);
	//initialize the model matrix
	model = glm::mat4(1.0f);
	//create model matrix

	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(0.4f, 0.4f, 0.4f));
	model = glm::translate(model, glm::vec3(-20.0f, 0.45f, -7.2f));
	//model = glm::rotate(model, glm::radians(-coadaangle), glm::vec3(1, 0, 0));
	model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
	//model = glm::rotate(model, glm::radians(-coadaangle), glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	if (coadaangle >= 0.0f && coadaangle <= 90.0f) {
		if (plus == 1) coadaangle++;
		else coadaangle--;
		if (coadaangle == 90.0f) plus = 0;
		if (coadaangle == 0.0f) plus = 1;
	}


	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	coada.Draw(myCustomShader);
	//coada.Draw(myCustomShader2);
	//COPAC

	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//model = glm::translate(model, glm::vec3(0.0f, -12.6f, 0.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	copac.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

	//create model matrix for nanosuit
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	copac.Draw(myCustomShader);
	//copac.Draw(myCustomShader2);

	//SKYBOX
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
		glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
		glm::value_ptr(projection));

	mySkyBox.Draw(skyboxShader, view, projection);


	//draw a white cube around the light
	if (ok2 == 1) {
		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		//	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
			/*model = glm::translate(model,glm::vec3(2.5f, -1.0f, 0.0f));
			model = glm::translate(model, 1.0f * lightDir);
			model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
			glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			*/
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		model = glm::translate(model, lightPoint[0]);
		//model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader);
	}
	if (ok3 == 1) {
		lightShader2.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		//glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));

		//model = glm::translate(model, glm::vec3(16.5f, -25.0f, 10.0f));
		model = glm::translate(model, glm::vec3(1.33f, 2.6f, -1.1f));
		//model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0, 1, 0));
		model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader2.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		lightCube.Draw(lightShader2);
	}
}


int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		myCustomShader.useShaderProgram();
		ok1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "ok1");
		glUniform1i(ok1Loc, ok1);

		ok2Loc = glGetUniformLocation(myCustomShader.shaderProgram, "ok2");
		glUniform1i(ok2Loc, ok2);
		ok3Loc = glGetUniformLocation(myCustomShader.shaderProgram, "ok3");
		glUniform1i(ok3Loc, ok3);
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();

	return 0;
}
