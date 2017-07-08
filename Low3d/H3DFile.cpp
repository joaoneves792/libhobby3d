#include <string.h>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef GLES
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include "H3DFile.h"

#include "Textures.h"

H3DFile::H3DFile() {
    _vao = NULL;
    _vbo = NULL;
    _eab = NULL;
}

H3DFile::~H3DFile() {
    if(NULL != _groups)
        unloadModel();
}

void H3DFile::Clear() {
    //TODO actually free all the memory properly
    for(int i=0; i< _groupCount; i++){
        delete[] _groups[i].name;
        delete[] _groups[i].vertices;
        delete[] _groups[i].triangles;
    }
    delete[] _groups;
    _groups = NULL;
}

void H3DFile::unloadModel() {
    if(NULL != _eab && NULL != _vbo) {
        glDeleteBuffers(_groupCount, _eab);
        glDeleteBuffers(_groupCount, _vbo);
        _eab = NULL;
        _vbo = NULL;
#ifndef GLES
        if(NULL != _vao) {
            glDeleteVertexArrays(_groupCount, _vao);
            _vao = NULL;
        }
#endif

    }
    /*for(unsigned int i=0; i< _mesh->materials.size(); i++){
        glDeleteTextures(1, (GLuint *)&_mesh->materials[i]->textureId);
    }*/

    Clear();
}

void H3DFile::prepareModel(GLuint shader) {
    _shader = shader;

    //We have one vao and one vbo per group (is this the best approach?)
    _vbo = new GLuint [_groupCount];
    _eab = new GLuint [_groupCount];
#ifndef GLES
    //VAOs are not supported in standard GLES2.0
    _vao = new GLuint [_groupCount];
    glGenVertexArrays(_groupCount, _vao);
#endif

    _vboDescriptions = new h3d_vboDescription[_groupCount];

    for(int i=0; i < _groupCount; i++){
            prepareGroup(&_groups[i], i, shader);
    }
}


void H3DFile::prepareGroup(h3d_group *group, unsigned int groupIndex, GLuint shader) {
#ifndef GLES
    glBindVertexArray(_vao[groupIndex]);
#endif

    GLfloat *vertices  = new GLfloat[group->numVertices*4];
    GLfloat *normals   = new GLfloat[group->numVertices*3];
    GLfloat *texCoords = new GLfloat[group->numVertices*2];
    GLfloat *joints    = new GLfloat[0];

    indexInt *indices  = new indexInt[group->numTriangles*3];

    int vi=0;
    int ni=0;
    int ti=0;
    for(int i=0;i<group->numVertices;i++){
        vertices[vi++] = group->vertices[i].vertex[0];
        vertices[vi++] = group->vertices[i].vertex[1];
        vertices[vi++] = group->vertices[i].vertex[2];
        vertices[vi++] = 1.0;

        normals[ni++] = group->vertices[i].normal[0];
        normals[ni++] = group->vertices[i].normal[1];
        normals[ni++] = group->vertices[i].normal[2];

        texCoords[ti++] = group->vertices[i].uv[0];
        texCoords[ti++] = group->vertices[i].uv[1];
    }


    int ii=0;
    for(int i=0; i<group->numTriangles;i++){
        indices[ii++] = group->triangles[i].vertexIndices[0];
        indices[ii++] = group->triangles[i].vertexIndices[1];
        indices[ii++] = group->triangles[i].vertexIndices[2];
    }

    glGenBuffers(1, &_vbo[groupIndex]);

    //_vboDescriptions[groupIndex].positionSize     = sizeof(GLfloat)*group->verticesPositionCount;
    _vboDescriptions[groupIndex].positionSize     = sizeof(GLfloat)*group->numVertices*4;
    _vboDescriptions[groupIndex].normalsSize      = sizeof(GLfloat)*group->numVertices*3;
    _vboDescriptions[groupIndex].textureCoordSize = sizeof(GLfloat)*group->numTriangles*2;
    _vboDescriptions[groupIndex].jointsSize       = sizeof(GLfloat)*0;

    _vboDescriptions[groupIndex].totalSize = _vboDescriptions[groupIndex].positionSize
                                             + _vboDescriptions[groupIndex].normalsSize
                                             + _vboDescriptions[groupIndex].textureCoordSize
                                             + _vboDescriptions[groupIndex].jointsSize;

    glBindBuffer(GL_ARRAY_BUFFER, _vbo[groupIndex]);
    glBufferData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].totalSize, NULL, GL_STATIC_DRAW);

    /*Copy the data to the buffer*/
    //glBufferSubData(GL_ARRAY_BUFFER, 0, _vboDescriptions[groupIndex].positionSize, group->verticesPosition);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _vboDescriptions[groupIndex].positionSize, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize,
                    _vboDescriptions[groupIndex].textureCoordSize, texCoords);
    glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize +
                                     _vboDescriptions[groupIndex].textureCoordSize,
                    _vboDescriptions[groupIndex].normalsSize, normals);
    glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize +
                                     _vboDescriptions[groupIndex].textureCoordSize +
                                     _vboDescriptions[groupIndex].normalsSize,
                    _vboDescriptions[groupIndex].jointsSize, joints);

    //Set up the indices
    glGenBuffers(1, &_eab[groupIndex]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eab[groupIndex]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexInt)*ii , indices, GL_STATIC_DRAW);


    //Position Attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
    glEnableVertexAttribArray(0);

    //Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)_vboDescriptions[groupIndex].positionSize);
    glEnableVertexAttribArray(1);

    //Normals attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)
            (_vboDescriptions[groupIndex].positionSize+
             _vboDescriptions[groupIndex].textureCoordSize) );
    glEnableVertexAttribArray(2);

    //Joints attribute
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid *)
            (_vboDescriptions[groupIndex].positionSize +
             _vboDescriptions[groupIndex].textureCoordSize +
             _vboDescriptions[groupIndex].normalsSize) );
    glEnableVertexAttribArray(3);

    //Free all the temporary memory
    delete[] vertices;
    delete[] indices;
    delete[] normals;
    delete[] texCoords;
    delete[] joints;

}


