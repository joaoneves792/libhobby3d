#include <GL/glew.h>
#include <string>
#include <math.h>
#include <sstream>
#include <glm/glm.hpp>
#include "Lights.h"
#include "shader.h"

Lights::Lights(shader* shader){

	_enabledID = glGetUniformLocation( shader->getShader(), "lightsEnabled[0]");
	_lightingDisabledID = glGetUniformLocation( shader->getShader(), "disableLighting");

	std::string lightsPosition = "lightPosition_worldspace";
	std::string lightsColor = "lightColor";
	std::string lightCone = "lightCone";
	for(int i = 0; i<MAX_LIGHTS; i++){
		_enabled[i] = 0;
		std::ostringstream ss;
		ss << i;
		std::string id = "[" + ss.str() + "]";
		_colorsID[i] = glGetUniformLocation( shader->getShader(), (lightsColor + id).c_str());
		_conesID[i] = glGetUniformLocation( shader->getShader(), (lightCone + id).c_str());
		_positionID[i] = glGetUniformLocation( shader->getShader(), (lightsPosition + id).c_str());
	}

	glUniform1iv(_enabledID, MAX_LIGHTS, &_enabled[0]);
	glUniform1i(_lightingDisabledID, 1); //Start with lighting disabled
}

Lights::~Lights(){
	//Is there anything to do here?
}

void Lights::enableLighting(){
	glUniform1i(_lightingDisabledID, 0);
}

void Lights::disableLighting(){
	glUniform1i(_lightingDisabledID, 1);
}

void Lights::enable(int light){
	_enabled[light] = 1;
	glUniform1iv(_enabledID, MAX_LIGHTS, &_enabled[0]);
}

void Lights::disable(int light){
	_enabled[light] = 0;
	glUniform1iv(_enabledID, MAX_LIGHTS, &_enabled[0]);
}

void Lights::setColor(int light, float red, float green, float blue, float intensity){
	glUniform4f(_colorsID[light], red, green, blue, intensity);
}

void Lights::setPosition(int light, float x, float y, float z){
	glUniform3f(_positionID[light], x, y, z);
}

void Lights::setCone(int light, float direction_x, float direction_y, float direction_z, float angle){
	glUniform4f(_conesID[light], direction_x, direction_y, direction_z, cos(glm::radians(angle)));
}

