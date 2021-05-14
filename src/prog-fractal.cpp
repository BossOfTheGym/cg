#include "prog-fractal.h"

#include "gl-header/gl-header.h"
#include "graphics-res/graphics-res-util.h"

#include <cassert>

namespace
{
	inline const char* VERT_SOURCE = R"vert(
	#version 450 core

	const vec2 verts[4] = 
	{
		vec2(-1.0, +1.0),
		vec2(-1.0, -1.0),
		vec2(+1.0, +1.0),
		vec2(+1.0, -1.0),
	};

	void main()
	{
		gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
	}
	)vert";

	inline const char* FRAG_SOURCE = R"frag(
	#version 450 core

	out vec4 color;

	uniform vec2 iResolution;
	uniform float iTime;

	float circle(vec2 uv, float size) {
		return smoothstep(size, size * 0.9, length(uv));
	}

	float cube(vec2 uv, float size) {
	    return smoothstep(size, size * 0.9, max(abs(uv.x), abs(uv.y)));
	}
	
	mat2 rot(float a) {
	    float c = cos(a);
	    float s = sin(a);
	    return mat2(c, s, -s, c);
	}
	
	vec3 coolTex(vec2 uv) {    
	    const float pi  = 3.14159265359;
	    const float pi2 = 2.0 * 3.14159265359;
	    const float p   = 2.0;
	    
		uv -= mod(uv, vec2(1.0 / 64.0)) - 1.0 / 128.0;
	    return vec3(
	        sin(uv.x * p*pi2) * sin(uv.y * p*pi2),
	        sin(uv.x * p*pi2 + p) * sin(uv.y * p*pi2),
	        sin(uv.x * p*pi2 + 2.*p) * sin(uv.y * p*pi2)
	    );
	}
	
	vec3 badTrip(vec2 uv) {
	    float t = 0.5 * iTime;
	    
		uv = mod(uv, 1.0) - 0.5;
	    for (float i = 0.0; i < 4.0; i++) {
	        uv = abs(uv) - 0.5;
	        uv = rot(t) * uv;
			uv = mod(uv, 1.0) - 0.5;
	        t *= 0.75;
	    }

	    float cu = cube(uv, 0.5);
	    float ci = circle(uv, 0.5);
	    float c = mix(cu, ci, 0.5 + 0.5 * sin(iTime));
	    
	    vec3 tex = coolTex(uv);
	    
	    return vec3(mix(vec3(0.0), tex, c));
	}
	
	void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
	    vec2 uv = (2.0 * fragCoord - iResolution.xy) / iResolution.y;
	    
	    vec3 color = badTrip(uv); 
	    
	    //color = pow(color, vec3(1.0 / 2.2));
	    
	    fragColor = vec4(color, 1.0);
	}

	void main() {
		mainImage(color, gl_FragCoord.xy);
	}
	)frag";
}

ProgFractal::ProgFractal()
{
	bool stat{};

	res::Shader vert;
	stat = res::try_create_shader_from_source(vert, GL_VERTEX_SHADER, VERT_SOURCE);
	assert(stat);

	res::Shader frag;
	stat = res::try_create_shader_from_source(frag, GL_FRAGMENT_SHADER, FRAG_SOURCE);
	assert(stat);

	stat = res::try_create_shader_program(m_prog, vert, frag);
	assert(stat);

	m_resolutionLoc = glGetUniformLocation(m_prog.id, "iResolution");
	assert(m_resolutionLoc != -1);
	m_timeLoc = glGetUniformLocation(m_prog.id, "iTime");
	assert(m_timeLoc != -1);
}

void ProgFractal::use()
{
	glUseProgram(m_prog.id);
}

void ProgFractal::setResolution(const prim::vec2& res)
{
	glProgramUniform2fv(m_prog.id, m_resolutionLoc, 1, prim::value_ptr(res));
}

void ProgFractal::setTime(f32 t)
{
	glProgramUniform1f(m_prog.id, m_timeLoc, t);
}
