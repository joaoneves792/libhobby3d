/*
 *	LXSR Load textures
 * 	adapted from multiple sources by E 
 *	Texture.h
 *
 *  Created on: Jun 2, 2012
 */
#ifndef TEXTURES_H_
#define TEXTURES_H_
#include <GL/glew.h>

typedef struct {
    int width;
    int height;
    bool alpha;
    unsigned char *data;
}textureImage;

GLuint LoadGLTexture( const char *filename );                    // Load Bitmaps And Convert To Textures
GLuint generateGLTexture(unsigned char* data, int height, int width, bool alpha);

#endif /* TEXTURES_H_*/
