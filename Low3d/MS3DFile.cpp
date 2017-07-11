/*
 * 	adapted from source at milkshape website by E 
 *	MS3DFile.cpp
 *
 *  Created on: Jun 3, 2012
 */
//#pragma warning(disable : 4786)
#ifdef GLES
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include "MS3DFile.h"
#include "MS3DFileI.h"
#include <iterator>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/type_ptr.inl>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

CMS3DFile::CMS3DFile()
{
	_i = new CMS3DFileI();
	_overrideAmbient = false;
	_overrideEmissive = false;
	_overrideDiffuse = false;
	_overrideSpecular = false;

	_white = new float [4];
	_black = new float [4];

	for(int i=0;i<4;i++)
		_white[i] = 1.0;
	for(int i=0;i<3;i++)
		_black[i] = 0.0;
	_black[3] = 1.0;

	_vao = NULL;
	_vbo = NULL;
	_eab = NULL;

	_isAnimated = false;

}

CMS3DFile::~CMS3DFile()
{
	Clear();
	delete _i;
	delete _white;
	delete _black;
}


void CMS3DFile::Clear()
{

	_i->arrVertices.clear();
	_i->arrTriangles.clear();
	_i->arrEdges.clear();
	_i->arrGroups.clear();
	_i->arrMaterials.clear();
	_i->arrJoints.clear();
	_i->arrTextures.clear();
}

int CMS3DFile::GetNumVertices()
{
	return (int) _i->arrVertices.size();
}

void CMS3DFile::GetVertexAt(int nIndex, ms3d_vertex_t **ppVertex)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrVertices.size())
		*ppVertex = &_i->arrVertices[nIndex];
}

int CMS3DFile::GetNumTriangles()
{
	return (int) _i->arrTriangles.size();
}

void CMS3DFile::GetTriangleAt(int nIndex, ms3d_triangle_t **ppTriangle)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrTriangles.size())
		*ppTriangle = &_i->arrTriangles[nIndex];
}

int CMS3DFile::GetNumEdges()
{
	return (int) _i->arrEdges.size();
}

void CMS3DFile::GetEdgeAt(int nIndex, ms3d_edge_t **ppEdge)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrEdges.size())
		*ppEdge = &_i->arrEdges[nIndex];
}

int CMS3DFile::GetNumGroups()
{
	return (int) _i->arrGroups.size();
}

void CMS3DFile::GetGroupAt(int nIndex, ms3d_group_t **ppGroup)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrGroups.size())
		*ppGroup = &_i->arrGroups[nIndex];
}

int CMS3DFile::FindGroupByName(const char* lpszName)
{
	for (size_t i = 0; i < _i->arrGroups.size(); i++)
		if (!strcmp(_i->arrGroups[i].name, lpszName))
			return i;
	return -1;
}
int CMS3DFile::GetNumMaterials()
{
	return (int) _i->arrMaterials.size();
}

void CMS3DFile::GetMaterialAt(int nIndex, ms3d_material_t **ppMaterial)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrMaterials.size())
		*ppMaterial = &_i->arrMaterials[nIndex];
}

int CMS3DFile::GetNumJoints()
{
	return (int) _i->arrJoints.size();
}

void CMS3DFile::GetJointAt(int nIndex, ms3d_joint_t **ppJoint)
{
	if (nIndex >= 0 && nIndex < (int) _i->arrJoints.size())
		*ppJoint = &_i->arrJoints[nIndex];
}

int CMS3DFile::FindJointByName(const char* lpszName)
{
	for (size_t i = 0; i < _i->arrJoints.size(); i++)
	{
		if (!strcmp(_i->arrJoints[i].name, lpszName))
			return i;
	}

	return -1;
}

float CMS3DFile::GetAnimationFPS()
{
	return _i->fAnimationFPS;
}

float CMS3DFile::GetCurrentTime()
{
	return _i->fCurrentTime;
}

int CMS3DFile::GetTotalFrames()
{
	return _i->iTotalFrames;
}

float lerp(float v0, float v1,float t){
    return (1 - t) * v0 + t * v1;
}

void CMS3DFile::enableAnimation(bool isAnimated) {
	_isAnimated = isAnimated;
}

