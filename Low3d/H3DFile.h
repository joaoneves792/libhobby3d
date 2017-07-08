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

#ifndef byte
typedef unsigned char byte;
#endif // byte

#ifndef word
typedef unsigned short word;
#endif // word

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
    //signed char    boneId;                                     // -1 = no bone
} h3d_vertex;

typedef struct
{
    int    vertexIndices[3];                           //
} h3d_triangle;

typedef struct
{
    char*           name;
    int             numTriangles;
    h3d_triangle*  	triangles;
	int 			numVertices;
	h3d_vertex* 	vertices;
    char            materialIndex;                      // -1 = no material
} h3d_group;

/* UNUSED (FOR NOW)
typedef struct
{
    char            name[32];                           //
    float           ambient[4];                         //
    float           diffuse[4];                         //
    float           specular[4];                        //
    float           emissive[4];                        //
    float           shininess;                          // 0.0f - 128.0f
    float           transparency;                       // 0.0f - 1.0f
    char            texture[128];                        // texture.bmp
} h3d_material_t;

typedef struct
{
    float           time;                               // time in seconds
    float           rotation[3];                        // x, y, z angles
} ms3d_keyframe_rot_t;

typedef struct
{
    float           time;                               // time in seconds
    float           position[3];                        // local position
} ms3d_keyframe_pos_t;

typedef struct
{
    byte            flags;                              // SELECTED | DIRTY
    char            name[32];                           //
    char            parentName[32];                     //
    int 			parentIndex; 						// -1 if no parent
	float           rotation[3];                        // local reference matrix
    float           position[3];

    word            numKeyFramesRot;                    //
    word            numKeyFramesTrans;                  //

	ms3d_keyframe_rot_t* keyFramesRot;      // local animation matrices
    ms3d_keyframe_pos_t* keyFramesTrans;  // local animation matrices

} ms3d_joint_t;
*/
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
	size_t totalSize;
}h3d_vboDescription;

class H3DFile
{
private:
	h3d_group* _groups;
	int _groupCount;
	//h3d_material* _materials;

	GLuint* _vao;
	GLuint* _vbo;
	GLuint* _eab;
	GLuint _shader;

	h3d_vboDescription* _vboDescriptions;

	bool _isAnimated;

public:
	H3DFile();
	virtual ~H3DFile();

	bool LoadFromFile(const char* lpszFileName);
	void Clear();

	void prepareModel(GLuint shader);
	void unloadModel();
	void drawGL2();
	void drawGLES2();

private:
	void prepareGroup(h3d_group* group, unsigned int groupIndex, GLuint shader);
	void drawGroup(h3d_group* group);
	void setMaterialGL3();
	/*void recursiveParentTransform(glm::mat4* transforms, bool* hasParentTransform, int jointIndex);
	glm::mat4 recursiveBindPose(int i);
	void handleAnimation();
    glm::mat4 getBoneRotation(int i);
    glm::mat4 getBoneTranslation(int i);*/
};

#endif // _H3DFILE_H_
