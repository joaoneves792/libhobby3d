#version 100

#define MAX_LIGHTS 1

varying lowp vec3 position_worldspace;
varying lowp vec2 texture_coord_from_vshader;
varying lowp vec3 normal_cameraspace;
varying lowp vec3 eyeDirection_cameraspace;
varying lowp vec3 lightDirection_cameraspace;

uniform sampler2D texture_sampler;


void main() {

    lowp vec4 lightColor = vec4(255, 255, 204, 90); // x,y,z -> color rgb; w -> intensity (if < 0 then does not decay)
    lowp vec4 lightCone = vec4(0, 0, 1, -0.5); //x,y,z -> direction; w -> cutoffanglecos (if < 0 then emmits in all directions)
    lowp vec3 lightPosition_worldspace = vec3(0, 100, 100);

	//Material properties
	lowp vec3 matDiffuse = texture2D(texture_sampler, texture_coord_from_vshader).rgb;
	lowp vec3 matAmbient = 0.7 * matDiffuse;
	//lowp vec3 matSpecular = specular.xyz;

	gl_FragColor.rgb = vec3(0,0,0);//Very important! start with black

	lowp vec3 n = normalize( normal_cameraspace );
	lowp vec3 E = normalize(eyeDirection_cameraspace);

	lowp vec3 light_color_sum = vec3(0,0,0);
	lowp vec3 lightDirection_worldspace = lightPosition_worldspace.xyz - position_worldspace;
	lowp float distance_to_light = length( lightDirection_worldspace );

	lowp vec3 l = normalize( lightDirection_cameraspace );

	lowp float spotEffect = dot( normalize(lightCone.xyz), normalize(-lightDirection_worldspace) );
	if(spotEffect > lightCone.w || lightCone.w <= 0.0){

		lowp vec3 R = reflect(-l, n);
		//lowp float cosAlpha = clamp( dot(E,R), 0.0,1.0);
        lowp float cosTheta = clamp( dot(n,l), 0.0,1.0);

		lowp float decay;
		if(lightColor.w <= 0.0)
			decay = 1.0;
		else
			decay = (distance_to_light*distance_to_light/2.0);

		light_color_sum += matDiffuse * lightColor.xyz * abs(lightColor.w) * cosTheta / decay; //+
			//matSpecular * (shininess/16.0) * lightColor[i].xyz * abs(lightColor[i].w) * pow(cosAlpha, 5.0) / decay;
	}

	gl_FragColor.rgb = light_color_sum;
	//gl_FragColor.a = texture2D(texture_sampler, texture_coord_from_vshader).a;

}
