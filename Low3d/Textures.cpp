/*
 *	LXSR Load textures
 * 	adapted from multiple sources by E 
 *	Texture.cpp
 *
 *  Created on: Jun 2, 2012
 */

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <iostream>
#include <jpeglib.h>
#include <png.h>

#include "Textures.h"

/*****************************************/
/*  Load Bitmaps, Jpegs, Pngs And Convert To Textures */
/*  Big mess of C and C++ code */

int loadPNG(const char* file_name, textureImage* texture){
    // This function was originally written by David Grayson for
    // https://github.com/DavidEGrayson/ahrs-visualizer
    //Adapted by Joao Neves
    //on 12-8-2015

    png_byte header[8];

    FILE *fp = fopen(file_name, "rb");
    if (fp == 0){
        perror(file_name);
        return 0;
    }
    // read the header
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)){
        fprintf(stderr, "error: %s is not a PNG.\n", file_name);
        fclose(fp);
        return 0;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr){
        fprintf(stderr, "error: png_create_read_struct returned 0.\n");
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr){
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        fclose(fp);
        return 0;
    }

    // create png info struct
    png_infop end_info = png_create_info_struct(png_ptr);
    if (!end_info){
        fprintf(stderr, "error: png_create_info_struct returned 0.\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp) NULL);
        fclose(fp);
        return 0;
    }

    // the code in this if statement gets called if libpng encounters an error
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "error from libpng\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // init png reading
    png_init_io(png_ptr, fp);

    // let libpng know you already read the first 8 bytes
    png_set_sig_bytes(png_ptr, 8);

    // read all the info up to the image data
    png_read_info(png_ptr, info_ptr);

    // variables to pass to get info
    int bit_depth, color_type;
    png_uint_32 temp_width, temp_height;

    // get info about png
    png_get_IHDR(png_ptr, info_ptr, &temp_width, &temp_height, &bit_depth, &color_type, NULL, NULL, NULL);

    
    texture->width = temp_width;
    texture->height = temp_height;
    
    if (bit_depth != 8){
        fprintf(stderr, "%s: Unsupported bit depth %d.  Must be 8.\n", file_name, bit_depth);
        return 0;
    }

    switch(color_type){
	case PNG_COLOR_TYPE_RGB:
        	//format = GL_RGB;
       		texture->alpha = false;
		break;
    	case PNG_COLOR_TYPE_RGB_ALPHA:
        	//format = GL_RGBA;
		texture->alpha = true;
        	break;
    	default:
        	fprintf(stderr, "%s: Unknown libpng color type %d.\n", file_name, color_type);
        	return 0;
    }

    // Update the png info struct.
    png_read_update_info(png_ptr, info_ptr);

    // Row size in bytes.
    int rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // glTexImage2d requires rows to be 4-byte aligned
    rowbytes += 3 - ((rowbytes-1) % 4);

    // Allocate the image_data as a big block, to be given to opengl
    texture->data = (png_byte *)malloc(rowbytes * temp_height * sizeof(png_byte)+15);
    if (texture->data == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG image data\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        fclose(fp);
        return 0;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
    png_byte ** row_pointers = (png_byte **)malloc(temp_height * sizeof(png_byte *));
    if (row_pointers == NULL)
    {
        fprintf(stderr, "error: could not allocate memory for PNG row pointers\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        free(texture->data);
        fclose(fp);
        return 0;
    }

    // set the individual row_pointers to point at the correct offsets of image_data
    for (unsigned int i = 0; i < temp_height; i++)
    {
        //row_pointers[temp_height - 1 - i] = texture->data + i * rowbytes;
        row_pointers[temp_height - 1 - i] = texture->data + (temp_height - 1 - i) * rowbytes;
    }

    // read the png into image_data through row_pointers
    png_read_image(png_ptr, row_pointers);


    // clean up
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    free(row_pointers);
    fclose(fp);
    return 1;
}

int loadJPEG(const char* filename, textureImage* texture){
	FILE* infile;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride;     /* physical row width in output buffer */
	long counter = 0;
	int buffer_height = 1;

	texture->alpha = false;
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	if ((infile = fopen(filename, "rb")) == NULL) {
            fprintf(stderr, "can't open %s\n", filename);
            exit(1);
        }
        jpeg_stdio_src(&cinfo, infile);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
	
	texture->width = cinfo.output_width;
	texture->height = cinfo.output_height;
	texture->data = new unsigned char[cinfo.output_width*cinfo.output_height*cinfo.output_components];

	row_stride = cinfo.output_width * cinfo.output_components;
    	

	JSAMPARRAY buffer = (JSAMPARRAY)malloc(sizeof(JSAMPROW)*buffer_height);


	/* Make a one-row-high sample array that will go away when done with image */
    	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	buffer[0] = (JSAMPROW)malloc(sizeof(JSAMPLE) * row_stride);

	while (cinfo.output_scanline < cinfo.output_height) {
		jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(texture->data+counter, buffer[0], row_stride);
		counter += row_stride;
	}

	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	fclose(infile);	
	return 1;
}


/* simple loader for 24bit bitmaps (data is in rgb-format) */
int loadBMP(const char *filename, textureImage *texture)
{
    FILE *file;
    unsigned short int bfType;
    long int bfOffBits;
    short int biPlanes;
    short int biBitCount;
    long int biSizeImage;
    int i;
    unsigned char temp;

	texture->alpha = false;

    /* make sure the file is there and open it read-only (binary) */
    if ((file = fopen(filename, "rb")) == NULL)
    {
        printf("File not found : %s\n", filename);
        return 0;
    }
    if(!fread(&bfType, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    /* check if file is a bitmap */
    if (bfType != 19778)
    {
        printf("Not a Bitmap-File!\n");
        return 0;
    }        
    /* get the file size */
    /* skip file size and reserved fields of bitmap file header */
    fseek(file, 8, SEEK_CUR);
    /* get the position of the actual bitmap data */
    if (!fread(&bfOffBits, sizeof(long int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Data at Offset: %ld\n", bfOffBits);
    /* skip size of bitmap info header */
    fseek(file, 4, SEEK_CUR);
    /* get the width of the bitmap */
    fread(&texture->width, sizeof(int), 1, file);
    printf("Width of Bitmap: %d\n", texture->width);
    /* get the height of the bitmap */
    fread(&texture->height, sizeof(int), 1, file);
    printf("Height of Bitmap: %d\n", texture->height);
    /* get the number of planes (must be set to 1) */
    fread(&biPlanes, sizeof(short int), 1, file);
    if (biPlanes != 1)
    {
        printf("Error: number of Planes not 1!\n");
        return 0;
    }
    /* get the number of bits per pixel */
    if (!fread(&biBitCount, sizeof(short int), 1, file))
    {
        printf("Error reading file!\n");
        return 0;
    }
    printf("Bits per Pixel: %d\n", biBitCount);
    if (biBitCount != 24)
    {
        printf("Bits per Pixel not 24\n");
        return 0;
    }
    /* calculate the size of the image in bytes */
    biSizeImage = texture->width * texture->height * 3;
    printf("Size of the image data: %ld\n", biSizeImage);
    texture->data = (unsigned char *)malloc(biSizeImage);
    /* seek to the actual data */
    fseek(file, bfOffBits, SEEK_SET);
    if (!fread(texture->data, biSizeImage, 1, file))
    {
        printf("Error loading file!\n");
        return 0;
    }
    /* swap red and blue (bgr -> rgb) */
    for (i = 0; i < biSizeImage; i += 3)
    {
        temp = texture->data[i];
        texture->data[i] = texture->data[i + 2];
        texture->data[i + 2] = temp;
    }
    return 1;
}

GLuint generateGLTexture(unsigned char* data, int height, int width, bool alpha){
	GLuint texID = 0;
     	if(data){
        	glGenTextures(1, &texID);   /* create the texture */
        	glBindTexture(GL_TEXTURE_2D, texID);
        	/* actually generate the texture */
        	glTexImage2D(GL_TEXTURE_2D, 0, (alpha)?GL_RGBA:GL_RGB, width, height, 0,
        	    (alpha)?GL_RGBA:GL_RGB, GL_UNSIGNED_BYTE, data);
        	/* enable linear filtering */
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    	}
	return texID;
}

GLuint LoadGLTexture(const char* name){
	textureImage *texti;
	std::string fn(name);
	GLuint textureID = 0;

	texti = (textureImage *)malloc(sizeof(textureImage));

	if(!fn.substr(fn.find_last_of(".")+1).compare("bmp"))
		loadBMP(name, texti);
	else if(!fn.substr(fn.find_last_of(".")+1).compare("jpg") || !fn.substr(fn.find_last_of(".")+1).compare("jpeg") )   
		loadJPEG(name, texti);
	else if(!fn.substr(fn.find_last_of(".")+1).compare("png"))
		loadPNG(name, texti);
	else
		return 0;
	
	if(texti)
		textureID = generateGLTexture(texti->data, texti->height, texti->width, texti->alpha);

	/* free the ram we used in our texture generation process */
        if (texti->data)
        	free(texti->data);
        free(texti);

	return textureID;
}
