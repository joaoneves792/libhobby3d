#include "Text.h"
#include <ms3d/Textures.h>
#include <GL/glew.h>
#include <string.h>

Text::Text(char* bitmapFont, int width, int height, int rows, int columns, int fontSize, int vertSpacing, int horizSpacing){
	_texture = LoadGLTexture(bitmapFont);
	_width = width;
	_height = height;
	_rows = rows;
	_columns = columns;
	_fontSize = fontSize;
	_vSpace = vertSpacing;
	_hSpace = horizSpacing;
	
	glGenVertexArrays(1, &_vao);
}

Text::~Text(){
	glDeleteTextures(1, &_texture);
	glDeleteVertexArrays(1, &_vao);
}


void Text::drawTextLine(char* text, float size){
	int numOfChars = strlen(text);


	GLfloat vertBuffer[numOfChars*4*4];
	GLfloat normals[numOfChars*4*3];
	GLfloat texCoord[numOfChars*4*2];

	GLuint indexes[numOfChars*6];

	for(int i=0; i<numOfChars; i++){

		//Vertex positions
		GLuint bufferIndex = i*4*4;
		vertBuffer[bufferIndex++] = (size/2)*i;			//x
		vertBuffer[bufferIndex++] = size; 			//y
		vertBuffer[bufferIndex++] = 0; 				//z
		vertBuffer[bufferIndex++] = 1.0; 			//w

		vertBuffer[bufferIndex++] = (size/2)*i;			//x
		vertBuffer[bufferIndex++] = 0; 				//y
		vertBuffer[bufferIndex++] = 0; 				//z
		vertBuffer[bufferIndex++] = 1.0; 			//w
		
		vertBuffer[bufferIndex++] = (size/2)*i + (size/2); 	//x
		vertBuffer[bufferIndex++] = 0; 				//y
		vertBuffer[bufferIndex++] = 0; 				//z
		vertBuffer[bufferIndex++] = 1.0; 			//w
		
		vertBuffer[bufferIndex++] = (size/2)*i + (size/2);	//x
		vertBuffer[bufferIndex++] = size;			//y
		vertBuffer[bufferIndex++] = 0; 				//z
		vertBuffer[bufferIndex++] = 1.0; 			//w

		//Normals
		bufferIndex = i*4*3;
		for(int j=0; j<4; j++){
			normals[bufferIndex++] = 0;
			normals[bufferIndex++] = 0;
			normals[bufferIndex++] = 1;
		}

		//Texture Coordinates
		char character = text[i];
		int charNum = character - 32; //32: ASCII for space
		if( charNum / _columns > _rows-1)
			return; //Not a valid char in this bitmap

		int topLeftY = (charNum / _columns) * (_fontSize+_vSpace); 
		int topLeftX = (charNum % _columns) * ((_fontSize/2)+_hSpace);
		
		int topRigthY = topLeftY;
		int topRigthX = topLeftX + (_fontSize/2);

		int bottomLeftY = topLeftY + _fontSize;
		int bottomLeftX = topLeftX;

		int bottomRightY = bottomLeftY;
		int bottomRightX = topRigthX;
		
		bufferIndex = i * 4 * 2;
		texCoord[bufferIndex++] = (float)topLeftX / (float)_width;
		texCoord[bufferIndex++] = (float)topLeftY / (float)_height;
		
		texCoord[bufferIndex++] = (float)bottomLeftX / (float)_width;
		texCoord[bufferIndex++] = (float)bottomLeftY / (float)_height;

		texCoord[bufferIndex++] = (float)bottomRightX / (float)_width;
		texCoord[bufferIndex++] = (float)bottomRightY / (float)_height;
		
		texCoord[bufferIndex++] = (float)topRigthX / (float)_width;
		texCoord[bufferIndex++] = (float)topRigthY / (float)_height;

		//Indexes
		bufferIndex = i*4;
		int indexesIndex = i*6;
		indexes[indexesIndex++] = bufferIndex;
		indexes[indexesIndex++] = bufferIndex+1;
		indexes[indexesIndex++] = bufferIndex+2;
		indexes[indexesIndex++] = bufferIndex+2;
		indexes[indexesIndex++] = bufferIndex+3;
		indexes[indexesIndex++] = bufferIndex;
	}


	GLuint vbo, eab;
	glBindVertexArray(_vao);
	glGenBuffers(1, &vbo);

	size_t positionSize = sizeof(GLfloat)*numOfChars*4*4;
	size_t normalsSize = sizeof(GLfloat)*numOfChars*4*3;
	size_t textCoordSize = sizeof(GLfloat)*numOfChars*4*2;

	size_t totalSize = positionSize + normalsSize + textCoordSize;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, totalSize, NULL, GL_STATIC_DRAW);

	//Copy the data
	glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, vertBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize, textCoordSize, texCoord);
	glBufferSubData(GL_ARRAY_BUFFER, positionSize + textCoordSize, normalsSize, normals);

	//Set up indices
	glGenBuffers(1, &eab);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*numOfChars*6, indexes, GL_STATIC_DRAW);

	//Set attributes
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)positionSize);
 	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(positionSize+textCoordSize) );
	glEnableVertexAttribArray(2);


        GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );

	glBindTexture(GL_TEXTURE_2D, _texture);
	glEnable(GL_TEXTURE_2D);

	//glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, numOfChars*6, GL_UNSIGNED_INT, 0);

	if ( texEnabled )
                glEnable( GL_TEXTURE_2D );
        else
                glDisable( GL_TEXTURE_2D );


	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &eab);
}
