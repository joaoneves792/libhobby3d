
#ifdef GLES
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <DAEMesh.h>
#include <Textures.h>
#include "DAE.h"
#include "shader.h"

DAE::DAE(char* filename){
	_model = new DAEMesh();
	_model->LoadFromFile(filename);
}


DAE::~DAE(){
	_model->Clear();
	delete _model;
}

void DAE::free() {
	_model->unloadModel();
}

void DAE::drawGL2(){
	_model->drawGL2();
}

void DAE::drawGLES2(){
    _model->drawGLES2();
}

void DAE::prepare(shader* shader){
    _model->prepareModel(shader->getShader());
}
