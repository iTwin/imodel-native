/*
    glh - is a platform-indepenedent C++ OpenGL helper library 


    Copyright (c) 2000 Cass Everitt
	Copyright (c) 2000 NVIDIA Corporation
    All rights reserved.

    Redistribution and use in source and binary forms, with or
	without modification, are permitted provided that the following
	conditions are met:

     * Redistributions of source code must retain the above
	   copyright notice, this list of conditions and the following
	   disclaimer.

     * Redistributions in binary form must reproduce the above
	   copyright notice, this list of conditions and the following
	   disclaimer in the documentation and/or other materials
	   provided with the distribution.

     * The names of contributors to this software may not be used
	   to endorse or promote products derived from this software
	   without specific prior written permission. 

       THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	   REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	   CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	   POSSIBILITY OF SUCH DAMAGE. 


    Cass Everitt - cass@r3.nu
*/


#ifndef GLH_COMBINERS_H
#define GLH_COMBINERS_H

#ifdef MACOS
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <glh_extensions.h>

//
//  Combiner Helper Classes
//

namespace glh
{

# ifdef GL_ARB_texture_env_combine

	struct texture_env_combine
	{
		texture_env_combine()
		{
			rgb.combine = GL_MODULATE;
			a.combine = GL_MODULATE;
			rgb.src[0] = GL_TEXTURE;
			rgb.src[1] = GL_PREVIOUS_ARB;
			rgb.src[2] = GL_CONSTANT_ARB;
			a.src[0] = GL_TEXTURE;
			a.src[1] = GL_PREVIOUS_ARB;
			a.src[2] = GL_CONSTANT_ARB;	
			rgb.op[0] = GL_SRC_COLOR;
			rgb.op[1] = GL_SRC_COLOR;
			rgb.op[2] = GL_SRC_COLOR;
			a.op[0] = GL_SRC_ALPHA;
			a.op[1] = GL_SRC_ALPHA;
			a.op[2] = GL_SRC_ALPHA;

		}
		void apply(GLenum part)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
			if(part == GL_RGB)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, rgb.combine);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, rgb.src[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, rgb.src[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, rgb.src[2]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, rgb.op[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, rgb.op[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, rgb.op[2]);
			}
			else if (part == GL_ALPHA)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, a.combine);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, a.src[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, a.src[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, a.src[2]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, a.op[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, a.op[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, a.op[2]);
			}
		}
		struct state
		{
			GLenum combine;
			GLenum op[3];
			GLenum src[3];
			GLfloat scale;
		};
		state rgb, a;
	};

# endif


# ifdef GL_NV_texture_env_combine4

	struct texture_env_combine4
	{
		texture_env_combine4()
		{
			rgb.combine = GL_MODULATE;
			a.combine = GL_MODULATE;
			rgb.src[0] = GL_TEXTURE;
			rgb.src[1] = GL_PREVIOUS_EXT;
			rgb.src[2] = GL_CONSTANT_EXT;
			rgb.src[3] = GL_ZERO;
			a.src[0] = GL_TEXTURE;
			a.src[1] = GL_PREVIOUS_EXT;
			a.src[2] = GL_CONSTANT_EXT;	
			a.src[3] = GL_ZERO;
			rgb.op[0] = GL_SRC_COLOR;
			rgb.op[1] = GL_SRC_COLOR;
			rgb.op[2] = GL_SRC_COLOR;
			rgb.op[3] = GL_ONE_MINUS_SRC_COLOR;
			a.op[0] = GL_SRC_ALPHA;
			a.op[1] = GL_SRC_ALPHA;
			a.op[2] = GL_SRC_ALPHA;
			a.op[3] = GL_ONE_MINUS_SRC_ALPHA;

		}
		void apply(GLenum part)
		{
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
			if(part == GL_RGB)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, rgb.combine);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_EXT, rgb.src[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_EXT, rgb.src[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_EXT, rgb.src[2]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_EXT, rgb.op[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_EXT, rgb.op[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_EXT, rgb.op[2]);
			}
			else if (part == GL_ALPHA)
			{
				glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_EXT, a.combine);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_EXT, a.src[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_EXT, a.src[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_EXT, a.src[2]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_EXT, a.op[0]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_EXT, a.op[1]);
				glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_EXT, a.op[2]);
			}
		}
		struct state
		{
			GLenum combine;
			GLenum op[4];
			GLenum src[4];
			GLfloat scale;
		};
		state rgb, a;
	};

# endif


#ifdef GL_NV_register_combiners

	struct general_combiner
	{
		general_combiner()
		{
			rgb.a = variable(GL_PRIMARY_COLOR_NV);
			rgb.b = variable(    GL_TEXTURE0_ARB);
			rgb.c = variable(            GL_ZERO); 
			rgb.d = variable(            GL_ZERO); 
			a.a =  variable(GL_PRIMARY_COLOR_NV, GL_ALPHA);
			a.b =  variable(    GL_TEXTURE0_ARB, GL_ALPHA);
			a.c =  variable(            GL_ZERO, GL_ALPHA);
			a.d =  variable(            GL_ZERO, GL_ALPHA);
			rgb.ab_output = rgb.cd_output = GL_DISCARD_NV;
			rgb.sum_output = GL_SPARE0_NV;
			rgb.ab_dot_product = rgb.cd_dot_product = rgb.mux_sum = GL_FALSE;
			rgb.scale = rgb.bias = GL_NONE;
			a.ab_output = a.cd_output = GL_DISCARD_NV;
			a.sum_output = GL_SPARE0_NV;
			a.ab_dot_product = a.cd_dot_product = a.mux_sum = GL_FALSE;
			a.scale = a.bias = GL_NONE;
		}

		void apply(GLenum stage, GLenum portion)
		{
			if(portion == GL_RGB)
			{
				glCombinerInputNV(stage, GL_RGB, GL_VARIABLE_A_NV, rgb.a.input, rgb.a.mapping, rgb.a.component);
				glCombinerInputNV(stage, GL_RGB, GL_VARIABLE_B_NV, rgb.b.input, rgb.b.mapping, rgb.b.component);
				glCombinerInputNV(stage, GL_RGB, GL_VARIABLE_C_NV, rgb.c.input, rgb.c.mapping, rgb.c.component);
				glCombinerInputNV(stage, GL_RGB, GL_VARIABLE_D_NV, rgb.d.input, rgb.d.mapping, rgb.d.component);
				glCombinerOutputNV(stage, GL_RGB, rgb.ab_output, rgb.cd_output, rgb.sum_output, rgb.scale, rgb.bias,
								   rgb.ab_dot_product, rgb.cd_dot_product, rgb.mux_sum);
			}
			else if(portion == GL_ALPHA)
			{
				glCombinerInputNV(stage, GL_ALPHA, GL_VARIABLE_A_NV, a.a.input, a.a.mapping, a.a.component);
				glCombinerInputNV(stage, GL_ALPHA, GL_VARIABLE_B_NV, a.b.input, a.b.mapping, a.b.component);
				glCombinerInputNV(stage, GL_ALPHA, GL_VARIABLE_C_NV, a.c.input, a.c.mapping, a.c.component);
				glCombinerInputNV(stage, GL_ALPHA, GL_VARIABLE_D_NV, a.d.input, a.d.mapping, a.d.component);
				glCombinerOutputNV(stage, GL_ALPHA, a.ab_output, a.cd_output, a.sum_output, a.scale, a.bias,
								   GL_FALSE, GL_FALSE, a.mux_sum);
			}
		}

		void zero()

		{

			rgb.zero(variable(GL_ZERO, GL_RGB));

			a.zero(variable(GL_ZERO, GL_ALPHA));

		}

		struct variable
		{
			variable(GLenum i = GL_ZERO, GLenum c = GL_RGB, GLenum m = GL_UNSIGNED_IDENTITY_NV)
				: input(i), mapping(m), component(c) {}
			GLenum input, mapping, component;

		};

		
		variable var(GLenum input, GLenum component_usage = GL_RGB, GLenum mapping = GL_UNSIGNED_IDENTITY_NV)
		{ return variable(input, component_usage, mapping); }

		struct state
		{

			void zero(const variable & v)

			{

				a = b = c = d = v;

				ab_dot_product = cd_dot_product = mux_sum = GL_FALSE;

				scale = bias = GL_NONE;

				ab_output = cd_output = sum_output = GL_DISCARD_NV;

			}
			variable a, b, c, d;
			GLenum ab_output, cd_output, sum_output;
			GLenum scale, bias;
			bool ab_dot_product;
			bool cd_dot_product;
			bool mux_sum;
		};
		state rgb, a;
	};

	struct final_combiner
	{
		final_combiner()
		{
			a = variable(GL_FOG, GL_ALPHA);
			b = variable(GL_SPARE0_PLUS_SECONDARY_COLOR_NV);
			c = variable(GL_FOG);
			d = variable();
			e = variable();
			f = variable();
			g = variable(GL_SPARE0_NV, GL_ALPHA);
		}
		void apply()
		{
			glFinalCombinerInputNV(GL_VARIABLE_A_NV, a.input, a.mapping, a.component);
			glFinalCombinerInputNV(GL_VARIABLE_B_NV, b.input, b.mapping, b.component);
			glFinalCombinerInputNV(GL_VARIABLE_C_NV, c.input, c.mapping, c.component);
			glFinalCombinerInputNV(GL_VARIABLE_D_NV, d.input, d.mapping, d.component);
			glFinalCombinerInputNV(GL_VARIABLE_E_NV, e.input, e.mapping, e.component);
			glFinalCombinerInputNV(GL_VARIABLE_F_NV, f.input, f.mapping, f.component);
			glFinalCombinerInputNV(GL_VARIABLE_G_NV, g.input, g.mapping, g.component);
		}

		void zero()

		{

			a = b = c = d = e = variable();

			g = variable(GL_ZERO, GL_ALPHA);

		}
		
		struct variable
		{
			variable(GLenum i = GL_ZERO, GLenum c = GL_RGB, GLenum m = GL_UNSIGNED_IDENTITY_NV)
				: input(i), mapping(m), component(c) {}
			GLenum input, mapping, component;
		};

		variable var(GLenum input = GL_ZERO, GLenum component_usage = GL_RGB, GLenum mapping = GL_UNSIGNED_IDENTITY_NV)
		{ return variable(input, component_usage, mapping); }

		variable a, b, c, d, e, f, g;
	};

#endif

}  // namespace glh

#endif // GLH_COMBINERS_H
