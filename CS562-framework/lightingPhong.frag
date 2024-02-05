/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

out vec4 FragColor;

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;
const int     floorId   = 11;
const int     bunnyId	= 12;

float pi = 3.14159;
float pi2 = 2*pi;

in vec3 normalVec, lightVec, eyeVec;
in vec2 TexCoords;

uniform int objectId;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

struct Light {
    vec3 Position;
    vec3 Color;
    
    float Linear;
    float Quadratic;
    float Radius;
};
const int NR_LIGHTS = 33;
uniform Light lights[NR_LIGHTS];
uniform vec3 viewPos;

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gDiffuse, TexCoords).rgb;
    float Specular = texture(gSpecular, TexCoords).a;

    // then calculate lighting as usual
    vec3 ambient  = Diffuse * 0.2; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    for(int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].Color;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lights[i].Color * spec * Specular;
        // attenuation
        float distance = length(lights[i].Position - FragPos);
        if(distance < lights[i].Radius)
        {
            float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
            diffuse *= attenuation;
            specular *= attenuation;
            ambient += diffuse + specular;        
        }
        
    }
    FragColor = vec4(ambient, 1.0);
}


/*
    vec3 ONE = vec3(1.0, 1.0, 1.0);
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
    vec3 V = normalize(eyeVec);
    vec3 H = normalize(L+V);
    float NL = max(dot(N,L),0.0);
    float NV = max(dot(N,V),0.0);
    float HN = max(dot(H,N),0.0);

    vec3 I = ONE;
    vec3 Ia = 0.2*ONE;
    vec3 Kd = diffuse; 
    
    // A checkerboard pattern to break up larte flat expanses.  Remove when using textures.
    //if (objectId==groundId || objectId==floorId || objectId==seaId) {
    //    ivec2 uv = ivec2(floor(100.0*texCoord));
    //    if ((uv[0]+uv[1])%2==0)
    //        Kd *= 0.9; }
    
   // Lighting is diffuse + ambient + specular
    vec3 fragColor = Ia*Kd;
        fragColor += I*Kd*NL;
        fragColor += I*specular*pow(HN,shininess); 
    FragColor.xyz = fragColor;
    */