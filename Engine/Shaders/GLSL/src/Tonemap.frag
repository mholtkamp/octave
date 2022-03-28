#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (set = 1, binding = 0) uniform sampler2D samplerLitColor;

layout (location = 0) in vec2 inTexcoord;

layout (location = 0) out vec4 outFinalColor;

void main()
{
    vec3 color = texture(samplerLitColor, inTexcoord).rgb;
    
    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // gamma correct
    //color = pow(color, vec3(1.0/2.2)); 
    
    outFinalColor = vec4(color, 1.0);
    
    
    //outFinalColor = texture(samplerLitColor, inTexcoord);
}