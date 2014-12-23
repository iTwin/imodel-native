
#ifdef PT_E
attribute 	float	pE;
#endif

uniform		vec4	Cs;

#ifdef PT_P
uniform		vec4	Pl;
#endif

uniform 	mat4	Mr;
uniform 	mat4	Mp;

uniform		vec4	Qs;
uniform		vec4	Qo;

varying 	vec4 	pC; 
varying 	vec2 	pL;
varying		vec4	Csl;

uniform 	mat4	Mb;

varying 	vec4	pA;
varying		float	pLam;
varying		vec3	pN;
uniform 	mat4	Mei;

uniform 	float	Ep;
uniform 	float	FS;

void main(void)
{
	gl_Position = ftransform();
	pC 			= gl_Color;
	
#ifdef PT_I
	gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
#endif
	
#ifdef PT_P
	pA = (gl_Vertex * Qs + Qo) * Mr;
	
	gl_TexCoord[1].x = dot(pA, Pl);
#endif
	Csl = Cs;
	Csl.a = FS;

#ifdef PT_E	
	pL.x = pE;
	pL.x += 0.5;
	pL.y = 0.5;
	pL /= 256.0;
	
#endif	

#ifdef PT_C	
	pA = (gl_Vertex * Qs + Qo) * Mp;
#endif

#ifdef PT_L
	/* get the actual normal */ 
	vec4 Na = vec4(gl_Normal.x, gl_Normal.y, gl_Normal.z, 0.0);
	Na *= Mei;
	pN = Na.xyz;
			
	#ifdef PT_EG
		pLam = pow(length( Na.xy ), Ep);
	#else
		vec3 lgtDir = normalize( gl_LightSource[0].position.xyz );
		pLam = max(dot(Na.xyz, lgtDir),0.0);
	#endif
#endif

#ifdef PT_F
	const float LOG2 = 1.442695;
	vec3 v = vec3(gl_ModelViewMatrix * gl_Vertex);
	
	gl_FogFragCoord = length(v);
#endif
}