void H3DFile::drawGL2() {
#ifndef GLES
    //handleAnimation();
    for(int i=0; i < _groupCount; i++){
        /*int materialIndex = (int)_mesh->groups[i]->materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(_mesh->materials[materialIndex]);*/
        glBindVertexArray(_vao[i]);
        glDrawElements(GL_TRIANGLES, _groups[i].numTriangles*3, GL_UNSIGNED_INT, 0);
    }
#endif
}

void H3DFile::drawGLES2() {
    //handleAnimation();
    for(int i=0; i < _groupCount; i++){
        /*int materialIndex = (int)_mesh->groups[i]->materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(_mesh->materials[materialIndex]);*/

        glBindBuffer(GL_ARRAY_BUFFER, _vbo[i]); //pos normal etc
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eab[i]);

        //Position Attribute
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, 0);
        glEnableVertexAttribArray(0);

        //Texture coord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)_vboDescriptions[i].positionSize);
        glEnableVertexAttribArray(1);

        //Normals attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(_vboDescriptions[i].positionSize+
                                                                      _vboDescriptions[i].textureCoordSize) );
        glEnableVertexAttribArray(2);

        //Joints Index attribute
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(_vboDescriptions[i].positionSize+
                                                                      _vboDescriptions[i].textureCoordSize+
                                                                      _vboDescriptions[i].normalsSize) );
        glEnableVertexAttribArray(3);

        glDrawElements(GL_TRIANGLES, _groups[i].numTriangles*3, GL_UNSIGNED_SHORT, 0);
    }
}


void H3DFile::setMaterialGL3(){
    /*GLint ambient = glGetUniformLocation(_shader, "ambient");
    GLint diffuse = glGetUniformLocation(_shader, "diffuse");
    GLint specular = glGetUniformLocation(_shader, "specular");
    GLint emissive = glGetUniformLocation(_shader, "emissive");
    GLint shininess = glGetUniformLocation(_shader, "shininess");
    GLint transparency = glGetUniformLocation(_shader, "transparency");

    glUniform4fv(ambient, 1, material->ambient);
    glUniform4fv(diffuse, 1, material->diffuse);
    glUniform4fv(specular, 1, material->specular);
    glUniform4fv(emissive, 1, material->emission);

    glUniform1f(shininess, material->shininess);
    //TODO
    //glUniform1f(transparency, 1-(material->transparency));

    if( material->textureId >= 0){
        glBindTexture( GL_TEXTURE_2D, material->textureId);
    }*/
}


bool H3DFile::LoadFromFile(const char *lpszFileName) {
    FILE *fp = fopen(lpszFileName, "rb");
    if (!fp)
        return false;

    h3d_header header;
    fread(&header, 1, sizeof(h3d_header), fp);

    if (strncmp(header.id, "H3D", 3) != 0)
        return false;

    if (header.version != 1)
        return false;

    int numGroups;
    fread(&numGroups, 1, sizeof(int), fp);

    _groupCount = numGroups;
    _groups = new h3d_group[_groupCount];

    for(int i=0;i<numGroups;i++){

        //Get the name
        byte numChars;
        fread(&numChars, 1, sizeof(byte), fp);
        _groups[i].name = new char[numChars+1];
        fread(_groups[i].name, numChars, sizeof(char), fp);
        _groups[i].name[numChars] = '\0';

        //Read the triangles and their vertex indexes
        fread(&_groups[i].numTriangles, 1, sizeof(int), fp);
        _groups[i].triangles = new h3d_triangle[_groups[i].numTriangles];
        for(int j=0; j<_groups[i].numTriangles;j++){
            fread(&_groups[i].triangles[j], 1, sizeof(h3d_triangle), fp);
        }

        //Read the vertex array (vertexes include their normal and UV)
        fread(&_groups[i].numVertices, 1, sizeof(int), fp);
        _groups[i].vertices = new h3d_vertex[_groups[i].numVertices];
        for(int j=0; j<_groups[i].numVertices; j++){
            fread(&_groups[i].vertices[j], 1, sizeof(h3d_vertex), fp);
        }

    }

    return true;
}