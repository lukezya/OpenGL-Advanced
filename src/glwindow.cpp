#include <iostream>
#include <stdio.h>
#include <ctime>

#include "SDL.h"
#include <GL/glew.h>

#include "glwindow.h"
#include "geometry.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

const char* glGetErrorString(GLenum error)
{
	switch (error)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	default:
		return "UNRECOGNIZED";
	}
}

void glPrintError(const char* label = "Unlabelled Error Checkpoint", bool alwaysPrint = false)
{
	GLenum error = glGetError();
	if (alwaysPrint || (error != GL_NO_ERROR))
	{
		printf("%s: OpenGL error flag is %s\n", label, glGetErrorString(error));
	}
}

GLuint loadShader(const char* shaderFilename, GLenum shaderType)
{
	/*FILE* shaderFile;// = fopen(shaderFilename, "r"); - deprecated fopen method
	errno_t err = fopen_s(&shaderFile, shaderFilename, "r");
	if (err != 0)
		return 0;*/
	FILE* shaderFile = fopen(shaderFilename, "r");
	if(!shaderFile)
	{
			return 0;
	}

	fseek(shaderFile, 0, SEEK_END);
	long shaderSize = ftell(shaderFile);
	fseek(shaderFile, 0, SEEK_SET);

	char* shaderText = new char[shaderSize + 1];
	size_t readCount = fread(shaderText, 1, shaderSize, shaderFile);
	shaderText[readCount] = '\0';
	fclose(shaderFile);

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, (const char**)&shaderText, NULL);
	glCompileShader(shader);

	delete[] shaderText;

	return shader;
}

GLuint loadShaderProgram(const char* vertShaderFilename,
	const char* fragShaderFilename)
{
	//create shaders
	GLuint vertShader = loadShader(vertShaderFilename, GL_VERTEX_SHADER);
	GLuint fragShader = loadShader(fragShaderFilename, GL_FRAGMENT_SHADER);

	// link the program
	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	GLint linkStatus;
	glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
	if (linkStatus != GL_TRUE)
	{
		GLsizei logLength = 0;
		GLchar message[1024];
		glGetProgramInfoLog(program, 1024, &logLength, message);
		cout << "Shader load error: " << message << endl;
		return 0;
	}

	return program;
}

OpenGLWindow::OpenGLWindow()
{
	//initialize variables
	objmode = NONE;
	tplane = XY;
	rotaxis = ZAXIS;
	translateSpeed = 0.001f;
	NOW = SDL_GetPerformanceCounter();
	LAST = 0;
	deltaTime = 0;
	NoLoadedModels = 1;
	ModelNo = 0;
	LightModelNo = 0;
	lightModelOn = true;
}


