
#ifndef _DAEMESH_H_
#define _DAEMESH_H_

#include <vector>

#ifdef GLES
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#ifndef GLES
typedef GLuint indexInt;
#else
typedef GLushort indexInt;
#endif


typedef struct{
    int vertexIndexes[3];
    int normalIndexes[3];
    int tcoordIndexes[3];
}dae_triangle;

typedef struct{
    float position[3];
}dae_vertex;

typedef struct{
    float vector[3];
}dae_normal;

typedef struct{
    float st[2];
}dae_texture_coordinates;

typedef struct{
    char name[32];
    char *geometryId;
    dae_vertex* vertices;
    float* verticesPosition;
    int verticesCount;
    int verticesPositionCount;
    dae_normal* normals;
    int normalsCount;
    dae_texture_coordinates* tcoords;
    int tcoordCount;
    dae_triangle* triangles;
    int trianglesCount;

    int materialIndex;
}dae_geometry;

typedef struct{
    char* name;
    char* materialId;
    char* effectName;
    char* textureName;
    char* textureFile;
    GLint textureId;

    float emission[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
}dae_material;

typedef struct{
    std::vector<dae_geometry*> groups;
    std::vector<dae_material*> materials;
}dae_mesh;

typedef struct{
    size_t positionSize;
    size_t textureCoordSize;
    size_t normalsSize;
    size_t jointsSize;
    size_t totalSize;
}dae_vboDescription;

class DAEMesh{
private:
    dae_mesh* _mesh;

    GLuint* _vao;
    GLuint* _vbo;
    GLuint* _eab;
    GLuint _shader;

    dae_vboDescription* _vboDescriptions;

public:
    DAEMesh();
    ~DAEMesh();

    bool LoadFromFile(const char* lpszFileName);
    void Clear();

    void prepareModel(GLuint shader);
    void unloadModel();
    void drawGL2();
    void drawGLES2();

private:
    void prepareGroup(dae_geometry* group, unsigned int groupIndex, GLuint shader);
    void setMaterialGL3(dae_material* material);
    void transformGroup(dae_geometry* group, glm::mat4 mat);

};

#endif
