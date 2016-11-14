#version 150 core

uniform mat4 model;
uniform vec3 cameraPosition;

#define MAX_LIGHTS 10
uniform int numLights;
uniform struct Light {
    vec3 position;
    vec3 color;
    float attenuation;
    float ambientCoeff;
} allLights[MAX_LIGHTS];

uniform struct {
    vec3 diffuseColor;
    vec3 specularColor;
    vec3 ambientColor;

    float specularIntensity;
    float specularHardness;

    float shininess;
} material;

in vec3 fragVert;
in vec3 fragNormal;
in vec4 fragColor;

out vec4 finalColor;

vec3 ApplyLight(Light light, vec3 color, vec3 normal, vec3 position, vec3 surfaceToCamera) {
    vec3 lightVector = normalize(light.position.xyz - position);
    // Point light
    float distanceToLight = length(light.position.xyz);// - position);
    float attenuation = 1.0 / (1.0 + light.attenuation * distanceToLight);

    // Ambient
    vec3 ambient = light.ambientCoeff * color.rgb * light.color;

    // Diffuse
    float diffuseCoefficient = max(0.0, - dot(normal, lightVector));
    vec3 diffuse = vec3(diffuseCoefficient) * color.rgb * light.color;

    // Specular
    float specularCoefficient = pow(max(0.0, dot(surfaceToCamera, reflect(- lightVector, normal))), material.shininess);
    vec3 specular = vec3(specularCoefficient * material.specularColor * light.color);

    // Linear color (color before gamma correction)
    return ambient + attenuation*(diffuse + specular);
}



void main() {
    vec3 normal = normalize(transpose(inverse(mat3(model))) * fragNormal);
    vec3 position = vec3(model * vec4(fragVert, 1));
    vec4 surfaceColor = fragColor; //texture(materialTex, fragTexCoord);
    vec3 surfaceToCamera = normalize(cameraPosition - position);

    // Combine color from all the lights
    vec3 linearColor = vec3(1.0);
    for(int i = 0 ; i < numLights ; ++i){
        linearColor += ApplyLight(allLights[i], surfaceColor.rgb, normal, position, surfaceToCamera);
    }

    // Final color (after gamma correction)
    vec3 gamma = vec3(1.0/2.2);
    finalColor = vec4(pow(linearColor, gamma), surfaceColor.a);
}