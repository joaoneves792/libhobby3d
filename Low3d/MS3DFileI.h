#pragma once

#include <vector>

class CMS3DFileI{
public:
	std::vector<ms3d_vertex_t> arrVertices;
	std::vector<ms3d_triangle_t> arrTriangles;
	std::vector<ms3d_edge_t> arrEdges;
	std::vector<ms3d_group_t> arrGroups;
	std::vector<ms3d_material_t> arrMaterials;
	std::vector<int> arrTextures; //Contains the ids of the OpenGL textures, Indexes match between this and arrMaterials
	float fAnimationFPS;
	float fCurrentTime;
	int iTotalFrames;
	std::vector<ms3d_joint_t> arrJoints;

public:
	CMS3DFileI()
	:	fAnimationFPS(24.0f),
		fCurrentTime(0.0f),
		iTotalFrames(0)
	{
	}
};