void CMS3DFile::setAnimationTime(float t) {
    _i->fCurrentTime = t;
}

void CMS3DFile::recursiveParentTransform(glm::mat4* transforms, bool* hasParentTransform, int jointIndex){
    int parentIndex =_i->arrJoints[jointIndex].parentIndex;
	if(parentIndex != -1 && !hasParentTransform[jointIndex]){
		recursiveParentTransform(transforms, hasParentTransform, parentIndex);
		transforms[jointIndex] = transforms[parentIndex] * transforms[jointIndex];
		hasParentTransform[jointIndex] = true;
	}
}

glm::mat4 CMS3DFile::getBoneRotation(int i) {
	ms3d_keyframe_rot_t* prevRotKeyframe = NULL;
	ms3d_keyframe_rot_t* nextRotKeyframe = NULL;

	for(int j=0; j<_i->arrJoints[i].numKeyFramesRot; j++){
		nextRotKeyframe = &_i->arrJoints[i].keyFramesRot[j];
		if(j>0){
			prevRotKeyframe = &_i->arrJoints[i].keyFramesRot[j-1];
		}else{
			prevRotKeyframe = nextRotKeyframe;
		}
		if(nextRotKeyframe->time > _i->fCurrentTime){
			break;
		}
	}


	if(_i->arrJoints[i].numKeyFramesRot > 0) {
		float curTime = _i->fCurrentTime;
		float lerpFactor;
		if (nextRotKeyframe->time - prevRotKeyframe->time == 0) {
			lerpFactor = 0;
		} else {
			lerpFactor = (curTime - prevRotKeyframe->time) /
						 (nextRotKeyframe->time - prevRotKeyframe->time);
		}
		if (lerpFactor < 0)
			lerpFactor = 0;
		float rx, ry, rz;

		rx = nextRotKeyframe->rotation[0];
		ry = nextRotKeyframe->rotation[1];
		rz = nextRotKeyframe->rotation[2];
		glm::vec3 euler = glm::vec3(rx, ry, rz);
		glm::quat nextRotation = glm::quat(euler);


		rx = prevRotKeyframe->rotation[0];
		ry = prevRotKeyframe->rotation[1];
		rz = prevRotKeyframe->rotation[2];
		euler = glm::vec3(rx, ry, rz);
		glm::quat prevRotation = glm::quat(euler);

		return glm::toMat4(glm::lerp(prevRotation, nextRotation, lerpFactor));
	}
	return glm::mat4(1.0f);
}
glm::mat4 CMS3DFile::getBoneTranslation(int i) {
	ms3d_keyframe_pos_t* prevKeyframe = NULL;
	ms3d_keyframe_pos_t* nextKeyframe = NULL;

	for(int j=0; j<_i->arrJoints[i].numKeyFramesTrans; j++){
		nextKeyframe = &_i->arrJoints[i].keyFramesTrans[j];
		if(j>0){
			prevKeyframe = &_i->arrJoints[i].keyFramesTrans[j-1];
		}else{
			prevKeyframe = nextKeyframe;
		}
		if(nextKeyframe->time > _i->fCurrentTime){
			break;
		}
	}


	if(_i->arrJoints[i].numKeyFramesTrans > 0) {
		float curTime = _i->fCurrentTime;
		float lerpFactor;
		if (nextKeyframe->time - prevKeyframe->time == 0) {
			lerpFactor = 0;
		} else {
			lerpFactor = (curTime - prevKeyframe->time) /
						 (nextKeyframe->time - prevKeyframe->time);
		}
		if (lerpFactor < 0)
			lerpFactor = 0;
		float rx, ry, rz;

		rx = nextKeyframe->position[0];
		ry = nextKeyframe->position[1];
		rz = nextKeyframe->position[2];
		glm::mat4 nextTrans = glm::translate(glm::mat4(1.0f), glm::vec3(rx, ry, rz));


		rx = prevKeyframe->position[0];
		ry = prevKeyframe->position[1];
		rz = prevKeyframe->position[2];
		glm::mat4 prevTrans = glm::translate(glm::mat4(1.0f), glm::vec3(rx, ry, rz));

		return glm::interpolate(prevTrans, nextTrans, lerpFactor);
	}
	return glm::mat4(1.0f);
}

