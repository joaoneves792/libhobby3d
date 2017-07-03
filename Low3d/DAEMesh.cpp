#include <string.h>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <rapidxml.hpp>

#ifdef GLES
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif

#include "DAEMesh.h"

#include "Textures.h"

using namespace rapidxml;
using namespace std;

DAEMesh::DAEMesh() {
    _vao = NULL;
    _vbo = NULL;
    _eab = NULL;
}

DAEMesh::~DAEMesh() {
    Clear();
    delete _mesh;
}

void DAEMesh::Clear() {
    //TODO actually free all the memory properly
    _mesh->groups.clear();
    _mesh->materials.clear();
}

void DAEMesh::unloadModel() {
    if(NULL != _vao && NULL != _eab && NULL != _vbo) {
        GLsizei numOfGroups = _mesh->groups.size();
        glDeleteBuffers(numOfGroups, _eab);
        glDeleteBuffers(numOfGroups, _vbo);
#ifndef GLES
        glDeleteVertexArrays(numOfGroups, _vao);
#endif
    }
    for(unsigned int i=0; i< _mesh->materials.size(); i++){
        glDeleteTextures(1, (GLuint *)&_mesh->materials[i]->textureId);
    }

    Clear();
}

void DAEMesh::prepareModel(GLuint shader) {
        _shader = shader;

        //We have one vao and one vbo per group (is this the best approach?)
        _vbo = new GLuint [_mesh->groups.size()];
        _eab = new GLuint [_mesh->groups.size()];
#ifndef GLES
        //VAOs are not supported in standard GLES2.0
        _vao = new GLuint [_mesh->groups.size()];
        glGenVertexArrays(_mesh->groups.size(), _vao);
#endif

        _vboDescriptions = new dae_vboDescription[_mesh->groups.size()];

    for(unsigned int i=0; i < _mesh->groups.size(); i++){
            prepareGroup(_mesh->groups[i], i, shader);
        }
}


//Pretty dumb implementation we should filter out duplicates and use the indexes properly
void DAEMesh::prepareGroup(dae_geometry *group, unsigned int groupIndex, GLuint shader) {
#ifndef GLES
    glBindVertexArray(_vao[groupIndex]);
#endif

    GLfloat *vertices  = new GLfloat[group->trianglesCount*3*4];
    GLfloat *normals   = new GLfloat[group->trianglesCount*3*3];
    GLfloat *texCoords = new GLfloat[group->trianglesCount*3*2];
    GLfloat *joints    = new GLfloat[0];

    indexInt *indices  = new indexInt[group->trianglesCount*3];

    int vertex_index = 0;
    int normal_index = 0;
    int tex_coord_index = 0;
    int indexes_index = 0;
    for(int i=0;i<group->trianglesCount; i++){
        for(int j=0;j<3;j++) {
            //indices[indexes_index++] = (indexInt)group->triangles[i].vertexIndexes[j];
            indices[indexes_index] = indexes_index;
            indexes_index++;

            int vi = group->triangles[i].vertexIndexes[j];
            int ni = group->triangles[i].normalIndexes[j];
            int ti = group->triangles[i].tcoordIndexes[j];

            vertices[vertex_index++] = group->vertices[vi].position[0];
            vertices[vertex_index++] = group->vertices[vi].position[1];
            vertices[vertex_index++] = group->vertices[vi].position[2];
            vertices[vertex_index++] = 1.0;

            normals[normal_index++] = group->normals[ni].vector[0];
            normals[normal_index++] = group->normals[ni].vector[1];
            normals[normal_index++] = group->normals[ni].vector[2];

            texCoords[tex_coord_index++] = group->tcoords[ti].st[0];
            texCoords[tex_coord_index++] = group->tcoords[ti].st[1];

        }
    }


    glGenBuffers(1, &_vbo[groupIndex]);

    //_vboDescriptions[groupIndex].positionSize     = sizeof(GLfloat)*group->verticesPositionCount;
    _vboDescriptions[groupIndex].positionSize     = sizeof(GLfloat)*group->trianglesCount*3*4;
    _vboDescriptions[groupIndex].normalsSize      = sizeof(GLfloat)*group->trianglesCount*3*3;
    _vboDescriptions[groupIndex].textureCoordSize = sizeof(GLfloat)*group->trianglesCount*3*2;
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexInt)*indexes_index , indices, GL_STATIC_DRAW);


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
    delete[] indices;
    delete[] normals;
    delete[] texCoords;
    delete[] joints;

}


