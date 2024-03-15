#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

struct DirLight{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};

struct SpotLight{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

};

in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentViewPos;
in vec3 TangentFragPos;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;

#define N_POINT_LIGHTS 18

uniform DirLight directional;
uniform PointLight pointlight[N_POINT_LIGHTS];
uniform SpotLight spotlight;

uniform float heightScale;
uniform bool blinn;

vec2 ParallaxMapping(vec2 texCoords, vec3 viewDir){
    // number of depth layers
    const float minLayers = 8;
    const float maxLayers = 32;
    float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    // calculate the size of each layer
    float layerDepth = 1.0 / numLayers;
    // depth of current layer
    float currentLayerDepth = 0.0;
    // the amount to shift the texture coordinates per layer (from vector P)
    vec2 P = viewDir.xy / viewDir.z * heightScale;
    vec2 deltaTexCoords = P / numLayers;

    // get initial values
    vec2  currentTexCoords     = texCoords;
    float currentDepthMapValue = texture(depthMap, currentTexCoords).r;

    while(currentLayerDepth < currentDepthMapValue){
        // shift texture coordinates along direction of P
        currentTexCoords -= deltaTexCoords;
        // get depthmap value at current texture coordinates
        currentDepthMapValue = texture(depthMap, currentTexCoords).r;
        // get depth of next layer
        currentLayerDepth += layerDepth;
    }

    // get texture coordinates before collision (reverse operations)
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

    // get depth after and before collision for linear interpolation
    float afterDepth  = currentDepthMapValue - currentLayerDepth;
    float beforeDepth = texture(depthMap, prevTexCoords).r - currentLayerDepth + layerDepth;

    // interpolation of texture coordinates
    float weight = afterDepth / (afterDepth - beforeDepth);
    vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

    return finalTexCoords;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);


void main()
{
    // offset texture coordinates with Parallax Mapping
    vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    vec2 texCoords = TexCoords;

    texCoords = ParallaxMapping(TexCoords,  viewDir);
    if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
        discard;

    // obtain normal from normal map
    vec3 normal = texture(normalMap, texCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 result = CalcDirLight(directional, normal, viewDir);
    for(int i = 0; i < N_POINT_LIGHTS; i++){
        result += CalcPointLight(pointlight[i], normal, FragPos, viewDir);
    }
    result += CalcSpotLight(spotlight, normal, FragPos, viewDir);

    // Bloom
    float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        BrightColor = vec4(result, 1.0);
    else
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir){
    //ambient
    vec3 ambient = light.ambient * texture(diffuseMap, TexCoords).rgb;
    //diffuse
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(diffuseMap, TexCoords).rgb;
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0f;

    //Blinn-Phong
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir),0.0), 32.0);
    }else{
        spec = pow(max(dot(viewDir, reflectDir),0.0), 32.0);
    }

    vec3 specular = light.specular * spec ;

    return (ambient + diffuse + specular);
}


vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    //ambient
    vec3 ambient = light.ambient * texture(diffuseMap, TexCoords).rgb;
    //diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(diffuseMap, TexCoords).rgb;
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0f;

    //Blinn-Phong
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir),0.0), 32.0);
    }else{
        spec = pow(max(dot(viewDir, reflectDir),0.0), 32.0);
    }
    vec3 specular = light.specular * spec;


    //attenuation

    float d = length(light.position - fragPos);
    float att = 1.0/(d*d);


//     float att = 1.0/(light.constant + light.linear * d + light.quadratic * d * d);
    ambient *= att;
    diffuse *= att;
    specular *= att;

    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir){
    //ambient
    vec3 ambient = light.ambient * texture(diffuseMap, TexCoords).rgb;
    //diffuse
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(lightDir, normal),0.0);
    vec3 diffuse = light.diffuse * diff * texture(diffuseMap, TexCoords).rgb;
    //specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0f;

    //Blinn-Phong
    if(blinn){
        vec3 halfwayDir = normalize(lightDir + viewDir);
        spec = pow(max(dot(normal, halfwayDir),0.0), 32.0);
    }else{
        spec = pow(max(dot(viewDir, reflectDir),0.0), 32.0);
    }

    vec3 specular = light.specular * spec;


    //attenuation
    float d = length(light.position - fragPos);
    float att = 1.0/(d*d);
//     float att = 1.0/(light.constant + light.linear * d + light.quadratic * d * d);
    ambient *= att;
    diffuse *= att;
    specular *= att;
    //spotlight
    float theta = dot(-lightDir, normalize(light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.0);
    ambient *= intensity;
    diffuse *= intensity;
    specular *= intensity;

    return (ambient + diffuse + specular);
}