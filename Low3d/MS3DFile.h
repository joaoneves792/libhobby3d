/*
 *	LXSR Import ms3d
 * 	adapted from source at milkshape website by E 
 *	MS3DFile.h
 *
 *  Created on: Jun 3, 2012
 */


#ifndef _MS3DFILE_H_
#define _MS3DFILE_H_

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
    char    id[10];                                     // always "MS3D000000"
    int     version;                                    // 4
} ms3d_header_t;

typedef struct
{
    byte    flags;                                      // SELECTED | SELECTED2 | HIDDEN
    float   vertex[3];                                  //
    signed char    boneId;                                     // -1 = no bone
    byte    referenceCount;
} ms3d_vertex_t;

typedef struct
{
    word    flags;                                      // SELECTED | SELECTED2 | HIDDEN
    word    vertexIndices[3];                           //
    float   vertexNormals[3][3];                        //
    GLfloat   s[3];                                       //
    GLfloat   t[3];                                       //
    byte    smoothingGroup;                             // 1 - 32
    byte    groupIndex;                                 //
} ms3d_triangle_t;

typedef struct
{
	word edgeIndices[2];
} ms3d_edge_t;

typedef struct
{
    byte            flags;                              // SELECTED | HIDDEN
    char            name[32];                           //
    word            numtriangles;                       //
    word*	    triangleIndices;			// the groups group the triangles
    char            materialIndex;                      // -1 = no material
} ms3d_group_t;

typedef struct
{
    char            name[32];                           //
    float           ambient[4];                         //
    float           diffuse[4];                         //
    float           specular[4];                        //
    float           emissive[4];                        //
    float           shininess;                          // 0.0f - 128.0f
    float           transparency;                       // 0.0f - 1.0f
    char            mode;                               // 0, 1, 2 is unused now
    char            texture[128];                        // texture.bmp
    char            alphamap[128];                       // alpha.bmp

} ms3d_material_t;

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
}vboDescription;

class CMS3DFileI;

class CMS3DFile
{
private:
	CMS3DFileI *_i;
	GLuint* _vao;
	GLuint* _vbo;
	GLuint* _eab;
	GLuint _shader;

	vboDescription* _vboDescriptions;

	bool _overrideAmbient;
	bool _overrideDiffuse;
	bool _overrideSpecular;
	bool _overrideEmissive;
	
	float* _white;
	float* _black;

	bool _isAnimated;

public:
	CMS3DFile();
	virtual ~CMS3DFile();

	bool LoadFromFile(const char* lpszFileName);
	bool SaveToFile(const char* lpszFileName);
	void Clear();

	void createRectangle(float width, float height, GLuint texture);

	void optimize();

	void prepareModel(GLuint shader);
	void unloadModel();
	void drawGL3();
	void drawGLES2();

	int GetNumVertices();
	void GetVertexAt(int nIndex, ms3d_vertex_t **ppVertex);
	int GetNumTriangles();
	void GetTriangleAt(int nIndex, ms3d_triangle_t **ppTriangle);
	int GetNumEdges();
	void GetEdgeAt(int nIndex, ms3d_edge_t **ppEdge);
	int GetNumGroups();
	void GetGroupAt(int nIndex, ms3d_group_t **ppGroup);
	int FindGroupByName(const char* lpszName);
	int GetNumMaterials();
	void GetMaterialAt(int nIndex, ms3d_material_t **ppMaterial);
	int GetNumJoints();
	void GetJointAt(int nIndex, ms3d_joint_t **ppJoint);
	int FindJointByName(const char* lpszName);

	float GetAnimationFPS();
	float GetCurrentTime();
	int GetTotalFrames();

	void draw();
	void setMaterial(ms3d_material_t* material, int textureIndex);
	void setMaterial(int texture, ms3d_group_t* group);
	void setMaterialGL3(ms3d_material_t* material, int textureIndex);
	void setTexture(unsigned int textureIndex, int texture);
	void setGLTexture(int texture, int group);

	void setOverrideAmbient(bool overrideAmbient);
	void setOverrideDiffuse(bool overrideDiffuse);
	void setOverrideSpecular(bool overrideSpecular);
	void setOverrideEmissive(bool overrideEmissive);

	void setMaterialEmissive(char* matName, float red, float green, float blue);
	void setMaterialTransparency(char* matName, float alpha);

	void translateModel(float x, float y, float z);

	void enableAnimation(bool isAnimated);
	void setAnimationTime(float t);

	CMS3DFile(const CMS3DFile& rhs);
	CMS3DFile& operator=(const CMS3DFile& rhs);

private:
	void mergeGroups();
	void removeUnusedMaterials();
	void prepareGroup(ms3d_group_t* group, unsigned int groupIndex, GLuint shader);
	void drawGroup(ms3d_group_t* group);
	void recursiveParentTransform(glm::mat4* transforms, bool* hasParentTransform, int jointIndex);
	glm::mat4 recursiveBindPose(int i);
	void handleAnimation();
    glm::mat4 getBoneRotation(int i);
    glm::mat4 getBoneTranslation(int i);
};

#endif // _MS3DFILE_H_
