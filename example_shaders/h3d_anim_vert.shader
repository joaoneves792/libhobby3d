#version 450

#define MAX_LIGHTS 10

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texture_coord;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 jointIndex;
layout(location = 4) in vec3 jointWeight;

out vec3 position_worldspace;
out vec2 texture_coord_from_vshader;
out vec3 normal_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace[MAX_LIGHTS];
out vec4 ShadowCoord;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 MVP;
uniform mat4 depthBiasMVP;
uniform vec3 lightPosition_worldspace[MAX_LIGHTS];
uniform int lightsEnabled[MAX_LIGHTS];
uniform mat4 bones[69];

void main() {
    vec4 newPosition;
    if(jointIndex.x >= 0){
        mat4 resultingTransform = jointWeight.x * bones[int(jointIndex.x)];
        resultingTransform += jointWeight.y * bones[int(jointIndex.y)]; //If jointIndex.y == -1 then its weight will be 0 so no problem
        resultingTransform += jointWeight.z * bones[int(jointIndex.z)];

        newPosition = resultingTransform * position;
	    normal_cameraspace = (View * Model * resultingTransform * vec4(normal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
    }else{
        newPosition = position;
	    normal_cameraspace = (View * Model * vec4(normal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
    }
	position_worldspace = (Model * newPosition).xyz;
	ShadowCoord = depthBiasMVP * Model * newPosition;
	
	gl_Position = MVP * vec4(newPosition.xyz, 1.0f);
	texture_coord_from_vshader = texture_coord;
	
	vec3 vertexPosition_cameraspace = (View * Model * newPosition).xyz;
	eyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	for(int i=0; i<MAX_LIGHTS; i++){
		if(lightsEnabled[i] == 0)
			continue;
		vec3 lightPosition_cameraspace = (View * vec4(lightPosition_worldspace[i].xyz, 1)).xyz;
		lightDirection_cameraspace[i] = lightPosition_cameraspace + eyeDirection_cameraspace;
	}

}
