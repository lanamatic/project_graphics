#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform int SCR_WIDTH;
uniform int SCR_HEIGHT;

uniform sampler2DMS scene;
uniform sampler2DMS bloomBlur;

uniform bool hdr;
uniform bool bloom;
uniform bool gammaEnabled;
uniform float exposure;
uniform bool grayscale;


void main(){
    const float gamma = 2.2;
//     vec3 hdrColor = texture(scene, TexCoords).rgb;
//     vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    ivec2 viewPortDim = ivec2(SCR_WIDTH, SCR_HEIGHT);
    ivec2 coord = ivec2(viewPortDim * TexCoords);

    //multisampling: 4 sample points
    vec3 sample0 = texelFetch(scene, coord, 0).rgb;
    vec3 sample1 = texelFetch(scene, coord, 1).rgb;
    vec3 sample2 = texelFetch(scene, coord, 2).rgb;
    vec3 sample3 = texelFetch(scene, coord, 3).rgb;

    vec3 hdrColor = 0.25 * (sample0 + sample1 + sample2 + sample3);

    sample0 = texelFetch(bloomBlur, coord, 0).rgb;
    sample1 = texelFetch(bloomBlur, coord, 1).rgb;
    sample2 = texelFetch(bloomBlur, coord, 2).rgb;
    sample3 = texelFetch(bloomBlur, coord, 3).rgb;

    vec3 bloomColor = 0.25 * (sample0 + sample1 + sample2 + sample3);

    if(bloom)
        hdrColor += bloomColor; // additive blending

    vec3 result;
    if(hdr)
       //tone mapping
       result = vec3(1.0) - exp(-hdrColor * exposure);
    else
       result = hdrColor;

    // gamma correction
    if(gammaEnabled)
        result = pow(result, vec3(1.0 / gamma));
    if(grayscale){
        float gray = 0.2126 * result.r + 0.7152 * result.g + 0.0722 * result.b;
        result = vec3(gray);
    }
    FragColor = vec4(result, 1.0);
}