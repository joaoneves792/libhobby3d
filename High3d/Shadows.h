#ifndef _Shadows_H_
#define _Shadows_H_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "shader.h"
#include "GLM.h"

class Shadows{
private:
	shader* _normalShader;
	shader* _shadowMapShader;
	GLuint _framebuffer;
	GLuint _depthTexture;
	GLM* _glm;

	glm::mat4 _depthBiasMVP;
	GLuint _depthBiasMVPID;
	GLuint _shadowMapID;
	GLuint _shadowTypeID;
	int _windowSize[2];

	int _textureWidth;
	int _textureHeight;
	float _orthoBox[6];

public:
	Shadows(GLM* glm, shader* normalShader, shader* shadowMapShader, int window_width, int window_height, int shadow_map_width, int shadow_map_height);
	virtual ~Shadows();
	void prepareToMapDepth(float lightPosX, float lightPosY, float lightPosZ);
	void changeOrthoBox(float left, float right, float bottom, float top, float near, float far);
	void returnToNormalDrawing();
	int getShadowTexture();
	void setShadowType(int type);
};

#endif
