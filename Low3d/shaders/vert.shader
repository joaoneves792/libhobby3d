#version 450

#define MAX_LIGHTS 10

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texture_coord;
layout(location = 2) in vec3 normal;

out vec3 position_worldspace;
out vec2 texture_coord_from_vshader;
out vec3 normal_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_cameraspace[MAX_LIGHTS];

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 lightPosition_worldspace[MAX_LIGHTS];
uniform int lightsEnabled[MAX_LIGHTS];

void main() {
	gl_Position = Projection * View * Model * position;
	texture_coord_from_vshader = texture_coord;

	position_worldspace = (Model * position).xyz;

	vec3 vertexPosition_cameraspace = (View * Model * position).xyz;
	eyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

	for(int i=0; i<MAX_LIGHTS; i++){
		if(lightsEnabled[i] == 0)
			continue;
		vec3 lightPosition_cameraspace = (View * vec4(lightPosition_worldspace[i].xyz, 1)).xyz;
		lightDirection_cameraspace[i] = lightPosition_cameraspace + eyeDirection_cameraspace;
	}

	normal_cameraspace = (View * Model * vec4(normal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.

}
