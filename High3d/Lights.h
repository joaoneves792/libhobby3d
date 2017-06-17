#ifndef _Lights_H_
#define _Lights_H_

#include <GL/glew.h>
#include "shader.h"

#define MAX_LIGHTS 10

class Lights{
private:
	GLint _lightingDisabledID;
	GLint _enabled[MAX_LIGHTS];
	GLint _enabledID;
	GLint _colorsID[MAX_LIGHTS];
	GLint _conesID[MAX_LIGHTS];
	GLint _positionID[MAX_LIGHTS];
public:
	Lights(shader* shader);
	virtual ~Lights();

	void enableLighting();
	void disableLighting();
	void enable(int light);
	void disable(int light);
	void setColor(int light, float red, float green, float blue, float intensity);
	void setPosition(int light, float x, float y, float z);
	void setCone(int light, float direction_x, float direction_y, float direction_z, float angle);
};

#endif
