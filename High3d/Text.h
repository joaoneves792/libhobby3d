#ifndef _Text_H_
#define _Text_H_

#include <GL/glew.h>

class Text{
private:
	GLuint _texture;
	GLuint _vao;
        int _width;
	int _height;
	int _rows;
	int _columns;
	int _fontSize;
	int _vSpace;
	int _hSpace;

public:
	Text(char* bitmapFont, int width, int height, int rows, int columns, int fontSize, int vertSpacing, int horizSpacing);
	virtual ~Text();

	void drawTextLine(char* text, float size);
};

#endif
