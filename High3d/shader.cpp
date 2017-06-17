#include "shader.h"
#include <GL/glew.h>
#include <ms3d/Shader.h>

shader::shader(char* vertShader, char* fragShader){
	_shader = new Shader(vertShader, fragShader);
}

shader::~shader(){
	delete _shader;
}

GLuint shader::getShader(){
	return _shader->getShader();
}
void shader::use(){
	_shader->use();
}
