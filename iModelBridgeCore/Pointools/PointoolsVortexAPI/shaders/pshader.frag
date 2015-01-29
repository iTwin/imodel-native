#ifdef PT_P
uniform 	sampler1D	Tp;
#endif

#ifdef PT_E	
uniform 	sampler2D	Tl;
varying 	vec2 		pL;
#endif

#ifdef PT_I
uniform 	sampler1D 	Ti;
#endif
	
varying 	vec4 		pC;
varying		vec4		Csl;

#ifdef PT_C
varying		vec4		pA;
uniform		vec4		pCP[6];
uniform		int			pCPe[6];
uniform		int			CSt;
#endif

#ifdef PT_L
varying		float		pLam;
varying		vec3		pN;
#endif

varying		float		fogFactor;

void main(void)
{	
	vec4 frag;	
	vec3 pt, pl;
	int ct = 0;
	
#ifdef PT_C
	for (int i = 0; i < 6; i++)
	{		
		if (pCPe[i] == 1)
		{
			pt.xyz = pA.xyz;
			pl.xyz = pCP[i].xyz;
			if ((dot(pl, pt) - pCP[i].w) > 0.0) 
			{
				if (CSt == 1) 
					discard;					
				else
					ct++;
				
			}
		}		
	}
	if ((CSt == 2) && (ct == 0))
		discard;
#endif

#ifdef PT_E
	vec4 l = texture2D ( Tl, pL );
	if (l.x == 0.0)	discard;

	if (pL.x > 0.5)
	{
		gl_FragColor.rgb = Csl.rgb;
		gl_FragColor.a = 1.0;
	}
	else
#endif
	{
		if (Csl.a > 0.0)
		{
			gl_FragColor = Csl;
		}
		else
		{
			vec4 p = vec4(1.0,1.0,1.0,1.0);
			
			
		#ifdef PT_P
			#ifdef PT_P_EDGE_CLAMP
				p = texture1D( Tp, clamp(gl_TexCoord[1].x, 0.0, 1.0 ) );	
			#else 
				#ifdef PT_P_EDGE_STOP
					if (gl_TexCoord[1].x >= 0.0 && gl_TexCoord[1].x <= 1.0 )		
						p = texture1D( Tp, gl_TexCoord[1].x );
				#else
					p = texture1D( Tp, mod(gl_TexCoord[1].x,1.0) );	
				#endif
			#endif
		#endif
		
		#ifdef PT_L
			#ifndef PT_EG
				vec4 spec = vec4(0.0, 0.0, 0.0, 0.0);
				
				vec4 diff = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
				vec4 amb = 	gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
				
				if (pLam > 0.0) 
				{
					float hv = max(dot(pN, gl_LightSource[0].halfVector.xyz),0.0);
					spec = gl_FrontMaterial.specular * gl_LightSource[0].specular 
								* pow(hv,gl_FrontMaterial.shininess);
				}
			#else
				vec4 diff = vec4(1.0,1.0,1.0,1.0);
				vec4 amb = vec4(0.0, 0.0, 0.0 ,0.0);
				vec4 spec = vec4(0.0, 0.0, 0.0, 0.0);
				
			#endif
				
			#ifdef PT_I
				vec4 i = texture1D( Ti, gl_TexCoord[0].x );
				frag = pC * i * p * pLam * diff + amb + spec;
			#else
				frag = pC * p * pLam * diff + amb + spec;	
			#endif		
		#else
			#ifdef PT_I
				vec4 i = texture1D( Ti, gl_TexCoord[0].x );
				frag = pC * i * p;
			#else
				frag = pC * p;	
			#endif	
		#endif
			
		#ifdef PT_F
			float fogFactor = 0.0;
			float z = gl_FragCoord.z / gl_FragCoord.w;
			const float LOG2 = 1.442695;	
			
			#ifdef PT_F_EXP2
				
				fogFactor = exp2( -gl_Fog.density * 
						gl_Fog.density * 
						z * z * LOG2 );
						
			#else
				#ifdef PT_F_EXP
					fogFactor = exp2( -gl_Fog.density * z * LOG2 );	
				#else //Linear
					fogFactor = 1.0 - clamp( (z - gl_Fog.start) / (gl_Fog.end - gl_Fog.start), 0.0, 1.0);	 
				#endif
			#endif
			
			fogFactor = clamp(fogFactor, 0.0, 1.0);
			gl_FragColor = mix(gl_Fog.color, frag, fogFactor );
			
		#else
			gl_FragColor = frag;
		#endif
		}
	}
}