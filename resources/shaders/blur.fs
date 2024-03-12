#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2DMS image;

uniform int SCR_WIDTH;
uniform int SCR_HEIGHT;

uniform bool horizontal;
uniform float weight[6] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162, 0.0004115591);

void main(){
//      vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel

     ivec2 viewPortDim = ivec2(SCR_WIDTH, SCR_HEIGHT);
     ivec2 coord = ivec2(viewPortDim * TexCoords);

     //multisample: 4 sample points
     vec3 sample0 = texelFetch(image, coord, 0).rgb;
     vec3 sample1 = texelFetch(image, coord, 1).rgb;
     vec3 sample2 = texelFetch(image, coord, 2).rgb;
     vec3 sample3 = texelFetch(image, coord, 3).rgb;

     //      vec3 result = texture(image, TexCoords).rgb * weight[0];
     vec3 result = 0.25 * (sample0 + sample1 + sample2 + sample3);
     result = result * weight[0];

     if(horizontal){
         for(int i = 1; i < 6; ++i){
//             result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
//             result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
           sample0 = texelFetch(image, coord + ivec2(i, 0.0), 0).rgb;
           sample1 = texelFetch(image, coord + ivec2(i, 0.0), 1).rgb;
           sample2 = texelFetch(image, coord + ivec2(i, 0.0), 2).rgb;
           sample3 = texelFetch(image, coord + ivec2(i, 0.0), 3).rgb;

           result += 0.25 * (sample0 + sample1 + sample2 + sample3) * weight[i];

           sample0 = texelFetch(image, coord - ivec2(i, 0.0), 0).rgb;
           sample1 = texelFetch(image, coord - ivec2(i, 0.0), 1).rgb;
           sample2 = texelFetch(image, coord - ivec2(i, 0.0), 2).rgb;
           sample3 = texelFetch(image, coord - ivec2(i, 0.0), 3).rgb;

           result += 0.25 * (sample0 + sample1 + sample2 + sample3) * weight[i];

         }
     }
     else{
         for(int i = 1; i < 6; ++i){
//              result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
//              result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            sample0 = texelFetch(image, coord + ivec2(0.0, i), 0).rgb;
            sample1 = texelFetch(image, coord + ivec2(0.0, i), 1).rgb;
            sample2 = texelFetch(image, coord + ivec2(0.0, i), 2).rgb;
            sample3 = texelFetch(image, coord + ivec2(0.0, i), 3).rgb;

            result += 0.25 * (sample0 + sample1 + sample2 + sample3) * weight[i];

            sample0 = texelFetch(image, coord - ivec2(0.0, i), 0).rgb;
            sample1 = texelFetch(image, coord - ivec2(0.0, i), 1).rgb;
            sample2 = texelFetch(image, coord - ivec2(0.0, i), 2).rgb;
            sample3 = texelFetch(image, coord - ivec2(0.0, i), 3).rgb;

            result += 0.25 * (sample0 + sample1 + sample2 + sample3) * weight[i];
         }
     }
     FragColor = vec4(result, 1.0);
}