glm::mat4 CMS3DFile::recursiveBindPose(int i) {
	glm::mat4 bindPoseTranslation = glm::translate(glm::mat4(1.0f),
												   glm::vec3(_i->arrJoints[i].position[0],
															 _i->arrJoints[i].position[1],
															 _i->arrJoints[i].position[2]));

	glm::mat4 bindPoseRotation = glm::toMat4(glm::quat(glm::vec3(_i->arrJoints[i].rotation[0],
																 _i->arrJoints[i].rotation[1],
																 _i->arrJoints[i].rotation[2])));

	glm::mat4 bindPose = bindPoseTranslation * bindPoseRotation;
	int parentIndex = _i->arrJoints[i].parentIndex;
    if(parentIndex == -1){
		return bindPose;
	}
	return recursiveBindPose(parentIndex) * bindPose;
}

void CMS3DFile::handleAnimation() {
	//Check for the Uniform, if its none existent then do nothing
	GLint bones = glGetUniformLocation(_shader, "bones");
	if(-1 == bones)
		return;

    glm::mat4 transforms[_i->arrJoints.size()];
    for(size_t i=0; i<_i->arrJoints.size(); i++){
        transforms[i] = glm::mat4(1.0f);
    }

    if(!_isAnimated){ //If not animated the we still have to upload identities if the bones exist
        for(size_t i=0; i<_i->arrJoints.size(); i++) {
            glUniformMatrix4fv(bones + i, 1, GL_FALSE, glm::value_ptr(transforms[i]));
        }
        return;
    }


    for(size_t i=0; i<_i->arrJoints.size(); i++){
		if(_i->arrJoints[i].numKeyFramesRot == 0 ||
				_i->arrJoints[i].numKeyFramesTrans ==0 )
			continue;
		glm::mat4 rotation = getBoneRotation(i);
		glm::mat4 translation = getBoneTranslation(i);

		glm::mat4 bindPose = recursiveBindPose(i);

		glm::mat4 invBindPose = glm::inverse(bindPose);

		transforms[i] = bindPose * translation * rotation * invBindPose;
    }

	//Recursively check and apply parent transforms
	bool hasParentTransform[_i->arrJoints.size()];
	for(unsigned int i=0;i<_i->arrJoints.size();i++){
		hasParentTransform[i] = false;
	}

	for(size_t i=0; i<_i->arrJoints.size(); i++) {
		recursiveParentTransform(transforms, hasParentTransform, i);
	}

    for(size_t i=0; i<_i->arrJoints.size(); i++) {
        glUniformMatrix4fv(bones + i, 1, GL_FALSE, glm::value_ptr(transforms[i]));
    }


}

void CMS3DFile::draw(){
#ifndef GLES
	GLboolean texEnabled = glIsEnabled( GL_TEXTURE_2D );
	
	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 ){
			setMaterial(&(_i->arrMaterials[materialIndex]), materialIndex); 
		}else
			glDisable( GL_TEXTURE_2D );
		drawGroup(&(_i->arrGroups[i]));
	}		

	if ( texEnabled )
		glEnable( GL_TEXTURE_2D );
	else
		glDisable( GL_TEXTURE_2D );
#endif
}

void CMS3DFile::drawGL3(){
#ifndef GLES
    handleAnimation();
	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 )
			setMaterialGL3(&(_i->arrMaterials[materialIndex]),materialIndex); 
		glBindVertexArray(_vao[i]);
		glDrawElements(GL_TRIANGLES, _i->arrGroups[i].numtriangles*3, GL_UNSIGNED_INT, 0);
	}
#endif
}

void CMS3DFile::drawGLES2() {
   	handleAnimation();
	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		int materialIndex = (int)_i->arrGroups[i].materialIndex;
		if( materialIndex >= 0 )
			setMaterialGL3(&(_i->arrMaterials[materialIndex]),materialIndex);

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

		glDrawElements(GL_TRIANGLES, _i->arrGroups[i].numtriangles*3, GL_UNSIGNED_SHORT, 0);
	}
}

