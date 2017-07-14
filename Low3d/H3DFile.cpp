#include <string.h>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

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

    _groups = NULL;
    _armatures = NULL;
    _materials = NULL;

    _groupCount = 0;
    _materialCount = 0;
    _armatureCount = 0;

    _currentFrame = 0;

    _isAnimated = false;
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

    for(int i=0; i< _armatureCount; i++){
        delete[] _armatures[i].name;
    }

    delete[] _groups;
    _groups = NULL;
    delete[] _armatures;
    _armatures = NULL;

    _groupCount = 0;
    _materialCount = 0;
    _armatureCount = 0;
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
    GLfloat *bone_indices = new GLfloat[group->numVertices*BONE_COUNT];
    GLfloat *bone_weights = new GLfloat[group->numVertices*BONE_COUNT];

    indexInt *indices  = new indexInt[group->numTriangles*3];

    int vi=0;
    int ni=0;
    int ti=0;
    int bi=0;

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

        for(int k=0;k<BONE_COUNT;k++) {
            bone_indices[bi] = (GLfloat) group->vertices[i].boneID[k];
            bone_weights[bi++] = (GLfloat) group->vertices[i].weight[k];
        }
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
    _vboDescriptions[groupIndex].textureCoordSize = sizeof(GLfloat)*group->numVertices*2;
    _vboDescriptions[groupIndex].jointsSize       = sizeof(GLfloat)*group->numVertices*BONE_COUNT*2;
    _vboDescriptions[groupIndex].weightsSize      = sizeof(GLfloat)*group->numVertices*BONE_COUNT*2;

    _vboDescriptions[groupIndex].totalSize = _vboDescriptions[groupIndex].positionSize
                                             + _vboDescriptions[groupIndex].normalsSize
                                             + _vboDescriptions[groupIndex].textureCoordSize
                                             + _vboDescriptions[groupIndex].jointsSize
                                             + _vboDescriptions[groupIndex].weightsSize;

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
                    _vboDescriptions[groupIndex].jointsSize, bone_indices);

    glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize +
                                     _vboDescriptions[groupIndex].textureCoordSize +
                                     _vboDescriptions[groupIndex].normalsSize +
                                     _vboDescriptions[groupIndex].jointsSize,
                    _vboDescriptions[groupIndex].weightsSize, bone_weights);

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

    //bone indexes attribute
    glVertexAttribPointer(3, BONE_COUNT, GL_FLOAT, GL_FALSE, 0, (GLvoid *)
            (_vboDescriptions[groupIndex].positionSize +
             _vboDescriptions[groupIndex].textureCoordSize +
             _vboDescriptions[groupIndex].normalsSize) );
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, BONE_COUNT, GL_FLOAT, GL_FALSE, 0, (GLvoid *)
            (_vboDescriptions[groupIndex].positionSize +
             _vboDescriptions[groupIndex].textureCoordSize +
             _vboDescriptions[groupIndex].normalsSize +
             _vboDescriptions[groupIndex].jointsSize) );
    glEnableVertexAttribArray(4);

    //Free all the temporary memory
    delete[] vertices;
    delete[] indices;
    delete[] normals;
    delete[] texCoords;
    delete[] bone_indices;
    delete[] bone_weights;

}

void H3DFile::setCurrentFrame(int f) {
    _currentFrame = f;
}

void H3DFile::recursiveParentTransform(glm::mat4* transforms, bool* hasParentTransform, h3d_joint* joints, int jointIndex){
    int parentIndex =joints[jointIndex].parentIndex;
    if(parentIndex != -1 && !hasParentTransform[jointIndex]){
        recursiveParentTransform(transforms, hasParentTransform, joints, parentIndex);
        transforms[jointIndex] = transforms[parentIndex] * transforms[jointIndex];
        hasParentTransform[jointIndex] = true;
    }
}

glm::mat4 H3DFile::getBoneTransform(h3d_joint* joint) {
    h3d_keyframe* prevKeyframe = NULL;
    h3d_keyframe* nextKeyframe = NULL;

    for(int j=0; j<joint->numKeyframes; j++){
        nextKeyframe = &joint->keyframes[j];
        if(j>0){
            prevKeyframe = &joint->keyframes[j-1];
        }else{
            prevKeyframe = nextKeyframe;
        }
        if(nextKeyframe->frame > _currentFrame){
            break;
        }
    }


    if(joint->numKeyframes > 0) {
        float lerpFactor;
        if (nextKeyframe->frame - prevKeyframe->frame == 0) {
            lerpFactor = 0;
        } else {
            lerpFactor = ((float)(_currentFrame - prevKeyframe->frame)) /
                         ((float)(nextKeyframe->frame - prevKeyframe->frame));
        }
        if (lerpFactor < 0)
            lerpFactor = 0;
        if (lerpFactor > 1)
            lerpFactor = 1;

        return glm::interpolate(prevKeyframe->transform, nextKeyframe->transform, lerpFactor);

    }
    return glm::mat4(1.0f);
}


