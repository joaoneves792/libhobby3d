#ifndef _DAE_H_
#define _DAE_H_

#include <MS3DFile.h>
#include "shader.h"
#include <DAEMesh.h>

class DAE{
private:
	DAEMesh* _model;

public: 
	DAE(char* filename);
	virtual ~DAE();
	void free();
	void drawGL2();
	void drawGLES2();
	void prepare(shader* shader);
};

#endif