void CMS3DFile::setMaterialGL3(ms3d_material_t* material, int textureIndex){
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
	
	if( _i->arrTextures[textureIndex] > 0){
		glBindTexture( GL_TEXTURE_2D, _i->arrTextures[textureIndex]);
		//glEnable( GL_TEXTURE_2D );
	}//else
	//	glDisable( GL_TEXTURE_2D );
}

void CMS3DFile::setMaterial(ms3d_material_t* material, int textureIndex){
#ifndef GLES
	if(_overrideAmbient)
		glMaterialfv( GL_FRONT, GL_AMBIENT, _white);
	else
		glMaterialfv( GL_FRONT, GL_AMBIENT, material->ambient);
	if(_overrideDiffuse)
		glMaterialfv( GL_FRONT, GL_DIFFUSE, _white);
	else
		glMaterialfv( GL_FRONT, GL_DIFFUSE, material->diffuse );
	if(_overrideSpecular)
		glMaterialfv( GL_FRONT, GL_SPECULAR, _white );
	else
		glMaterialfv( GL_FRONT, GL_SPECULAR, material->specular );
	if(_overrideEmissive)
		glMaterialfv( GL_FRONT, GL_EMISSION, _black);
	else
		glMaterialfv( GL_FRONT, GL_EMISSION, material->emissive );

	glMaterialf( GL_FRONT, GL_SHININESS, material->shininess );
	if( _i->arrTextures[textureIndex] > 0){
		glBindTexture( GL_TEXTURE_2D, _i->arrTextures[textureIndex]);
		glEnable( GL_TEXTURE_2D );
	}else
		glDisable( GL_TEXTURE_2D );
#endif
}

//binds an opengl texture with the material information from a given group (usually to draw that same group)
void CMS3DFile::setMaterial(int texture, ms3d_group_t* group){
#ifndef GLES
	ms3d_material_t* material = &(_i->arrMaterials[group->materialIndex]);	
	if(_overrideAmbient)
		glMaterialfv( GL_FRONT, GL_AMBIENT, _white);
	else
		glMaterialfv( GL_FRONT, GL_AMBIENT, material->ambient);
	if(_overrideDiffuse)
		glMaterialfv( GL_FRONT, GL_DIFFUSE, _white);
	else
		glMaterialfv( GL_FRONT, GL_DIFFUSE, material->diffuse );
	if(_overrideSpecular)
		glMaterialfv( GL_FRONT, GL_SPECULAR, _white );
	else
		glMaterialfv( GL_FRONT, GL_SPECULAR, material->specular );
	if(_overrideEmissive)
		glMaterialfv( GL_FRONT, GL_EMISSION, _black);
	else
		glMaterialfv( GL_FRONT, GL_EMISSION, material->emissive );

	glMaterialf( GL_FRONT, GL_SHININESS, material->shininess );
	glBindTexture( GL_TEXTURE_2D, texture);
	glEnable( GL_TEXTURE_2D );
#endif
}

void CMS3DFile::setTexture(unsigned int textureIndex, int texture){
	if(textureIndex < _i->arrTextures.size())
		_i->arrTextures[textureIndex] = texture;
}

void CMS3DFile::setGLTexture(int texture, int group){
	int materialIndex = _i->arrGroups[group].materialIndex;
	_i->arrTextures[materialIndex] = texture;
}


void CMS3DFile::drawGroup(ms3d_group_t* group){
#ifndef GLES
	glBegin( GL_TRIANGLES );{
		int numTriangles = group->numtriangles;
		for(int j=0; j<numTriangles; j++){
			int triangleIndex = (int)group->triangleIndices[j];
			ms3d_triangle_t* tri = &(_i->arrTriangles[triangleIndex]);
			//Draw each vertex
			//for(int k=0;k<3;k++){
				//Aparently gcc would still do the for loop even with -O3 so we just expand it by hand
				glNormal3fv( tri->vertexNormals[0] );
				glTexCoord2f( tri->s[0], tri->t[0] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[0]].vertex );
			
				glNormal3fv( tri->vertexNormals[1] );
				glTexCoord2f( tri->s[1], tri->t[1] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[1]].vertex );
				
				glNormal3fv( tri->vertexNormals[2] );
				glTexCoord2f( tri->s[2], tri->t[2] );
				glVertex3fv( _i->arrVertices[tri->vertexIndices[2]].vertex );
			//}
		}
	}glEnd();
