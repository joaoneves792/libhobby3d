/*
 *	LXSR Load textures
 * 	adapted from multiple sources by E 
 *	Texture.h
 *
 *  Created on: Jun 2, 2012
 */
#ifndef TEXTURES_H_
#define TEXTURES_H_

#ifdef GLES
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif

typedef struct {
    int width;
    int height;
    bool alpha;
    bool compressed;
    GLsizei data_lenght;
    unsigned char *data;
}textureImage;


GLuint LoadGLTexture( const char *filename );                    // Load Bitmaps And Convert To Textures
GLuint generateGLTexture(unsigned char* data, int height, int width, bool alpha, bool compressed, GLsizei lenght);

#endif /* TEXTURES_H_*/
