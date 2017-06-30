#include "DAEMesh.h"

int main(void){
	DAEMesh* dae = new DAEMesh();
	dae->LoadFromFile("/home/joao/workspace/prototypes/gltest/res/cube.dae");
	dae->prepareModel(0);
	return 0;
}
