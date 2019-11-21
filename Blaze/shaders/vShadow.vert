#version 450
#extension GL_EXT_multiview : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoords0;
layout(location = 3) in vec2 inTexCoords1;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outLightPos;

layout(set = 0, binding = 0) uniform ProjView {
	mat4 projection;
	mat4 view[6];
	vec3 lightPos;
} pv;

layout(push_constant) uniform PushConsts {
	layout (offset = 0) mat4 mvp;
} trs;

void main() {
	outPosition = (trs.mvp * vec4(inPosition, 1.0f)).xyz;
	gl_Position = pv.projection * pv.view[gl_ViewIndex] * trs.mvp * vec4(inPosition.xyz, 1.0f);
	outLightPos = pv.lightPos;
}
