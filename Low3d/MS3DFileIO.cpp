#include "MS3DFile.h"
#include "MS3DFileI.h"
#include "Textures.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <set>


#define MAKEDWORD(a, b)      ((unsigned int)(((word)(a)) | ((word)((word)(b))) << 16))

bool CMS3DFile::LoadFromFile(const char* lpszFileName)
{
	FILE *fp = fopen(lpszFileName, "rb");
	if (!fp)
		return false;

	size_t i;
	ms3d_header_t header;
	fread(&header, 1, sizeof(ms3d_header_t), fp);

	if (strncmp(header.id, "MS3D000000", 10) != 0)
		return false;

	if (header.version != 4)
		return false;

	// vertices
	word nNumVertices;
	fread(&nNumVertices, 1, sizeof(word), fp);
	_i->arrVertices.resize(nNumVertices);
	fread(&_i->arrVertices[0], nNumVertices, sizeof(ms3d_vertex_t), fp);

	// triangles
	word nNumTriangles;
	fread(&nNumTriangles, 1, sizeof(word), fp);
	_i->arrTriangles.resize(nNumTriangles);
	fread(&_i->arrTriangles[0], nNumTriangles, sizeof(ms3d_triangle_t), fp);

	// edges
	std::set<unsigned int> setEdgePair;
	for (i = 0; i < _i->arrTriangles.size(); i++)
	{
		word a, b;
		a = _i->arrTriangles[i].vertexIndices[0];
		b = _i->arrTriangles[i].vertexIndices[1];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = _i->arrTriangles[i].vertexIndices[1];
		b = _i->arrTriangles[i].vertexIndices[2];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));

		a = _i->arrTriangles[i].vertexIndices[2];
		b = _i->arrTriangles[i].vertexIndices[0];
		if (a > b)
			std::swap(a, b);
		if (setEdgePair.find(MAKEDWORD(a, b)) == setEdgePair.end())
			setEdgePair.insert(MAKEDWORD(a, b));
	}

	for(std::set<unsigned int>::iterator it = setEdgePair.begin(); it != setEdgePair.end(); ++it)
	{
		unsigned int EdgePair = *it;
		ms3d_edge_t Edge;
		Edge.edgeIndices[0] = (word) EdgePair;
		Edge.edgeIndices[1] = (word) ((EdgePair >> 16) & 0xFFFF);
		_i->arrEdges.push_back(Edge);
	}

	// groups
	word nNumGroups;
	fread(&nNumGroups, 1, sizeof(word), fp);
	_i->arrGroups.resize(nNumGroups);
	for (i = 0; i < nNumGroups; i++)
	{
		fread(&_i->arrGroups[i].flags, 1, sizeof(byte), fp);
		fread(&_i->arrGroups[i].name, 32, sizeof(char), fp);
		fread(&_i->arrGroups[i].numtriangles, 1, sizeof(word), fp);
		_i->arrGroups[i].triangleIndices = new word[_i->arrGroups[i].numtriangles];
		fread(_i->arrGroups[i].triangleIndices, _i->arrGroups[i].numtriangles, sizeof(word), fp);
		fread(&_i->arrGroups[i].materialIndex, 1, sizeof(char), fp);
	}

	// materials
	word nNumMaterials;
	fread(&nNumMaterials, 1, sizeof(word), fp);
	_i->arrMaterials.resize(nNumMaterials);
	fread(&_i->arrMaterials[0], nNumMaterials, sizeof(ms3d_material_t), fp);

	fread(&_i->fAnimationFPS, 1, sizeof(float), fp);
	fread(&_i->fCurrentTime, 1, sizeof(float), fp);
	fread(&_i->iTotalFrames, 1, sizeof(int), fp);

	// joints
	word nNumJoints;
	fread(&nNumJoints, 1, sizeof(word), fp);
	_i->arrJoints.resize(nNumJoints);
	for (i = 0; i < nNumJoints; i++)
	{
		fread(&_i->arrJoints[i].flags, 1, sizeof(byte), fp);
		fread(&_i->arrJoints[i].name, 32, sizeof(char), fp);
		fread(&_i->arrJoints[i].parentName, 32, sizeof(char), fp);
		fread(&_i->arrJoints[i].rotation, 3, sizeof(float), fp);
		fread(&_i->arrJoints[i].position, 3, sizeof(float), fp);
		fread(&_i->arrJoints[i].numKeyFramesRot, 1, sizeof(word), fp);
		fread(&_i->arrJoints[i].numKeyFramesTrans, 1, sizeof(word), fp);
		_i->arrJoints[i].keyFramesRot = new ms3d_keyframe_rot_t[_i->arrJoints[i].numKeyFramesRot];
		_i->arrJoints[i].keyFramesTrans = new ms3d_keyframe_pos_t[_i->arrJoints[i].numKeyFramesTrans];
		fread(_i->arrJoints[i].keyFramesRot, _i->arrJoints[i].numKeyFramesRot, sizeof(ms3d_keyframe_rot_t), fp);
		fread(_i->arrJoints[i].keyFramesTrans, _i->arrJoints[i].numKeyFramesTrans, sizeof(ms3d_keyframe_pos_t), fp);
	}

	fclose(fp);


	//Finally we must read all the textures and save their opengl ids
	_i->arrTextures.resize(nNumMaterials);//One texture per material (if no texture is used id=0)
	std::string folderPath(lpszFileName);
        std::string texturePath("./");
        folderPath = folderPath.substr(0, folderPath.find_last_of("/")+1);


	for(int i=0; i<nNumMaterials; i++){
		if(!strlen(_i->arrMaterials[i].texture)){
			_i->arrTextures[i] = 0;
		}else{
			texturePath.assign(folderPath);
			texturePath.append(_i->arrMaterials[i].texture);
			_i->arrTextures[i] = LoadGLTexture(texturePath.c_str());
		}
	}
	
	return true;
}

