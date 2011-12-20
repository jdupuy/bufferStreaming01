#version 420 core

#ifdef _VERTEX_

layout(location=0)  in vec3 iPosition;
layout(location=1)  in vec3 iNormal;
layout(location=2)  in vec2 iTexCoord;

layout(location=0)  out vec3 oNormal;
layout(location=1)  out vec2 oTexCoord;

uniform mat4 uModelViewProjection;

void main()
{
	oNormal       = normalize(iNormal);
	oTexCoord     = iTexCoord;
	gl_Position   = uModelViewProjection * vec4(iPosition, 1.0);
}

#endif // _VERTEX_

#ifdef _FRAGMENT_

uniform sampler2D sSkin;

layout(location=0)  in vec3 iNormal;
layout(location=1)  in vec2 iTexCoord;

layout(location=0)  out vec4 oColor;

void main()
{
	vec3 N = normalize(iNormal);
	vec3 L = normalize(vec3(2.0,1.0,1.0));
	oColor = max(0.0, dot(N, L)) * texture(sSkin, iTexCoord);
//	oColor.rgb = abs(N);
}

#endif // _FRAGMENT_

