#version 450

#define MAX_LIGHTS 10

in vec3 position_worldspace;
in vec2 texture_coord_from_vshader;
in vec3 normal_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 lightDirection_cameraspace[MAX_LIGHTS];
in vec4 ShadowCoord;

out vec4 out_color;

uniform sampler2D texture_sampler;
uniform sampler2DShadow shadowMap;
uniform vec3 lightPosition_worldspace[MAX_LIGHTS];
uniform vec4 lightCone[MAX_LIGHTS]; //x,y,z -> direction; w -> cutoffanglecos (if < 0 then emmits in all directions)
uniform vec4 lightColor[MAX_LIGHTS]; // x,y,z -> color rgb; w -> intensity (if < 0 then does not decay) 
uniform int lightsEnabled[MAX_LIGHTS];

uniform int disableLighting;

uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emissive;

uniform float shininess;
uniform float transparency;

vec2 poissonDisk[16] = vec2[]( 
   vec2( -0.94201624, -0.39906216 ), 
   vec2( 0.94558609, -0.76890725 ), 
   vec2( -0.094184101, -0.92938870 ), 
   vec2( 0.34495938, 0.29387760 ), 
   vec2( -0.91588581, 0.45771432 ), 
   vec2( -0.81544232, -0.87912464 ), 
   vec2( -0.38277543, 0.27676845 ), 
   vec2( 0.97484398, 0.75648379 ), 
   vec2( 0.44323325, -0.97511554 ), 
   vec2( 0.53742981, -0.47373420 ), 
   vec2( -0.26496911, -0.41893023 ), 
   vec2( 0.79197514, 0.19090188 ), 
   vec2( -0.24188840, 0.99706507 ), 
   vec2( -0.81409955, 0.91437590 ), 
   vec2( 0.19984126, 0.78641367 ), 
   vec2( 0.14383161, -0.14100790 ) 
);

void main() {
	if(disableLighting == 1){
		out_color = texture(texture_sampler, texture_coord_from_vshader);
		out_color.a -= transparency;
		return;
	}

	//Material properties
	vec3 matDiffuse = (texture(texture_sampler, texture_coord_from_vshader).rgb * diffuse.xyz);
	vec3 ambientCoef = ambient.xyz / vec3(255, 255, 255);
	vec3 matAmbient = ambientCoef * matDiffuse;
	vec3 matSpecular = specular.xyz;
	float visibility = 1.0;
	
	out_color.rgb = vec3(0,0,0);//Very important! start with black

	vec3 n = normalize( normal_cameraspace );
	vec3 E = normalize(eyeDirection_cameraspace);

	vec3 light_color_sum = vec3(0,0,0);
	for(int i=0; i<MAX_LIGHTS; i++){
		if(lightsEnabled[i] == 1){
			vec3 lightDirection_worldspace = lightPosition_worldspace[i].xyz - position_worldspace; 
			float distance_to_light = length( lightDirection_worldspace );

			vec3 l = normalize( lightDirection_cameraspace[i] );

			float spotEffect = dot( normalize(lightCone[i].xyz), normalize(-lightDirection_worldspace) );
			if(spotEffect > lightCone[i].w || lightCone[i].w <= 0){

				vec3 R = reflect(-l, n);
				float cosAlpha = clamp( dot(E,R), 0,1);
				float cosTheta = clamp( dot(n,l), 0,1);

				float decay;
				if(lightColor[i].w <= 0)
					decay = 1;
				else
					decay = (distance_to_light*distance_to_light/2);

				light_color_sum += matDiffuse * lightColor[i].xyz * abs(lightColor[i].w) * cosTheta / decay +
					matSpecular * (shininess/16.0) * lightColor[i].xyz * abs(lightColor[i].w) * pow(cosAlpha, 5) / decay;
			}
		}
		
	}
	//Shadows:
	float bias = 0.008;
	for (int i=0; i<8; i++){
		visibility -= 0.1*(1.0-texture( shadowMap, vec3(ShadowCoord.xy + poissonDisk[i]/4000, (ShadowCoord.z-bias)/ ShadowCoord.w) ) );
	}

	
	out_color.rgb =matAmbient + emissive.xyz + visibility*light_color_sum;
	out_color.a = texture(texture_sampler, texture_coord_from_vshader).a - transparency;
}
