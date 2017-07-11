
#ifdef GLES
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <H3DFile.h>
#include <Textures.h>
#include "H3D.h"
#include "shader.h"

H3D::H3D(char* filename){
	_model = new H3DFile();
	_model->LoadFromFile(filename);
}


H3D::~H3D(){
	_model->unloadModel();
	delete _model;
}

void H3D::free() {
	_model->unloadModel();
}

void H3D::drawGL2(){
	_model->drawGL2();
}

void H3D::drawGLES2(){
    _model->drawGLES2();
}

void H3D::prepare(shader* shader){
    _model->prepareModel(shader->getShader());
}

void H3D::setCurrentFrame(int f) {
    _model->setCurrentFrame(f);
}
