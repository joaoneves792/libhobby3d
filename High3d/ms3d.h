#ifndef _ms3d_H_
#define _ms3d_H_

#include <MS3DFile.h>
#include "shader.h"

class ms3d{
private:
	CMS3DFile* _model;

public: 
	ms3d(char* filename, bool overrideAmbient=false, bool overrideSpecular=false, bool overrideDiffuse=false, bool overrideEmissive=false);
	ms3d();
	virtual ~ms3d();
	void free();
	void draw();
	void drawGL3();
	void drawGLES2();
	void createRectangle(float width, float height, int texture);
	void translateModel(float x, float y, float z);
	void changeRectangleTexture(int texture);
	void prepare(shader* shader);
	float* getJointPosition(char* jointName);
	void changeTexture(char* groupName, char* textureFile);
	void changeGLTexture(char* groupName, int texture);
	void changeMaterialEmissive(char* name, float red, float green, float blue);
	void changeMaterialTransparency(char* name, float alpha);
	void setAnimationTime(float t);
	static void initGlew();
};

#endif
