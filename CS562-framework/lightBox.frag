/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

layout (location = 0) out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D gSpecular;

uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform float lightRadius;
uniform vec3 viewPos;
vec3 outLightColor;

void main()
{           

    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gDiffuse, TexCoords).rgb;
    float Specular = texture(gSpecular, TexCoords).a;

    vec3 ambient  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);

    float distance = length(lightPosition - FragPos);

        if(distance < lightRadius)
        {
        // diffuse
        vec3 lightDir = normalize(lightPosition - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = lightColor * spec * Specular;
        // attenuation
        float distance = length(lightPosition - FragPos);
        if(distance < lightRadius)
        {
            float attenuation = 1.0 / (distance * distance) - 1.0 / (lightRadius * lightRadius);
            diffuse *= attenuation;
            specular *= attenuation;
            ambient += diffuse + specular;   
            outLightColor = ambient;     
        }    
        }
        else
        {
            outLightColor = vec3(0.0,0.0,0.0);
        }
    FragColor = vec4(1.0,1.0,1.0, 1.0);
}