void DAEMesh::drawGL2() {
#ifndef GLES
    //handleAnimation();
    for(unsigned int i=0; i < _mesh->groups.size(); i++){
        int materialIndex = (int)_mesh->groups[i]->materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(_mesh->materials[materialIndex]);
        glBindVertexArray(_vao[i]);
        glDrawElements(GL_TRIANGLES, _mesh->groups[i]->trianglesCount*3, GL_UNSIGNED_INT, 0);
    }
#endif
}

void DAEMesh::drawGLES2() {
    //handleAnimation();
    for(unsigned int i=0; i < _mesh->groups.size(); i++){
        int materialIndex = (int)_mesh->groups[i]->materialIndex;
        if( materialIndex >= 0 )
            setMaterialGL3(_mesh->materials[materialIndex]);

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

        glDrawElements(GL_TRIANGLES, _mesh->groups[i]->trianglesCount*3, GL_UNSIGNED_SHORT, 0);
    }
}


void DAEMesh::setMaterialGL3(dae_material* material){
    GLint ambient = glGetUniformLocation(_shader, "ambient");
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

    if( material->textureId > 0){
        glBindTexture( GL_TEXTURE_2D, material->textureId);
    }
}

int str2int(char* s){
	std::stringstream str(s);
	int x;
	str >> x;
	return x;
}

void char2floatArray(char* chars, float* array, int count){
    int i = 0;
    stringstream ssin(chars);
    while (ssin.good() && i < count){
        float fl;
        ssin >> fl;
        array[i] = fl;
        ++i;
    }
}

