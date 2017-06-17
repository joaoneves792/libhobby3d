#ifndef _Tex_H_
#define _Tex_H_

class Tex{
private:
	int _tex;
public:
	Tex(char* filename);
	int getTexture();
	int genTexture(unsigned char *data, int width, int height);
};

#endif
