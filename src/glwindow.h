#ifndef GL_WINDOW_H
#define GL_WINDOW_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "geometry.h"

enum OBJMODE {
	NONE,
	TRANSLATE,
	ROTATE,
	SCALE,
};

enum PLANE {
	XY,
	XZ
};

enum ROTAXIS {
	XAXIS,
	YAXIS,
	ZAXIS
};

class OpenGLWindow
{
public:
	OpenGLWindow();

	void initGL();
	void render();
	bool handleEvent(SDL_Event e);
	void cleanup();

private:
	SDL_Window * sdlWin;
	unsigned char *data;
	int x, y, n;
	unsigned int texture;
	unsigned int normal;

	GLuint vaos[2];
	GLuint vertexBuffers[2];
	GLuint normalBuffers[2];
	GLuint textureBuffers[2];
	GLuint tangentBuffers[2];
	GLuint bitangentBuffers[2];
	GeometryData geometry[2];
	GLuint MatrixID;
	GLuint MVMatrixID;
	GLuint NormalID;
	GLuint ModelID;
	GLuint texID;

	//GLuint colorID[2];
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model[2];
	Uint64 NOW;
	Uint64 LAST;
	double deltaTime;

	OBJMODE objmode;
	PLANE tplane;
	ROTAXIS rotaxis;
	float translateSpeed;
	int NoLoadedModels;
	int ModelNo;
	int LightModelNo;
	bool lightModelOn;

	//lighting
	GLuint lightColorModel1;
	GLuint lightPositionModel1;
	GLuint lightAmbientModel1;
	GLuint lightDiffuseModel1;
	GLuint lightSpecularModel1;
	GLuint lightSpecularShine1;

	GLuint lightColorModel2;
	GLuint lightPositionModel2;
	GLuint lightAmbientModel2;
	GLuint lightDiffuseModel2;
	GLuint lightSpecularModel2;
	GLuint lightSpecularShine2;


	GLuint textureModel;
	GLuint normalModel;

	GLuint shaderLamps;
	GLuint lightColorLamps;

	glm::vec3 lightColor[2];
	glm::vec3 lightPos1;
	glm::vec3 lightAmbient1;
	glm::vec3 lightDiffuse1;
	glm::vec3 lightSpecular1;
	float specShine1;

	glm::vec3 lightPos2;
	glm::vec3 lightAmbient2;
	glm::vec3 lightDiffuse2;
	glm::vec3 lightSpecular2;
	float specShine2;

	GLuint shaderModels;

	GLuint vaoLight[2];
	GLuint vertexLightBuffer[2];
	GLuint MVPID;
	GeometryData lightGeometry[2];
	glm::mat4 lightModel[2];
	//glm::mat4 lightView;
	//glm::mat4 lightProjection;


};

#endif