#endif
}

void CMS3DFile::unloadModel(){
	if(NULL != _vao && NULL != _eab && NULL != _vbo) {
		GLsizei numOfGroups = _i->arrGroups.size();
		glDeleteBuffers(numOfGroups, _eab);
		glDeleteBuffers(numOfGroups, _vbo);
#ifndef GLES
		glDeleteVertexArrays(numOfGroups, _vao);
#endif
	}
	for(unsigned int i=0; i< _i->arrTextures.size(); i++){
		glDeleteTextures(1, (GLuint *)&_i->arrTextures[i]);
	}

	Clear();
}

void CMS3DFile::prepareModel(GLuint shader){
	_shader = shader;

	//We have one vao and one vbo per group (is this the best approach?)
	_vbo = new GLuint [_i->arrGroups.size()];
	_eab = new GLuint [_i->arrGroups.size()];
#ifndef GLES
    //VAOs are not supported in standard GLES2.0
	_vao = new GLuint [_i->arrGroups.size()];
	glGenVertexArrays(_i->arrGroups.size(), _vao);
#endif

	_vboDescriptions = new vboDescription [_i->arrGroups.size()];

	for(unsigned int i=0; i < _i->arrGroups.size(); i++){
		prepareGroup(&(_i->arrGroups[i]), i, shader);
	}		

}
void CMS3DFile::prepareGroup(ms3d_group_t* group, unsigned int groupIndex, GLuint shader){
#ifndef GLES
	glBindVertexArray(_vao[groupIndex]);
#endif

	int numTriangles = group->numtriangles;

	long totalVertices = _i->arrVertices.size();
	indexInt* indexes_table = new indexInt[totalVertices]; //We allocate all temporary arrays on the heap because they can potentially be quite big!
	
	for(int i=0;i<totalVertices;i++)
		indexes_table[i] = (indexInt)-1;  // -1 means this vertex has not been used yet other value means its index


	GLfloat* vertices_position = new GLfloat[numTriangles*3*4];
	GLfloat* vertices_normals = new GLfloat[numTriangles*9];
	GLfloat* texture_coord = new GLfloat[numTriangles*6];
	GLfloat* joints = new GLfloat[numTriangles*3];

	int numVertices = numTriangles*3;
	//GLuint* indices = new GLuint[numVertices];
	//GLushort* indices = new GLushort[numVertices];
	indexInt* indices = new indexInt[numVertices];

	//Fill the arrays
	indexInt vertex_coordinate_index = 0;
	indexInt texture_coordinate_index = 0;
	indexInt normal_coordinate_index = 0;
	indexInt joints_index = 0;
	indexInt indices_index = 0; //Current index in the indexes table
	indexInt index = 0; //Actual index of the vertex
	for(int j=0; j<numTriangles; j++){
		int triangleIndex = (int)group->triangleIndices[j];
		ms3d_triangle_t* tri = &(_i->arrTriangles[triangleIndex]);
		for(int k = 0; k < 3; k++){
			indexInt existing_tex_coord_index = indexes_table[tri->vertexIndices[k]]*(indexInt)2;
			indexInt existing_normal_index = indexes_table[tri->vertexIndices[k]]*(indexInt)3;
			if( indexes_table[tri->vertexIndices[k]] == (indexInt)-1 || 					 //If we havent seen this vertex before
					(tri->s[k] != texture_coord[existing_tex_coord_index] 			 //Or if for some reason it has a different texture mapping
					 || tri->t[k] != texture_coord[existing_tex_coord_index+1]) || 		
					(tri->vertexNormals[k][0] != vertices_normals[existing_normal_index]     //Or if for some reason it has different normals...
					|| tri->vertexNormals[k][1] != vertices_normals[existing_normal_index+1] //create a new vertex (I know, comparing floats..., but what else can I do?)
					|| tri->vertexNormals[k][2] != vertices_normals[existing_normal_index+2]) ){

				vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[0]; 
				vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[1]; 
				vertices_position[vertex_coordinate_index++] = _i->arrVertices[tri->vertexIndices[k]].vertex[2];
				vertices_position[vertex_coordinate_index++] = 1.0;

				vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][0];
				vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][1];
				vertices_normals[normal_coordinate_index++] = tri->vertexNormals[k][2];

				texture_coord[texture_coordinate_index++] = tri->s[k];
				texture_coord[texture_coordinate_index++] = tri->t[k];

				joints[joints_index++] = (GLfloat)_i->arrVertices[tri->vertexIndices[k]].boneId;

				indexes_table[tri->vertexIndices[k]] = index; // change from -1 to the index of this vertex
				indices[indices_index++] = (indexInt)index;
				index++;
			}else{
				indices[indices_index++] = (indexInt)indexes_table[tri->vertexIndices[k]];
			}
		}
	}

	glGenBuffers(1, &_vbo[groupIndex]);

	_vboDescriptions[groupIndex].positionSize     = sizeof(GLfloat)*vertex_coordinate_index;
	_vboDescriptions[groupIndex].normalsSize      = sizeof(GLfloat)*normal_coordinate_index;
	_vboDescriptions[groupIndex].textureCoordSize = sizeof(GLfloat)*texture_coordinate_index;
	_vboDescriptions[groupIndex].jointsSize       = sizeof(GLfloat)*joints_index;

	_vboDescriptions[groupIndex].totalSize = _vboDescriptions[groupIndex].positionSize
											 + _vboDescriptions[groupIndex].normalsSize
											 + _vboDescriptions[groupIndex].textureCoordSize
			                                 + _vboDescriptions[groupIndex].jointsSize;

	glBindBuffer(GL_ARRAY_BUFFER, _vbo[groupIndex]);
	glBufferData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].totalSize, NULL, GL_STATIC_DRAW);

	/*Copy the data to the buffer*/
	glBufferSubData(GL_ARRAY_BUFFER, 0, _vboDescriptions[groupIndex].positionSize, vertices_position);
	glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize,
					_vboDescriptions[groupIndex].textureCoordSize, texture_coord);
	glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize +
			_vboDescriptions[groupIndex].textureCoordSize,
					_vboDescriptions[groupIndex].normalsSize, vertices_normals);
    glBufferSubData(GL_ARRAY_BUFFER, _vboDescriptions[groupIndex].positionSize +
                                     _vboDescriptions[groupIndex].textureCoordSize +
                                     _vboDescriptions[groupIndex].normalsSize,
                    _vboDescriptions[groupIndex].jointsSize, joints);

	//Set up the indices
	glGenBuffers(1, &_eab[groupIndex]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _eab[groupIndex]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexInt)*numVertices , indices, GL_STATIC_DRAW);


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

    /*Initialize the bones with identity matrixes
    GLint currentProgramId;
    glGetIntegerv(GL_CURRENT_PROGRAM,&currentProgramId); //Save and restore current used program
    glUseProgram(_shader);
    GLint bones = glGetUniformLocation(_shader, "bones");
    if(-1 != bones) {
        glm::mat4 identity = glm::mat4(1.0f);
        for(size_t i=0; i<_i->arrJoints.size() ;i++) {
            glUniformMatrix4fv(bones + (GLint )i, 1, GL_FALSE, glm::value_ptr(identity[0]));
        }
    }
    glUseProgram(currentProgramId);*/
	//Free all the temporary memory
	delete[] indexes_table;
	delete[] vertices_position;
	delete[] vertices_normals;
	delete[] texture_coord;
	delete[] indices;
	delete[] joints;
}

