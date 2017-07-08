#ifndef _H3D_H_
#define _H3D_H_

#include "shader.h"
#include <H3DFile.h>

class H3D{
private:
	H3DFile* _model;

public: 
	H3D(char* filename);
	virtual ~H3D();
	void free();
	void drawGL2();
	void drawGLES2();
	void prepare(shader* shader);
};

#endif
