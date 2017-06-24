#version 100

#define MAX_LIGHTS 1

attribute lowp vec4 position;
attribute lowp vec2 texture_coord;
attribute lowp vec3 normal;

varying lowp vec3 position_worldspace;
varying lowp vec2 texture_coord_from_vshader;
varying lowp vec3 normal_cameraspace;
varying lowp vec3 eyeDirection_cameraspace;
varying lowp vec3 lightDirection_cameraspace;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 MVP;
uniform int lightsEnabled;

void main() {
	position_worldspace = (Model * position).xyz;

	gl_Position = MVP * position;
	texture_coord_from_vshader = texture_coord;

	vec3 vertexPosition_cameraspace = (View * Model * position).xyz;
	eyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

    lowp vec3 lightPosition_worldspace = vec3(0, 100, 100);
	vec3 lightPosition_cameraspace = (View * vec4(lightPosition_worldspace.xyz, 1)).xyz;
	lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace;

	normal_cameraspace = (View * Model * vec4(normal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not.
}
