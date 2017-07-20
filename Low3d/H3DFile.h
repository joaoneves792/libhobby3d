/*
 *  Created on: Jul 7, 2017
 */


#ifndef _H3DFILE_H_
#define _H3DFILE_H_

#ifdef GLES
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

//#include <pshpack1.h>

#ifndef RC_INVOKED
#pragma pack(push,1)
#endif

#include <vector>
#include <glm/detail/type_mat.hpp>
#include <glm/gtx/quaternion.hpp>

#ifndef byte
typedef unsigned char byte;
#endif // byte

#ifndef word
typedef unsigned short word;
#endif // word

#define BONE_COUNT 3

typedef struct
{
    char    id[3];                                     // always "H3D"
    char    version;                                   // 1
} h3d_header;

typedef struct
{
    float   vertex[3];
	float 	normal[3];
	float 	uv[2];
	int     boneID[BONE_COUNT];                                    // -1 = no bone
    float   weight[BONE_COUNT];
} h3d_vertex;

typedef struct
{
    int    vertexIndices[3];                           //
} h3d_triangle;

typedef struct
{
	int             frame;
	float           position[3];						//local reference
	float           rotation[3];						//local reference
    glm::mat4       transform;
}h3d_keyframe;

typedef struct
{
    char*           name;                           	//
    int 			parentIndex; 						// -1 if no parent
    float           position[3];						//world reference
	float           rotation[3];                        //eulerXYZ	//world reference

    int             numKeyframes;                       //
    h3d_keyframe*  	keyframes;

	glm::mat4 		bindPose;							//Filled when loading
	glm::mat4		invBindPose;						//Filled when loading

} h3d_joint;


typedef struct
{
    char*           name;
	int 			jointsCount;
	h3d_joint*		joints;

}h3d_armature;

typedef struct
{
	char* 			name;
	int 			numVertices;
	h3d_vertex*		vertices;
}h3d_shape_key;

typedef struct
{
    char*           name;
    int             numTriangles;
    h3d_triangle*  	triangles;
	int 			numVertices;
	h3d_vertex* 	vertices;
    int             materialIndex;                      // -1 = no material
	char 			isAnimated; 						// 0 if no armature
	char* 			armatureName;						// if isAnimated is 1
	int 			armatureIndex;

	int 			shapeKeyCount;
	h3d_shape_key*	shapeKeys;
} h3d_group;

typedef struct
{
    char*           name;
    float           ambient[4];                             //
    float           diffuse[4];                         //
    float           specular[4];                        //
    float           emissive[4];                        //
    float           shininess;                          // 0.0f - 128.0f
    float           transparency;                       // 0.0f - 1.0f
    char*           textureImage;
	GLint 			textureId;
} h3d_material;


//#include <poppack.h>
#ifndef RC_INVOKED
#pragma pack(pop)
#endif

#ifndef GLES
typedef GLuint indexInt;
#else
typedef GLushort indexInt;
#endif

typedef struct{
	size_t positionSize;
	size_t textureCoordSize;
	size_t normalsSize;
    size_t jointsSize;
	size_t weightsSize;
    size_t* shapeKeysIndices; //Size is same as position/normals size
	size_t totalSize;
}h3d_vboDescription;

class H3DFile
{
private:
	h3d_group* _groups;
	int _groupCount;
	h3d_material* _materials;
	int _materialCount;
	h3d_armature* _armatures;
	int _armatureCount;

	GLuint* _vao;
	GLuint* _vbo;
	GLuint* _eab;
	GLuint _shader;

	h3d_vboDescription* _vboDescriptions;

	bool _isAnimated;
	int _currentFrame;

public:
	H3DFile();
	virtual ~H3DFile();

	bool LoadFromFile(const char* lpszFileName);
	void Clear();

	void prepareModel(GLuint shader);
	void unloadModel();
	void drawGL2();
	void drawGLES2();

	void setCurrentFrame(int f);

private:
	void prepareGroup(h3d_group* group, unsigned int groupIndex, GLuint shader);
	void setMaterialGL3(h3d_material* material);
	void recursiveParentTransform(glm::mat4* transforms, bool* hasParentTransform, h3d_joint* joints, int jointIndex);
	glm::mat4 recursiveBindPose(h3d_joint* joints, int i);
    glm::mat4 getBindPose(h3d_joint* joint);
	void handleAnimation(h3d_group* group);
    glm::mat4 getBoneTransform(h3d_joint* joint);
};

#endif // _H3DFILE_H_
