#ifndef _shader_H_
#define _shader_H_

#ifdef GLES
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <Shader.h>

class shader{
private:
	Shader* _shader;
public:
	shader(char* vertShader, char* fragShader);
	virtual ~shader();
	GLuint getShader();
	void use();
};

#endif
