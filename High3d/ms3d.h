#ifndef _ms3d_H_
#define _ms3d_H_

#include <ms3d/MS3DFile.h>
#include "shader.h"

class ms3d{
private:
	CMS3DFile* _model;

public: 
	ms3d(char* filename, bool overrideAmbient=false, bool overrideSpecular=false, bool overrideDiffuse=false, bool overrideEmissive=false);
	ms3d();
	virtual ~ms3d();
	void draw();
	void drawGL3();
	void createRectangle(float width, float height, int texture);
	void translateModel(float x, float y, float z);
	void changeRectangleTexture(int texture);
	void prepare(shader* shader);
	float* getJointPosition(char* jointName);
	void changeTexture(char* groupName, char* textureFile);
	void changeMaterialEmissive(char* name, float red, float green, float blue);
	void changeMaterialTransparency(char* name, float alpha);
	static void initGlew();
};

#endif
