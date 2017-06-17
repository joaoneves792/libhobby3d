#include "MS3DFile.h"
#include "MS3DFileI.h"

#include <GL/glew.h>
#include <iostream>
#include <vector>
#include <list>

void CMS3DFile::mergeGroups(){
	std::vector<ms3d_group_t>* groups = &_i->arrGroups;

	ms3d_group_t* curGroup; 
	ms3d_group_t* otherGroup;
	char curMaterial = -1;
	char otherMaterial = -1;
	for(unsigned int i = 0; i < groups->size(); i++){
		curGroup = &(*groups)[i];
		curMaterial = curGroup->materialIndex;
		for(unsigned int j=i+1; j<groups->size(); j++){
			otherGroup = &(*groups)[j];
			otherMaterial = otherGroup->materialIndex;
			if(curMaterial == otherMaterial){
				/*Merge*/
				int oldNumTriangles = curGroup->numtriangles;
				word* oldIndices = curGroup->triangleIndices;
				
				curGroup->numtriangles += otherGroup->numtriangles;
				curGroup->triangleIndices = new word[curGroup->numtriangles];
				
				std::copy(oldIndices, oldIndices+oldNumTriangles, curGroup->triangleIndices);
				std::copy(otherGroup->triangleIndices, (otherGroup->triangleIndices)+(otherGroup->numtriangles), curGroup->triangleIndices+oldNumTriangles);

				delete otherGroup->triangleIndices;
				groups->erase(groups->begin()+j);

				/*After merging there is one less group (this one) so we decrement j*/
				j--;
			}
		}
	}
}

void CMS3DFile::removeUnusedMaterials(){

	/*Not actually removing it, just erasing the texture associated with it so we dont load the image next time*/

	int numOfMaterials = _i->arrMaterials.size();
	bool used[numOfMaterials];
	for(int i=0; i<numOfMaterials; i++)
		used[i] = false;

	for(unsigned int i=0; i<_i->arrGroups.size(); i++)
		used[(int)_i->arrGroups[i].materialIndex] = true;

	for(int i=0; i<numOfMaterials; i++)
		if(!used[i]){
			_i->arrMaterials[i].texture[0] = '\0';
			std::cout << "Cleared out material: " << _i->arrMaterials[i].name << "\n";
		}
}
