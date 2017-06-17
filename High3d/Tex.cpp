#include "Tex.h"
#include <ms3d/Textures.h>

Tex::Tex(char* filename){
	_tex = LoadGLTexture(filename);
}
int Tex::getTexture(){
	return _tex;
}

int Tex::genTexture(unsigned char *data, int width, int height){
	return generateGLTexture(data, width, height, true);
}