void CMS3DFile::setOverrideAmbient(bool overrideAmbient){
	_overrideAmbient = overrideAmbient;
}
void CMS3DFile::setOverrideDiffuse(bool overrideDiffuse){
	_overrideDiffuse = overrideDiffuse;
}
void CMS3DFile::setOverrideSpecular(bool overrideSpecular){
	_overrideSpecular = overrideSpecular;
}
void CMS3DFile::setOverrideEmissive(bool overrideEmissive){
	_overrideEmissive = overrideEmissive;
}

void CMS3DFile::optimize(){
	mergeGroups();
	removeUnusedMaterials();
}

void CMS3DFile::createRectangle(float width, float height, GLuint texture){

	float vertBuffer[6*3] = { 0,height,0, 0,0,0, width,0,0,  width,0,0, width,height,0, 0,height,0 };
	float normal[3] = { 0,0,1 };
	float texCoordBuffer[6*2] = { 0,0, 0,1, 1,1, 1,1, 1,0, 0,0 };

	int vertIndex = 0;

	word nNumVertices = 6;
	_i->arrVertices.resize(nNumVertices);
	ms3d_vertex_t* vert;
	for(int i=0; i<nNumVertices; i++){
		vert = &_i->arrVertices[i];
		vert->flags = 0;
		vert->boneId = -1;
		vert->vertex[0] = vertBuffer[vertIndex++];
		vert->vertex[1] = vertBuffer[vertIndex++];
		vert->vertex[2] = vertBuffer[vertIndex++];
	}
	
	vertIndex = 0;
	indexInt texIndex = 0;
	
	word nNumTriangles = 2;
	_i->arrTriangles.resize(nNumTriangles);
	ms3d_triangle_t* tri;
	for(int i=0; i<nNumTriangles; i++){
		tri = &_i->arrTriangles[i];
		tri->flags = 0;
		tri->vertexIndices[0] = (word)vertIndex++;
		tri->vertexIndices[1] = (word)vertIndex++;
		tri->vertexIndices[2] = (word)vertIndex++;

		for(int j=0; j<3; j++){
			tri->vertexNormals[j][0] = normal[0];
			tri->vertexNormals[j][1] = normal[1];
			tri->vertexNormals[j][2] = normal[2];
		}
		for(int j=0; j<3; j++){
			tri->s[j] = texCoordBuffer[texIndex++];
			tri->t[j] = texCoordBuffer[texIndex++];
		}
		tri->smoothingGroup = 1;
		tri->groupIndex = 0;
	}

	word nNumGroups = 1;
	_i->arrGroups.resize(nNumGroups);
	ms3d_group_t* group = &_i->arrGroups[0];
	group->flags = 0;
	group->name[0] = '1';
	group->name[1] = '\0';
	group->numtriangles = 2;
	group->triangleIndices = new word[2];
	group->triangleIndices[0] = 0;
	group->triangleIndices[1] = 1;
	group->materialIndex = 0;

	word nNumMaterials = 1;
	_i->arrMaterials.resize(nNumMaterials);
	ms3d_material_t* mat = &_i->arrMaterials[0];
	mat->name[0] = '1';
	mat->name[1] = '\0';
	for(int i=0; i<3; i++){
		mat->ambient[i] = 25;
		mat->diffuse[i] = 200;
		mat->specular[i] = 0;
		mat->emissive[i] = 0;
	}
	mat->ambient[3] = 0;
	mat->diffuse[3] = 0;
	mat->specular[3] = 0;
	mat->emissive[3] = 0;
	mat->shininess = 0;
	mat->transparency = 1;
	mat->mode = 0;
	mat->texture[0] = '\0';
	mat->alphamap[0] = '\0';
	
	_i->arrTextures.resize(1);
	_i->arrTextures[0] = texture;

	_i->arrJoints.resize(0);
}

void CMS3DFile::setMaterialEmissive(char* matName, float red, float green, float blue){
	int numMaterials = _i->arrMaterials.size();
	ms3d_material_t* mat;
	for(int i =0; i<numMaterials; i++){
		mat = &_i->arrMaterials[i];
		if(!strcmp(matName,mat->name)){
			mat->emissive[0] = red;
			mat->emissive[1] = green;
			mat->emissive[2] = blue;
		}
	}
}

void CMS3DFile::setMaterialTransparency(char* matName, float alpha){
	int numMaterials = _i->arrMaterials.size();
	ms3d_material_t* mat;
	for(int i =0; i<numMaterials; i++){
		mat = &_i->arrMaterials[i];
		if(!strcmp(matName,mat->name))
			mat->transparency = alpha;
	}
}


void CMS3DFile::translateModel(float x, float y, float z){
	for(unsigned int i = 0; i<_i->arrVertices.size(); i++){
		_i->arrVertices[i].vertex[0] += x;
		_i->arrVertices[i].vertex[1] += y;
		_i->arrVertices[i].vertex[2] += z;
	}
}