glm::mat4 H3DFile::getBindPose(h3d_joint* joint) {
    glm::mat4 bindPoseTranslation = glm::translate(glm::mat4(1.0f),
                                                   glm::vec3(joint->position[0],
                                                             joint->position[1],
                                                             joint->position[2]));

    glm::mat4 bindPoseRotation = glm::toMat4(glm::quat(glm::vec3(joint->rotation[0],
                                                                 joint->rotation[1],
                                                                 joint->rotation[2])));

    glm::mat4 bindPose = bindPoseTranslation * bindPoseRotation;
    return bindPose;
}

void H3DFile::handleAnimation(h3d_group* group) {
    //Check for the Uniform, if its none existent then do nothing
    GLint bones = glGetUniformLocation(_shader, "bones");
    if(-1 == bones)
        return;

    h3d_armature* armature = &_armatures[group->armatureIndex];

    glm::mat4 transforms[armature->jointsCount];
    for(int i=0; i<armature->jointsCount; i++){
        transforms[i] = glm::mat4(1.0f);
    }

    if(!group->isAnimated){ //If not animated the we still have to upload identities if the bones exist
        for(int i=0; i<armature->jointsCount; i++) {
            glUniformMatrix4fv(bones + i, 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        return;
    }


    for(int i=0; i<armature->jointsCount; i++){
        if(armature->joints[i].numKeyframes == 0)
            continue;
        glm::mat4 transform = getBoneTransform(&armature->joints[i]);

        glm::mat4 bindPose = armature->joints[i].bindPose;

        glm::mat4 invBindPose = armature->joints[i].invBindPose;

        transforms[i] = bindPose * transform * invBindPose;

    }

    //Recursively check and apply parent transforms
    bool hasParentTransform[armature->jointsCount];
    for(int i=0;i<armature->jointsCount;i++){
        hasParentTransform[i] = false;
    }

    for(int i=0; i<armature->jointsCount; i++) {
        recursiveParentTransform(transforms, hasParentTransform, armature->joints, i);
    }

    for(int i=0; i<armature->jointsCount; i++) {
        glUniformMatrix4fv(bones + i, 1, GL_FALSE, glm::value_ptr(transforms[i]));
    }


}

void H3DFile::drawGL2() {
#ifndef GLES
    for(int i=0; i < _groupCount; i++){
        handleAnimation(&_groups[i]);
        int materialIndex = _groups[i].materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(&_materials[materialIndex]);
        glBindVertexArray(_vao[i]);
        glDrawElements(GL_TRIANGLES, _groups[i].numTriangles*3, GL_UNSIGNED_INT, 0);
    }
#endif
}

void H3DFile::drawGLES2() {
    for(int i=0; i < _groupCount; i++){
        handleAnimation(&_groups[i]);
        int materialIndex = _groups[i].materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(&_materials[materialIndex]);

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


void H3DFile::setMaterialGL3(h3d_material* material){
    GLint ambient = glGetUniformLocation(_shader, "ambient");
    GLint diffuse = glGetUniformLocation(_shader, "diffuse");
    GLint specular = glGetUniformLocation(_shader, "specular");
    GLint emissive = glGetUniformLocation(_shader, "emissive");
    GLint shininess = glGetUniformLocation(_shader, "shininess");
    GLint transparency = glGetUniformLocation(_shader, "transparency");

    glUniform4fv(ambient, 1, material->ambient);
    glUniform4fv(diffuse, 1, material->diffuse);
    glUniform4fv(specular, 1, material->specular);
    glUniform4fv(emissive, 1, material->emissive);

    glUniform1f(shininess, material->shininess);
    glUniform1f(transparency, 1-(material->transparency));

    if( material->textureId >= 0){
        glBindTexture( GL_TEXTURE_2D, material->textureId);
    }
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

        //Get the material index
        fread(&_groups[i].materialIndex, 1, sizeof(int), fp);

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
            /*fread(&_groups[i].vertices[j], 1, sizeof(h3d_vertex), fp);*/
            fread(_groups[i].vertices[j].vertex, 3, sizeof(float), fp);
            fread(_groups[i].vertices[j].normal, 3, sizeof(float), fp);
            fread(_groups[i].vertices[j].uv, 2, sizeof(float), fp);
            for(int k=0;k<BONE_COUNT;k++) {
                fread(&_groups[i].vertices[j].boneID[k], 1, sizeof(int), fp);
                fread(&_groups[i].vertices[j].weight[k], 1, sizeof(float), fp);
            }
        }

        fread(&_groups[i].isAnimated, 1, sizeof(byte), fp);
        if(_groups[i].isAnimated){
            byte numChars;
            fread(&numChars, 1, sizeof(byte), fp);
            _groups[i].armatureName = new char[numChars+1];
            fread(_groups[i].armatureName, numChars, sizeof(char), fp);
            _groups[i].armatureName[numChars] = '\0';
        }

    }

    fread(&_materialCount, 1, sizeof(int), fp);
    _materials = new h3d_material[_materialCount];

    std::string folderPath(lpszFileName);
    folderPath = folderPath.substr(0, folderPath.find_last_of("/")+1);

    for(int i=0;i<_materialCount;i++){
        //Get the texture file
        byte numChars;
        fread(&numChars, 1, sizeof(byte), fp);
        _materials[i].textureImage = new char[numChars+1];
        if(numChars > 0) {
            fread(_materials[i].textureImage, numChars, sizeof(char), fp);
            _materials[i].textureImage[numChars] = '\0';

            //Load the texture
            std::string texturePath("./");
            texturePath.assign(folderPath);
            texturePath.append(_materials[i].textureImage);
            _materials[i].textureId = LoadGLTexture(texturePath.c_str());
        }else {
            _materials[i].textureId = -1;
        }

        fread(&_materials[i].ambient, 1, sizeof(float), fp);
        fread(&_materials[i].diffuse, 3, sizeof(float), fp);
        fread(&_materials[i].specular, 3, sizeof(float), fp);
        fread(&_materials[i].emissive, 3, sizeof(float), fp);
        fread(&_materials[i].shininess, 1, sizeof(float), fp);
        fread(&_materials[i].transparency, 1, sizeof(float), fp);
        _materials[i].ambient[1] = _materials[i].ambient[0];
        _materials[i].ambient[2] = _materials[i].ambient[0];
        _materials[i].ambient[3] = 1.0f;
        _materials[i].diffuse[3] = 1.0f;
        _materials[i].specular[3] = 1.0f;
        _materials[i].emissive[3] = 1.0f;
    }

    fread(&_armatureCount, 1, sizeof(int), fp);
    _armatures = new h3d_armature[_armatureCount];
    for(int i=0;i<_armatureCount;i++){
        //Get the Armature name file
        byte numChars;
        fread(&numChars, 1, sizeof(byte), fp);
        _armatures[i].name = new char[numChars+1];
        fread(_armatures[i].name, numChars, sizeof(char), fp);
        _armatures[i].name[numChars] = '\0';

        fread(&_armatures[i].jointsCount, 1, sizeof(int), fp);
        _armatures[i].joints = new h3d_joint[_armatures[i].jointsCount];

        for(int j=0;j<_armatures[i].jointsCount;j++){
            fread(&numChars, 1, sizeof(byte), fp);
            _armatures[i].joints[j].name = new char[numChars+1];
            fread(_armatures[i].joints[j].name, numChars, sizeof(char), fp);
            _armatures[i].joints[j].name[numChars] = '\0';

            fread(&_armatures[i].joints[j].position[0], 1, sizeof(float), fp);
            fread(&_armatures[i].joints[j].position[1], 1, sizeof(float), fp);
            fread(&_armatures[i].joints[j].position[2], 1, sizeof(float), fp);

            fread(&_armatures[i].joints[j].rotation[0], 1, sizeof(float), fp);
            fread(&_armatures[i].joints[j].rotation[1], 1, sizeof(float), fp);
            fread(&_armatures[i].joints[j].rotation[2], 1, sizeof(float), fp);

            fread(&_armatures[i].joints[j].parentIndex, 1, sizeof(int), fp);

            //Calculate the bindpose matrix
            _armatures[i].joints[j].bindPose = getBindPose(&_armatures[i].joints[j]);
            _armatures[i].joints[j].invBindPose = glm::inverse(_armatures[i].joints[j].bindPose);

            fread(&_armatures[i].joints[j].numKeyframes, 1, sizeof(int), fp);
            _armatures[i].joints[j].keyframes = new h3d_keyframe[_armatures[i].joints[j].numKeyframes];
            for(int k=0;k<_armatures[i].joints[j].numKeyframes;k++){
                fread(&_armatures[i].joints[j].keyframes[k].frame, 1, sizeof(int), fp);
                fread(_armatures[i].joints[j].keyframes[k].position, 3, sizeof(float), fp);
                fread(_armatures[i].joints[j].keyframes[k].rotation, 3, sizeof(float), fp);
                float x, y, z;
                h3d_keyframe* keyframe = &_armatures[i].joints[j].keyframes[k];

                x = keyframe->rotation[0];
                y = keyframe->rotation[1];
                z = keyframe->rotation[2];
                glm::vec3 euler = glm::vec3(x, y, z);

                x = keyframe->position[0];
                y = keyframe->position[1];
                z = keyframe->position[2];
                keyframe->transform = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z)) *
                                        glm::toMat4(glm::quat(euler));
            }

        }

    }

    for(int i=0;i<_groupCount;i++){
        if(_groups[i].isAnimated){
            for(int j=0;j<_armatureCount;j++){
                if(!strcmp(_groups[i].armatureName, _armatures[j].name)){
                    _groups[i].armatureIndex = j;
                }
            }
        }
    }


    return true;
}