bool DAEMesh::LoadFromFile(const char *lpszFileName) {
	xml_document<> doc;
	xml_node<>* root_node;
    // Read the xml file into a vector
	ifstream theFile (lpszFileName);
	vector<char> buffer((istreambuf_iterator<char>(theFile)), istreambuf_iterator<char>());
	buffer.push_back('\0');
	// Parse the buffer using the xml file parsing library into doc 
	doc.parse<0>(&buffer[0]);
	// Find our root node

    root_node = doc.first_node("COLLADA");

    _mesh = new dae_mesh;



    /*------------Materials----------------------*/
    xml_node<> * materials_library = root_node->first_node("library_materials");
    for(xml_node<> * daeMat = materials_library->first_node("material"); daeMat; daeMat = daeMat->next_sibling("material")) {
            dae_material* material = new dae_material;
            material->name = daeMat->first_attribute("name")->value();
            material->materialId = daeMat->first_attribute("id")->value();
            material->effectName = daeMat->first_node("instance_effect")->first_attribute("url")->value();
            _mesh->materials.push_back(material);
    }
    xml_node<> * effects_library = root_node->first_node("library_effects");
    for(int i=0; i<_mesh->materials.size(); i++){
        dae_material* mat = _mesh->materials[i];
        for(xml_node<> * effect = effects_library->first_node("effect"); effect; effect = effect->next_sibling("effect")) {
            if(!strcmp((mat->effectName+1), effect->first_attribute("id")->value())){
                char* textureParam = NULL;
                xml_node<>* profile = effect->first_node("profile_COMMON");
                xml_node<>* technique = profile->first_node("technique");
                if(!strcmp(technique->first_attribute("sid")->value(), "common")){
                    xml_node<>* phong = technique->first_node("phong");
                    if(!phong){
                        phong = technique->first_node("lambert");
                    }
                    if(!phong){
                        continue;
                    }
                    xml_node<>* emmisive = phong->first_node("emission");
                    if(emmisive){
                        char2floatArray(emmisive->first_node("color")->value(), mat->emission, 4);
                    }
                    xml_node<>* ambient = phong->first_node("ambient");
                    if(ambient){
                        char2floatArray(ambient->first_node("color")->value(), mat->ambient, 4);
                    }
                    xml_node<>* diffuse = phong->first_node("diffuse");
                    if(diffuse){
                        if(diffuse->first_node("color")) {
                            char2floatArray(diffuse->first_node("color")->value(), mat->diffuse, 4);
                        }else if(diffuse->first_node("texture")) {
                            xml_node<> *diffuse_texture = phong->first_node("diffuse")->first_node(
                                    "texture");
                            textureParam = diffuse_texture->first_attribute("texture")->value();
                        }
                    }
                    xml_node<>* specular = phong->first_node("specular");
                    if(specular){
                        char2floatArray(specular->first_node("color")->value(), mat->specular, 4);
                    }
                    xml_node<>* shininess = phong->first_node("shininess");
                    if(shininess){
                        mat->shininess = atof(shininess->first_node("float")->value());
                    }

                }
                mat->textureFile = NULL;
                if(NULL != textureParam) {
                    for (xml_node<> *newparam = profile->first_node(
                            "newparam"); newparam; newparam = newparam->next_sibling("newparam")) {
                        if (!strcmp(newparam->first_attribute("sid")->value(), textureParam)) {
                            char *surfaceID = newparam->first_node("sampler2D")->first_node(
                                    "source")->value();
                            for (xml_node<> *innernewparam = profile->first_node(
                                    "newparam"); innernewparam; innernewparam = innernewparam->next_sibling(
                                    "newparam")) {
                                if (!strcmp(innernewparam->first_attribute("sid")->value(),
                                            surfaceID)) {
                                    char *imageID = innernewparam->first_node("surface")->first_node(
                                            "init_from")->value();
                                    for (xml_node<> *image = root_node->first_node(
                                            "library_images")->first_node("image"); image;
                                         image = image->next_sibling("image")) {
                                        if (!strcmp(imageID,
                                                    image->first_attribute("id")->value())) {
                                            mat->textureFile = image->first_node(
                                                    "init_from")->value();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if(NULL != mat->textureFile) {
            std::string folderPath(lpszFileName);
            std::string texturePath("./");
            folderPath = folderPath.substr(0, folderPath.find_last_of("/")+1);
            texturePath.assign(folderPath);
            texturePath.append(mat->textureFile);
            mat->textureId = LoadGLTexture(texturePath.c_str());
        }else{
            mat->textureId = -1;
        }
    }


    xml_node<> * geometries;
	geometries = root_node->first_node("library_geometries");
	for (xml_node<> * geometry = geometries->first_node("geometry"); geometry; geometry = geometry->next_sibling()) {
		int vertex_offset = -1;
		int normal_offset = -1;
		int tcoord_offset = -1;
        int max_offset = -1;

		char* vertex_source = NULL;
		char* normal_source = NULL;
		char* tcoord_source = NULL;

		dae_geometry* g = new dae_geometry;
		char* name = geometry->first_attribute("name")->value();
        g->verticesCount = 0;
        g->normalsCount = 0;
        g->tcoordCount = 0;

		strncpy(g->name, name, 32);

		xml_node<>* mesh = geometry->first_node("mesh");

		/*---------POLYLIST-------------------------------*/
		char* materialName = NULL;
		xml_node<> * polylist = mesh->first_node("polylist");
        if(polylist->first_attribute("material")){
			materialName = polylist->first_attribute("material")->value();
		}

        int inputCount = 0;
        for(xml_node<> * input = polylist->first_node("input"); input; input = input->next_sibling("input")) {
			char* semantic = input->first_attribute("semantic")->value();
            inputCount++;
			if(!strncmp("VERTEX", semantic, 6)){
				vertex_offset = str2int(input->first_attribute("offset")->value());
				vertex_source = input->first_attribute("source")->value();
                if(vertex_offset > max_offset)
                    max_offset = vertex_offset;
			}else if(!strncmp("NORMAL", semantic, 6)){
				normal_offset = str2int(input->first_attribute("offset")->value());
				normal_source = input->first_attribute("source")->value();
                if(normal_offset > max_offset)
                    max_offset = normal_offset;
			}else if(!strncmp("TEXCOORD", semantic, 8)){
				tcoord_offset = str2int(input->first_attribute("offset")->value());
				tcoord_source = input->first_attribute("source")->value();
                if(tcoord_offset > max_offset)
                    max_offset = tcoord_offset;
			}else{
                int other_offset = str2int(input->first_attribute("offset")->value());
                if(other_offset > max_offset)
                    max_offset = other_offset;
            }
		}
        inputCount = max_offset+1;
		g->trianglesCount= str2int(polylist->first_attribute("count")->value());

		char* str_indexes = polylist->first_node("p")->value();
		int p_count = g->trianglesCount*3*inputCount;

		int p[p_count];

		//Fill the indexes array
		int i = 0;
		stringstream ssin(str_indexes);
		while (ssin.good() && i < p_count){
			int ind;
			ssin >> ind;
			p[i] = ind;
			++i;
		}

		/*Fill the triangles data*/
		g->triangles = new dae_triangle[g->trianglesCount];
        //g->triangles = (dae_triangle*)malloc(sizeof(dae_triangle)*g->trianglesCount);
		i = 0;
		for(int triangle_i=0;triangle_i<g->trianglesCount;triangle_i++){
			for(int j=0; j<3; j++) {
				g->triangles[triangle_i].vertexIndexes[j] = p[i+vertex_offset];
				g->triangles[triangle_i].normalIndexes[j] = p[i+normal_offset];
				if(tcoord_offset >= 0) {
					g->triangles[triangle_i].tcoordIndexes[j] = p[i+tcoord_offset];
				}
                i = i+inputCount;
			}
		}

		/*-----------------Get the vertices source-------------------*/
		for(xml_node<> * vert = mesh->first_node("vertices"); vert; vert = vert->next_sibling("vertices")) {
			if(!strcmp((vertex_source+1), vert->first_attribute("id")->value())){
				vertex_source = vert->first_node("input")->first_attribute("source")->value();
			}
		}


		/*------------------SOURCES-----------------------*/
	    for(xml_node<> * source = mesh->first_node("source"); source; source = source->next_sibling("source")) {
			if (!strcmp(source->first_attribute("id")->value(), (vertex_source + 1))) {
				xml_node<> *float_array = source->first_node("float_array");
				int f_count = str2int(float_array->first_attribute("count")->value());
				char *floats = float_array->value();
				float f[f_count];
				i = 0;
				stringstream ssin(floats);
				while (ssin.good() && i < f_count) {
					float fl;
					ssin >> fl;
					f[i] = fl;
					++i;
				}

				g->verticesCount = f_count / 3;
				g->vertices = new dae_vertex[g->verticesCount];
                //g->verticesPosition = new float[g->verticesCount*4];
				int f_index = 0;
                int position_index = 0;
				for (int i = 0; i < g->verticesCount; i++) {
                    /*g->verticesPosition[position_index++] = f[f_index++];
                    g->verticesPosition[position_index++] = f[f_index++];
                    g->verticesPosition[position_index++] = f[f_index++];
                    g->verticesPosition[position_index++] = 1.0;*/

					g->vertices[i].position[0] = f[f_index];
					g->vertices[i].position[1] = f[f_index + 1];
					g->vertices[i].position[2] = f[f_index + 2];
					f_index += 3;
				}
                //g->verticesPositionCount = position_index;

			} else if (!strcmp(source->first_attribute("id")->value(), (normal_source + 1))) {
				xml_node<> *float_array = source->first_node("float_array");
				int f_count = str2int(float_array->first_attribute("count")->value());
				char *floats = float_array->value();
				float* f = new float[f_count];
				i = 0;
				stringstream ssin(floats);
				while (ssin.good() && i < f_count) {
					float fl;
					ssin >> fl;
					f[i] = fl;
					++i;
				}

				g->normalsCount = f_count / 3;
				g->normals = new dae_normal[g->normalsCount];
				int f_index = 0;
				for (int i = 0; i < g->normalsCount; i++) {
					g->normals[i].vector[0] = f[f_index++];
					g->normals[i].vector[1] = f[f_index++];
					g->normals[i].vector[2] = f[f_index++];
				}
                delete[] f;
			} else if (!strcmp(source->first_attribute("id")->value(), (tcoord_source + 1))) {
				xml_node<> *float_array = source->first_node("float_array");
				int f_count = str2int(float_array->first_attribute("count")->value());
				char *floats = float_array->value();
				float* f = new float[f_count];
				i = 0;
				stringstream ssin(floats);
				while (ssin.good() && i < f_count) {
					float fl;
					ssin >> fl;
					f[i] = fl;
					++i;
				}

				g->tcoordCount = f_count / 2;
				g->tcoords = new dae_texture_coordinates[g->tcoordCount];
				int f_index = 0;
				for (int i = 0; i < g->tcoordCount; i++) {
					g->tcoords[i].st[0] = f[f_index++];
					g->tcoords[i].st[1] = f[f_index++];
				}
                delete[] f;
			}

			if (NULL == materialName) {
				g->materialIndex = -1;
			} else {
				for (int i = 0; i < _mesh->materials.size(); i++) {
					if (!strcmp(_mesh->materials[i]->materialId, materialName)) {
						g->materialIndex = i;
					}
				}
			}

        }
        _mesh->groups.push_back(g);
	}
    return true;
}