bool CMS3DFile::SaveToFile(const char* lpszFileName)
{
	FILE *fp = fopen(lpszFileName, "w");
	if (!fp)
		return false;

	//header
	ms3d_header_t header;
	header.id[0] = 'M';
	header.id[1] = 'S';
	header.id[2] = '3';
	header.id[3] = 'D';
	header.id[4] = '0';
	header.id[5] = '0';
	header.id[6] = '0';
	header.id[7] = '0';
	header.id[8] = '0';
	header.id[9] = '0';
	header.version = 4;
	fwrite(&header, sizeof(ms3d_header_t), 1, fp);

	// vertices
	word nNumVertices = _i->arrVertices.size();
	fwrite(&nNumVertices, sizeof(word), 1, fp);
	fwrite(&_i->arrVertices[0], sizeof(ms3d_vertex_t), nNumVertices, fp);

	
	// triangles
	word nNumTriangles = _i->arrTriangles.size();
	fwrite(&nNumTriangles, sizeof(word), 1, fp);
	fwrite(&_i->arrTriangles[0], sizeof(ms3d_triangle_t), nNumTriangles, fp);	
	
	
	// groups
	word nNumGroups = _i->arrGroups.size();
	fwrite(&nNumGroups, sizeof(word), 1, fp);

	word i;
	for (i = 0; i < nNumGroups; i++)
	{
		fwrite(&_i->arrGroups[i].flags, sizeof(byte), 1, fp);
		fwrite(&_i->arrGroups[i].name, sizeof(char), 32, fp);
		fwrite(&_i->arrGroups[i].numtriangles, sizeof(word), 1, fp);
		fwrite(_i->arrGroups[i].triangleIndices, sizeof(word), _i->arrGroups[i].numtriangles, fp);
		fwrite(&_i->arrGroups[i].materialIndex, sizeof(char), 1, fp);
	}

	
	// materials
	word nNumMaterials = _i->arrMaterials.size();
	fwrite(&nNumMaterials, sizeof(word), 1, fp);
	fwrite(&_i->arrMaterials[0], sizeof(ms3d_material_t), nNumMaterials, fp);

	
	fwrite(&_i->fAnimationFPS, sizeof(float), 1, fp);
	fwrite(&_i->fCurrentTime, sizeof(float), 1, fp);
	fwrite(&_i->iTotalFrames, sizeof(int), 1, fp);
	

	// joints
	word nNumJoints = _i->arrJoints.size();
	fwrite(&nNumJoints, sizeof(word), 1, fp);
	for (i = 0; i < nNumJoints; i++)
	{
		fwrite(&_i->arrJoints[i].flags, sizeof(byte), 1, fp);
		fwrite(&_i->arrJoints[i].name, sizeof(char), 32, fp);
		fwrite(&_i->arrJoints[i].parentName, sizeof(char), 32, fp);
		fwrite(&_i->arrJoints[i].rotation, sizeof(float), 3, fp);
		fwrite(&_i->arrJoints[i].position, sizeof(float), 3, fp);
		fwrite(&_i->arrJoints[i].numKeyFramesRot, sizeof(word), 1, fp);
		fwrite(&_i->arrJoints[i].numKeyFramesTrans, sizeof(word), 1, fp);
		fwrite(_i->arrJoints[i].keyFramesRot, sizeof(ms3d_keyframe_rot_t), _i->arrJoints[i].numKeyFramesRot, fp);
		fwrite(_i->arrJoints[i].keyFramesTrans, sizeof(ms3d_keyframe_pos_t), _i->arrJoints[i].numKeyFramesTrans, fp);
	}

	fclose(fp);
	return true;
}