void OpenGLWindow::initGL()
{
	// We need to first specify what type of OpenGL context we need before we can create the window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	sdlWin = SDL_CreateWindow("OpenGL Prac 1",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		960, 720, SDL_WINDOW_OPENGL);
	if (!sdlWin)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Error", "Unable to create window", 0);
	}
	SDL_GLContext glc = SDL_GL_CreateContext(sdlWin);
	SDL_GL_MakeCurrent(sdlWin, glc);
	SDL_GL_SetSwapInterval(1);

	// Initialize GLEW
	glewExperimental = true;
	GLenum glewInitResult = glewInit();
	glGetError(); // Consume the error erroneously set by glewInit()
	if (glewInitResult != GLEW_OK)
	{
		const GLubyte* errorString = glewGetErrorString(glewInitResult);
		cout << "Unable to initialize glew: " << errorString;
	}

	int glMajorVersion;
	int glMinorVersion;
	glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);
	cout << "Loaded OpenGL " << glMajorVersion << "." << glMinorVersion << " with:" << endl;
	cout << "\tVendor: " << glGetString(GL_VENDOR) << endl;
	cout << "\tRenderer: " << glGetString(GL_RENDERER) << endl;
	cout << "\tVersion: " << glGetString(GL_VERSION) << endl;
	cout << "\tGLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glClearColor(1.0f, 0.56f, 0.06f, 1.0f); //use orange to clear the screen
	glClearColor(0.0f, 0.0f, 0.2f, 1.0f); // use dark blue to clear the screen
	//--------------------------------------------------------------
	//Lights 1
	lightColor[LightModelNo] = glm::vec3(1.0f, 0.6f, 0.0f);
	lightPos1 = glm::vec3(1.5f, 1.0f, 5.0f);
	lightAmbient1 = glm::vec3(0.1f);
	lightDiffuse1 = glm::vec3(0.7f);
	lightSpecular1 = glm::vec3(0.8f);
	specShine1 = 64.0f;

	glGenVertexArrays(2, vaoLight);
	glBindVertexArray(vaoLight[LightModelNo]);

	//shader for light models
	shaderLamps = loadShaderProgram("lamp.vert", "lamp.frag");

	lightColorLamps = glGetUniformLocation(shaderLamps, "lightColor");
	MVPID = glGetUniformLocation(shaderLamps, "MVP");

	lightGeometry[LightModelNo].loadFromOBJFile("../resources/cube.obj");
	GLuint lightVertexLoc = glGetAttribLocation(shaderLamps, "position");
	glGenBuffers(2, vertexLightBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexLightBuffer[LightModelNo]);
	glBufferData(GL_ARRAY_BUFFER, lightGeometry[LightModelNo].vertexCount() * 3 * sizeof(float), (float*)lightGeometry[LightModelNo].vertexData(), GL_STATIC_DRAW);
	glVertexAttribPointer(lightVertexLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(lightVertexLoc);
	//model centred on the origin in world space - translate
	//get centre of model and move to light position
	lightModel[LightModelNo] = glm::mat4(1.0f);
	lightModel[LightModelNo] = glm::translate(lightModel[LightModelNo], glm::vec3(lightGeometry[LightModelNo].getCentreX(), lightGeometry[LightModelNo].getCentreY(), lightGeometry[LightModelNo].getCentreZ()));
	lightModel[LightModelNo] = glm::translate(lightModel[LightModelNo], lightPos1);
	LightModelNo++;

	//Lights 2
	lightColor[LightModelNo] = glm::vec3(0.0f, 1.0f, 1.0f);
	lightPos2 = glm::vec3(-2.0f, -1.0f, 2.0f);
	lightAmbient2 = glm::vec3(0.1f);
	lightDiffuse2 = glm::vec3(0.7f);
	lightSpecular2 = glm::vec3(0.8f);
	specShine2 = 128.0f;

	lightGeometry[LightModelNo].loadFromOBJFile("../resources/cube.obj");
	glBindVertexArray(vaoLight[LightModelNo]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexLightBuffer[LightModelNo]);
	glBufferData(GL_ARRAY_BUFFER, lightGeometry[LightModelNo].vertexCount() * 3 * sizeof(float), (float*)lightGeometry[LightModelNo].vertexData(), GL_STATIC_DRAW);
	glVertexAttribPointer(lightVertexLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(lightVertexLoc);

	lightModel[LightModelNo] = glm::mat4(1.0f);
	lightModel[LightModelNo] = glm::translate(lightModel[LightModelNo], glm::vec3(lightGeometry[LightModelNo].getCentreX(), lightGeometry[LightModelNo].getCentreY(), lightGeometry[LightModelNo].getCentreZ()));
	lightModel[LightModelNo] = glm::translate(lightModel[LightModelNo], lightPos2);
	LightModelNo++;
	//-------------------------------------------------------------
	glGenVertexArrays(2, vaos);
	glBindVertexArray(vaos[ModelNo]);

	//shader for models
	shaderModels = loadShaderProgram("bumping.vert", "bumping.frag");
	glUseProgram(shaderModels);

	//get location of uniform variables
	lightColorModel1 = glGetUniformLocation(shaderModels, "Light[0].LCol");
	lightPositionModel1 = glGetUniformLocation(shaderModels, "lightPos1");//Light[0].LPos lightPos1
	lightAmbientModel1 = glGetUniformLocation(shaderModels, "Light[0].La");
	lightDiffuseModel1 = glGetUniformLocation(shaderModels, "Light[0].Ld");
	lightSpecularModel1 = glGetUniformLocation(shaderModels, "Light[0].Ls");
	lightSpecularShine1 = glGetUniformLocation(shaderModels, "Light[0].SpecShine");

	//send to location
	glUniform3f(lightColorModel1, lightColor[0].r, lightColor[0].g, lightColor[0].b);
	glUniform3f(lightPositionModel1, lightPos1.x, lightPos1.y, lightPos1.z);
	glUniform3f(lightAmbientModel1, lightAmbient1.r, lightAmbient1.g, lightAmbient1.b);
	glUniform3f(lightDiffuseModel1, lightDiffuse1.r, lightDiffuse1.g, lightDiffuse1.b);
	glUniform3f(lightSpecularModel1, lightSpecular1.r, lightSpecular1.g, lightSpecular1.b);
	glUniform1f(lightSpecularShine1, specShine1);

	//get location of uniform variables
	lightColorModel2 = glGetUniformLocation(shaderModels, "Light[1].LCol");
	lightPositionModel2 = glGetUniformLocation(shaderModels, "lightPos2"); //Light[1].LPos lightPos2
	lightAmbientModel2 = glGetUniformLocation(shaderModels, "Light[1].La");
	lightDiffuseModel2 = glGetUniformLocation(shaderModels, "Light[1].Ld");
	lightSpecularModel2 = glGetUniformLocation(shaderModels, "Light[1].Ls");
	lightSpecularShine2 = glGetUniformLocation(shaderModels, "Light[1].SpecShine");

	//send to location
	glUniform3f(lightColorModel2, lightColor[1].r, lightColor[1].g, lightColor[1].b);
	glUniform3f(lightPositionModel2, lightPos2.x, lightPos2.y, lightPos2.z);
	glUniform3f(lightAmbientModel2, lightAmbient2.r, lightAmbient2.g, lightAmbient2.b);
	glUniform3f(lightDiffuseModel2, lightDiffuse2.r, lightDiffuse2.g, lightDiffuse2.b);
	glUniform3f(lightSpecularModel2, lightSpecular2.r, lightSpecular2.g, lightSpecular2.b);
	glUniform1f(lightSpecularShine2, specShine2);

	//more uniform variables
	textureModel = glGetUniformLocation(shaderModels, "Tex");
	normalModel = glGetUniformLocation(shaderModels, "Norm");
	MatrixID = glGetUniformLocation(shaderModels, "MVP");
	MVMatrixID = glGetUniformLocation(shaderModels, "ModelViewMatrix");
	NormalID = glGetUniformLocation(shaderModels, "NormalMatrix");
	ModelID = glGetUniformLocation(shaderModels, "ModelMatrix");

	//set texture0 and texture1
	glUniform1i(textureModel, 0);
	glUniform1i(normalModel, 1);

	//setting up texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char *data = stbi_load("../resources/textureDiffuse.png", &width, &height, &nrChannels, STBI_rgb);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data);
	}

	//setting up normal texture
	glGenTextures(1, &normal);
	glBindTexture(GL_TEXTURE_2D, normal);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int widthNorm, heightNorm, nrChannelsNorm;
	unsigned char *dataNorm = stbi_load("../resources/textureNormal.png", &widthNorm, &heightNorm, &nrChannelsNorm, STBI_rgb);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, dataNorm);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
		stbi_image_free(data);
	}

	// Projection matrix : 45 degrees Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units - clipping
	Projection = glm::perspective(glm::radians(45.0f),4.0f/3.0f,0.1f, 100.0f);

	// Camera matrix
	View = glm::lookAt(
		glm::vec3(0, 0, 20), // Camera is at (0,0,20), in World Space - on the z-axis - so far away to accomodate for dragon object
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
	);

	// Load the model that we want to use
	geometry[ModelNo].loadFromOBJFile("../resources/sphere.obj");
	//vector<float>* vertices = static_cast<float*>(geometry.vertexData());
	int vertexLoc = glGetAttribLocation(shaderModels, "VertexPosition");
	int normalLoc = glGetAttribLocation(shaderModels, "VertexNormal");
	int textureLoc = glGetAttribLocation(shaderModels, "VertexTex");
	int tangentLoc = glGetAttribLocation(shaderModels, "VertexTangent");
	int bitangentLoc = glGetAttribLocation(shaderModels, "VertexBitangent");

	//buffer vertices
	glGenBuffers(2, vertexBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].vertexCount()* 3 * sizeof(float), (float*)geometry[ModelNo].vertexData(), GL_STATIC_DRAW);
	glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(vertexLoc);

	//buffer normals
	glGenBuffers(2, normalBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getNormalCount() * sizeof(float), (float*)geometry[ModelNo].normalData(), GL_STATIC_DRAW);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(normalLoc);

	//buffer textures
	glGenBuffers(2, textureBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getTextureCount() * sizeof(float), (float*)geometry[ModelNo].textureCoordData(), GL_STATIC_DRAW);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(textureLoc);

	//buffer tangents
	glGenBuffers(2, tangentBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getTangentCount() * sizeof(float), (float*)geometry[ModelNo].tangentData(), GL_STATIC_DRAW);
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(tangentLoc);

	//buffer bitangents
	glGenBuffers(2, bitangentBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getBitangentCount() * sizeof(float), (float*)geometry[ModelNo].bitangentData(), GL_STATIC_DRAW);
	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(bitangentLoc);

	//model centred on the origin in world space - translate
	//identity matrix, get centre of model and move to origin
	Model[ModelNo] = glm::mat4(1.0f);
	Model[ModelNo] = glm::translate(Model[ModelNo], glm::vec3(geometry[ModelNo].getCentreX(), geometry[ModelNo].getCentreY(), geometry[ModelNo].getCentreZ()));
	++ModelNo;

	// Load the model that we want to use
	geometry[ModelNo].loadFromOBJFile("../resources/suzanne.obj");
	glBindVertexArray(vaos[ModelNo]);

	//buffer vertices
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].vertexCount() * 3 * sizeof(float), (float*)geometry[ModelNo].vertexData(), GL_STATIC_DRAW);
	glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(vertexLoc);

	//buffer normals
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getNormalCount() * sizeof(float), (float*)geometry[ModelNo].normalData(), GL_STATIC_DRAW);
	glVertexAttribPointer(normalLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(normalLoc);

	//buffer textures
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getTextureCount() * sizeof(float), (float*)geometry[ModelNo].textureCoordData(), GL_STATIC_DRAW);
	glVertexAttribPointer(textureLoc, 2, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(textureLoc);
	Model[ModelNo] = glm::mat4(1.0f);

	//buffer tangents
	glGenBuffers(2, tangentBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, tangentBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getTangentCount() * sizeof(float), (float*)geometry[ModelNo].tangentData(), GL_STATIC_DRAW);
	glVertexAttribPointer(tangentLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(tangentLoc);

	//buffer bitangents
	glGenBuffers(2, bitangentBuffers);
	glBindBuffer(GL_ARRAY_BUFFER, bitangentBuffers[ModelNo]);
	glBufferData(GL_ARRAY_BUFFER, geometry[ModelNo].getBitangentCount() * sizeof(float), (float*)geometry[ModelNo].bitangentData(), GL_STATIC_DRAW);
	glVertexAttribPointer(bitangentLoc, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(bitangentLoc);
	++ModelNo;

	int windowWidth, windowHeight;
	SDL_GetWindowSize(sdlWin, &windowWidth, &windowHeight);
	SDL_WarpMouseInWindow(NULL,windowWidth/2, windowHeight/2);

	glPrintError("Setup complete", true);
}

void OpenGLWindow::render()
{
	LAST = NOW;
	NOW = SDL_GetPerformanceCounter();
	deltaTime = (NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//shader for models
	glUseProgram(shaderModels);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, normal);

	Model[1] = glm::translate(Model[0], glm::vec3(-geometry[0].getCentreX() + geometry[1].getCentreX(), -geometry[0].getCentreY()+ geometry[1].getCentreY(), -geometry[0].getCentreZ() + geometry[1].getCentreZ()+((geometry[0].getMaxZ() - geometry[0].getMinZ()) / 2)+((geometry[1].getMaxZ()-geometry[1].getMinZ())/2)));
	for (int i = 0;i < NoLoadedModels;++i) {
		// Our ModelViewProjection : multiplication of our 3 matrices
		glm::mat4 mvp = Projection * View * Model[i];
		glm::mat4 mv = View * Model[i];
		glm::mat3 n = glm::transpose(glm::inverse(View*Model[i]));
		glm::vec3 lightViewPos1 = glm::vec3(View * glm::vec4(lightPos1, 1.0f));
		glm::vec3 lightViewPos2 = glm::vec3(View * glm::vec4(lightPos2, 1.0f));
		//send uniform variables that may change due to keyboard input
		glUniform3f(lightPositionModel1, lightViewPos1.x, lightViewPos1.y, lightViewPos1.z);
		glUniform3f(lightPositionModel2, lightViewPos2.x, lightViewPos2.y, lightViewPos2.z);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(MVMatrixID, 1, GL_FALSE, &mv[0][0]);
		glUniformMatrix3fv(NormalID, 1, GL_FALSE, &n[0][0]);
		glUniformMatrix3fv(ModelID, 1, GL_FALSE, &Model[i][0][0]);
		//draw model
		glBindVertexArray(vaos[i]);
		glDrawArrays(GL_TRIANGLES, 0, geometry[i].vertexCount());
	}

	//if show light models on
	if (lightModelOn) {
		//shader for lamps
		glUseProgram(shaderLamps);

		for (int j = 0;j < LightModelNo;++j) {
			glUniform3f(lightColorLamps, lightColor[j].r, lightColor[j].g, lightColor[j].b);
			glm::mat4 lightMVP = Projection * View * lightModel[j];
			glUniformMatrix4fv(MVPID, 1, GL_FALSE, &lightMVP[0][0]);
			//draw light model
			glBindVertexArray(vaoLight[j]);
			glDrawArrays(GL_TRIANGLES, 0, lightGeometry[j].vertexCount());
		}
	}

	// Swap the front and back buffers on the window, effectively putting what we just "drew"
	// onto the screen (whereas previously it only existed in memory)
	SDL_GL_SwapWindow(sdlWin);
}

// The program will exit if this function returns false
bool OpenGLWindow::handleEvent(SDL_Event e)
{
	// handles mouse and keyboard inputs
	switch (e.type) {

		//key pressed: to switch between different modes
		case SDL_KEYDOWN:
			switch (e.key.keysym.sym) {
				//'ESC' to quit
				case SDLK_ESCAPE:
					return false;

				//'T' to switch between planes
				case SDLK_t:
					cout << "Translate Mode" << endl;
					objmode = TRANSLATE;
					//set first call to relative mouse state to mark start position
					SDL_GetRelativeMouseState(NULL, NULL);
					switch (tplane) {
						case XY:
							tplane = XZ;
							cout << "Translating on XZ-plane..." << endl;
							break;
						case XZ:
							tplane = XY;
							cout << "Translating on XY-plane..." << endl;
							break;
						default:
							break;
					}
					break;

				//'R' for to switch between rotation axes
				case SDLK_r:
					cout << "Rotate Mode" << endl;
					objmode = ROTATE;
					switch (rotaxis) {
						case XAXIS:
							rotaxis = YAXIS;
							cout << "Rotating around Y-axis..." << endl;
							break;
						case YAXIS:
							rotaxis = ZAXIS;
							cout << "Rotating around Z-axis..." << endl;
							break;
						case ZAXIS:
							rotaxis = XAXIS;
							cout << "Rotating around X-axis..." << endl;
							break;
						default:
							break;
					}
					break;

				//'S' for scale
				case SDLK_e:
					cout << "Scale Mode" << endl;
					objmode = SCALE;
					break;

				case SDLK_SPACE:
					cout << "No Mode" << endl;
					objmode = NONE;
					break;

				//load in second model
				case SDLK_l:
					cout << "Resetting all transformations of first model..." << endl;
					Model[0] = glm::translate(glm::mat4(1.0f), glm::vec3(geometry[0].getCentreX(), geometry[0].getCentreY(), geometry[0].getCentreZ()));
					cout << "Loading second model..." << endl;
					++NoLoadedModels;
					break;

				//move light1
				case SDLK_DOWN:
					lightPos1 = lightPos1 - glm::vec3(0.0f,0.2f,0.0f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(0.0f, -0.2f, 0.0f));
					break;

				case SDLK_UP:
					lightPos1 = lightPos1 - glm::vec3(0.0f, -0.2f, 0.0f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(0.0f, 0.2f, 0.0f));
					break;

				case SDLK_LEFT:
					lightPos1 = lightPos1 - glm::vec3(0.2f, -0.0f, 0.0f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(-0.2f, -0.0f, 0.0f));
					break;

				case SDLK_RIGHT:
					lightPos1 = lightPos1 - glm::vec3(-0.2f, 0.0f, 0.0f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(0.2f, 0.0f, 0.0f));
					break;

				case SDLK_n:
					lightPos1 = lightPos1 - glm::vec3(0.0f, 0.0f, 0.2f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(0.0f, 0.0f, -0.2f));
					break;

				case SDLK_m:
					lightPos1 = lightPos1 - glm::vec3(0.0f, 0.0f, -0.2f);
					lightModel[0] = glm::translate(lightModel[0], glm::vec3(0.0f, 0.0f, 0.2f));
					break;

				//move light2
				case SDLK_s:
					lightPos2 = lightPos2 - glm::vec3(0.0f, 0.2f, 0.0f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(0.0f, -0.2f, 0.0f));
					break;

				case SDLK_w:
					lightPos2 = lightPos2 - glm::vec3(0.0f, -0.2f, 0.0f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(0.0f, 0.2f, 0.0f));
					break;

				case SDLK_a:
					lightPos2 = lightPos2 - glm::vec3(0.2f, -0.0f, 0.0f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(-0.2f, -0.0f, 0.0f));
					break;

				case SDLK_d:
					lightPos2 = lightPos2 - glm::vec3(-0.2f, 0.0f, 0.0f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(0.2f, 0.0f, 0.0f));
					break;

				case SDLK_z:
					lightPos2 = lightPos2 - glm::vec3(0.0f, 0.0f, 0.2f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(0.0f, 0.0f, -0.2f));
					break;

				case SDLK_x:
					lightPos2 = lightPos2 - glm::vec3(0.0f, 0.0f, -0.2f);
					lightModel[1] = glm::translate(lightModel[1], glm::vec3(0.0f, 0.0f, 0.2f));
					break;

				//toggle light models
				case SDLK_o:
					if (lightModelOn)
						lightModelOn = false;
					else
						lightModelOn = true;
					break;

				default:
					break;
			}
			break;

		//mouse moved: move movement moves model on plane for translation
		case SDL_MOUSEMOTION:
			if (objmode == TRANSLATE) {
				//get difference in mouse positions of mouse movement
				int mouseX, mouseY;
				SDL_GetRelativeMouseState(&mouseX, &mouseY);
				float translateX = translateSpeed *deltaTime*mouseX;
				float translateY = translateSpeed *deltaTime*mouseY;
				switch (tplane) {
				case XY:
					Model[0] = glm::translate(Model[0], glm::vec3(translateX, -translateY, 0.0f));
					break;
				case XZ:
					Model[0] = glm::translate(Model[0], glm::vec3(translateX, 0.0f, translateY));
					break;
				default:
					break;
				}

			}
			break;

		//mouse button pressed: right click rotate right, left click rotate left for rotation
		case SDL_MOUSEBUTTONDOWN:
			if (objmode == ROTATE) {
				float fRotate = 0.0f;
				if (e.button.button == SDL_BUTTON_LEFT) {
					fRotate = -10.0f;
				}
				else if (e.button.button == SDL_BUTTON_RIGHT) {
					fRotate = 10.0f;
				}
				switch (rotaxis) {
				case XAXIS:
					//Model[0] = glm::rotate(Model[0], glm::radians(fRotate), glm::vec3(1.0f, 0.0f, 0.0f));
					View = glm::rotate(View, glm::radians(fRotate), glm::vec3(1.0f, 0.0f, 0.0f));
					break;
				case YAXIS:
					//Model[0] = glm::rotate(Model[0], glm::radians(fRotate), glm::vec3(0.0f, 1.0f, 0.0f));
					View = glm::rotate(View, glm::radians(fRotate), glm::vec3(0.0f, 1.0f, 0.0f));
					break;
				case ZAXIS:
					//Model[0] = glm::rotate(Model[0], glm::radians(fRotate), glm::vec3(0.0f, 0.0f, 1.0f));
					View = glm::rotate(View, glm::radians(fRotate), glm::vec3(0.0f, 0.0f, 1.0f));
					break;
				default:
					break;
				}
			}
			break;

		//mouse wheel motion: mouse wheel scroll up scale up, scroll down scale down for scale
		case SDL_MOUSEWHEEL:
			if (objmode == SCALE) {
				//scroll up
				if (e.wheel.y > 0) {
					//Model[0] = glm::scale(Model[0], glm::vec3(1.2f,1.2f,1.2f));
					View = glm::scale(View, glm::vec3(1.2f, 1.2f, 1.2f));
				}
				//scroll down
				else if (e.wheel.y < 0) {
					//Model[0] = glm::scale(Model[0], glm::vec3(0.8f, 0.8f, 0.8f));
					View = glm::scale(View, glm::vec3(0.8f, 0.8f, 0.8f));
				}

			}
			break;

		default:
			break;
	}

	return true;
}

void OpenGLWindow::cleanup()
{
	glDeleteBuffers(2, vertexBuffers);
	glDeleteBuffers(2, normalBuffers);
	glDeleteBuffers(2, textureBuffers);
	glDeleteBuffers(2, vertexLightBuffer);
	glDeleteVertexArrays(2, vaos);
	glDeleteVertexArrays(2, vaoLight);
	SDL_DestroyWindow(sdlWin);
}
