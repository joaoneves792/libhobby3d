#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shadows.h"
#include "shader.h"
#include "GLM.h"

Shadows::Shadows(GLM* glm, shader* normalShader, shader* shadowMapShader, int window_width, int window_height, int shadow_map_width, int shadow_map_height){
	_glm = glm;
	_normalShader = normalShader;
	_shadowMapShader = shadowMapShader;
	_windowSize[0] = window_width;
	_windowSize[1] = window_height;

	_textureHeight = shadow_map_height;
	_textureWidth = shadow_map_width;

	_orthoBox[0] = -200;
	_orthoBox[1] = 200;
	_orthoBox[2] = -200;
	_orthoBox[3] = 200;
	_orthoBox[4] = -500;
	_orthoBox[5] = 500;

	_depthBiasMVPID = glGetUniformLocation( normalShader->getShader(), "depthBiasMVP");
	_shadowMapID = glGetUniformLocation( normalShader->getShader(), "shadowMap" );
	_shadowTypeID = glGetUniformLocation( normalShader->getShader(), "shadowType" );

	setShadowType(0);

	glGenFramebuffers(1, &_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

	glGenTextures(1, &_depthTexture);
        glBindTexture(GL_TEXTURE_2D, _depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, _textureWidth, _textureHeight, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _depthTexture, 0);
        glDrawBuffer(GL_NONE); // No color buffer is drawn to.

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Shadows::~Shadows(){
	glDeleteTextures(1, &_depthTexture);
	glDeleteFramebuffers(1, &_framebuffer);
}

void Shadows::changeOrthoBox(float left, float right, float bottom, float top, float near, float far){
	_orthoBox[0] = left;
	_orthoBox[1] = right;
	_orthoBox[2] = bottom;
	_orthoBox[3] = top;
	_orthoBox[4] = near;
	_orthoBox[5] = far;
}

int Shadows::getShadowTexture(){
	return _depthTexture;
}
void Shadows::prepareToMapDepth(float lightPosX, float lightPosY, float lightPosZ){
	glm::mat4 biasMatrix(
			0.5, 0.0, 0.0, 0.0,
		       	0.0, 0.5, 0.0, 0.0,
		       	0.0, 0.0, 0.5, 0.0,
		       	0.5, 0.5, 0.5, 1.0);

	_glm->selectMatrix(MODEL);
	_glm->pushMatrix();
	_glm->selectMatrix(VIEW);
	_glm->pushMatrix();
	_glm->selectMatrix(PROJECTION);
	_glm->pushMatrix();

	_glm->changeShader(_shadowMapShader);


	glm::vec3 position = glm::vec3(lightPosX, lightPosY, lightPosZ);
	glm::vec3 invDir = glm::normalize(position);
	
	_glm->otho(_orthoBox[0] , _orthoBox[1], _orthoBox[2], _orthoBox[3], _orthoBox[4], _orthoBox[5]);
	_glm->lookAt(invDir.x, invDir.y, invDir.z, 0, 0, 0, 0, 1, 0);
	_glm->selectMatrix(MODEL);
	_glm->loadIdentity();

	_depthBiasMVP = biasMatrix*(_glm->getMVP());

	glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
	glViewport(0, 0, _textureWidth, _textureHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_FRONT);
}

void Shadows::returnToNormalDrawing(){
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	_glm->changeShader(_normalShader);
	_glm->selectMatrix(PROJECTION);
	_glm->popMatrix();
	_glm->selectMatrix(VIEW);
	_glm->popMatrix();
	_glm->selectMatrix(MODEL);
	_glm->popMatrix();

	glUniformMatrix4fv(_depthBiasMVPID, 1, GL_FALSE, glm::value_ptr(_depthBiasMVP));
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _depthTexture);
	glUniform1i(_shadowMapID, 1);

	glActiveTexture(GL_TEXTURE0);
	glViewport(0, 0, _windowSize[0], _windowSize[1]);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glCullFace(GL_BACK);
}
	
void Shadows::setShadowType(int type){
	glUniform1i(_shadowTypeID, type);
}
