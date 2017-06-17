#ifndef _shader_H_
#define _shader_H_

#include <GL/glew.h>
#include <ms3d/Shader.h>

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
