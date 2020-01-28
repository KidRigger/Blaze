#version 450
#extension GL_ARB_separate_shader_objects : enable

#define MAX_POINT_LIGHTS 16
#define MAX_DIR_LIGHTS 1
#define MAX_CSM_SPLITS 4

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 projection;
	vec3 viewPos;
	mat4 dirLightPV[MAX_DIR_LIGHTS][MAX_CSM_SPLITS];
	vec4 lightDir[MAX_DIR_LIGHTS];
	vec4 csmSplits[MAX_CSM_SPLITS];
	vec4 lightPos[MAX_POINT_LIGHTS];
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords0;
layout(location = 3) in vec2 inTexCoords1;

layout(push_constant) uniform TRS {
	mat4 model;
} trs;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 normals;
layout(location = 2) out vec2 texCoords0;
layout(location = 3) out vec2 texCoords1;

layout(location = 4) out vec4 lightCoord[MAX_DIR_LIGHTS][MAX_CSM_SPLITS];

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
	gl_Position = ubo.projection * ubo.view * trs.model * vec4(inPosition, 1.0f);
	outPosition = (trs.model * vec4(inPosition, 1.0f)).xyz;
	normals = mat3(transpose(inverse(trs.model))) * inNormal;
	texCoords0 = inTexCoords0;
	texCoords1 = inTexCoords1;

	for (int i = 0; i < MAX_DIR_LIGHTS; i++) {
        for (int j = 0; j < MAX_CSM_SPLITS; j++) {
            lightCoord[i][j] = biasMat * ubo.dirLightPV[i][j] * trs.model * vec4(inPosition, 1.0f);
            // Light coord always is flipped vertically due to Vulkan's -y = up
            lightCoord[i][j].y = 1.0f - lightCoord[i][j].y;
	    }
    }
}
