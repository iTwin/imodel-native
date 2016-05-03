/*
	ARB Vertex Program source file.
	By Nutty
*/

#include <ptgl/arb_vertex_program.h>

//The actual function pointer instances.
PFNGLVERTEXATTRIB1SARB glVertexAttrib1sARB;
PFNGLVERTEXATTRIB1FARB glVertexAttrib1fARB;
PFNGLVERTEXATTRIB1DARB glVertexAttrib1dARB;
PFNGLVERTEXATTRIB2SARB glVertexAttrib2sARB;
PFNGLVERTEXATTRIB2FARB glVertexAttrib2fARB;
PFNGLVERTEXATTRIB2DARB glVertexAttrib2dARB;
PFNGLVERTEXATTRIB3SARB glVertexAttrib3sARB;
PFNGLVERTEXATTRIB3FARB glVertexAttrib3fARB;
PFNGLVERTEXATTRIB3DARB glVertexAttrib3dARB;
PFNGLVERTEXATTRIB4SARB glVertexAttrib4sARB;
PFNGLVERTEXATTRIB4FARB glVertexAttrib4fARB;
PFNGLVERTEXATTRIB4DARB glVertexAttrib4dARB;
PFNGLVERTEXATTRIB4NUBARB glVertexAttrib4NubARB;

PFNGLVERTEXATTRIB1SVARB glVertexAttrib1svARB;
PFNGLVERTEXATTRIB1FVARB glVertexAttrib1fvARB;
PFNGLVERTEXATTRIB1DVARB glVertexAttrib1dvARB;
PFNGLVERTEXATTRIB2SVARB glVertexAttrib2svARB;
PFNGLVERTEXATTRIB2FVARB glVertexAttrib2fvARB;
PFNGLVERTEXATTRIB2DVARB glVertexAttrib2dvARB;
PFNGLVERTEXATTRIB3SVARB glVertexAttrib3svARB;
PFNGLVERTEXATTRIB3FVARB glVertexAttrib3fvARB;
PFNGLVERTEXATTRIB3DVARB glVertexAttrib3dvARB;
PFNGLVERTEXATTRIB4BVARB glVertexAttrib4bvARB;
PFNGLVERTEXATTRIB4SVARB glVertexAttrib4svARB;
PFNGLVERTEXATTRIB4IVARB glVertexAttrib4ivARB;
PFNGLVERTEXATTRIB4UBVARB glVertexAttrib4ubvARB;
PFNGLVERTEXATTRIB4USVARB glVertexAttrib4usvARB;
PFNGLVERTEXATTRIB4UIVARB glVertexAttrib4uivARB;
PFNGLVERTEXATTRIB4FVARB glVertexAttrib4fvARB;
PFNGLVERTEXATTRIB4DVARB glVertexAttrib4dvARB;
PFNGLVERTEXATTRIB4NBVARB glVertexAttrib4NbvARB;
PFNGLVERTEXATTRIB4NSVARB glVertexAttrib4NsvARB;
PFNGLVERTEXATTRIB4NIVARB glVertexAttrib4NivARB;
PFNGLVERTEXATTRIB4NUBVARB glVertexAttrib4NubvARB;
PFNGLVERTEXATTRIB4NUSVARB glVertexAttrib4NusvARB;
PFNGLVERTEXATTRIB4NUIVARB glVertexAttrib4NuivARB;

PFNGLVERTEXATTRIBPOINTERARB glVertexAttribPointerARB;
PFNGLENABLEVERTEXATTRIBARRAYARB glEnableVertexAttribArrayARB;
PFNGLDISABLEVERTEXATTRIBARRAYARB glDisableVertexAttribArrayARB;
PFNGLPROGRAMSTRINGARB glProgramStringARB;
PFNGLBINDPROGRAMARB glBindProgramARB;
PFNGLDELETEPROGRAMSARB glDeleteProgramsARB;
PFNGLGENPROGRAMSARB glGenProgramsARB;

PFNGLPROGRAMENVPARAMETER4DARB glProgramEnvParameter4dARB;
PFNGLPROGRAMENVPARAMETER4DVARB glProgramEnvParameter4dvARB;
PFNGLPROGRAMENVPARAMETER4FARB glProgramEnvParameter4fARB;
PFNGLPROGRAMENVPARAMETER4FVARB glProgramEnvParameter4fvARB;

PFNGLPROGRAMLOCALPARAMETER4DARB glProgramLocalParameter4dARB;
PFNGLPROGRAMLOCALPARAMETER4DVARB glProgramLocalParameter4dvARB;
PFNGLPROGRAMLOCALPARAMETER4FARB glProgramLocalParameter4fARB;
PFNGLPROGRAMLOCALPARAMETER4FVARB glProgramLocalParameter4fvARB;

PFNGLGETPROGRAMENVPARAMETERDVARB glGetProgramEnvParameterdvARB;
PFNGLGETPROGRAMENVPARAMETERFVARB glGetProgramEnvParameterfvARB;

PFNGLGETPROGRAMLOCALPARAMETERDVARB glGetProgramLocalParameterdvARB;
PFNGLGETPROGRAMLOCALPARAMETERFVARB glGetProgramLocalParameterfvARB;

PFNGLGETPROGRAMIVARB glGetProgramivARB;
PFNGLGETPROGRAMSTRINGARB glGetProgramStringARB;
PFNGLGETVERTEXATTRIBDVARB glGetVertexAttribdvARB;
PFNGLGETVERTEXATTRIBFVARB glGetVertexAttribfvARB;
PFNGLGETVERTEXATTRIBIVARB glGetVertexAttribivARB;
PFNGLGETVERTEXATTRIBPOINTERVARB glGetVertexAttribPointervARB;
PFNGLISPROGRAMARB glIsProgramARB;
 

bool ptgl::InitARBVertexProgram(void)
{
	glVertexAttrib1sARB = (PFNGLVERTEXATTRIB1SARB) wglGetProcAddress("glVertexAttrib1sARB");
	if(!glVertexAttrib1sARB)
	{
		return false;
	}

	glVertexAttrib1fARB = (PFNGLVERTEXATTRIB1FARB) wglGetProcAddress("glVertexAttrib1fARB");
	if(!glVertexAttrib1fARB)
	{
		return false;
	}

	glVertexAttrib1dARB = (PFNGLVERTEXATTRIB1DARB) wglGetProcAddress("glVertexAttrib1dARB");
	if(!glVertexAttrib1dARB)
	{
		return false;
	}

	glVertexAttrib2sARB = (PFNGLVERTEXATTRIB2SARB) wglGetProcAddress("glVertexAttrib2sARB");
	if(!glVertexAttrib2sARB)
	{
		return false;
	}

	glVertexAttrib2fARB = (PFNGLVERTEXATTRIB2FARB) wglGetProcAddress("glVertexAttrib2fARB");
	if(!glVertexAttrib2fARB)
	{
		return false;
	}

	glVertexAttrib2dARB = (PFNGLVERTEXATTRIB2DARB) wglGetProcAddress("glVertexAttrib2dARB");
	if(!glVertexAttrib2dARB)
	{
		return false;
	}

	glVertexAttrib3sARB = (PFNGLVERTEXATTRIB3SARB) wglGetProcAddress("glVertexAttrib3sARB");
	if(!glVertexAttrib3sARB)
	{
		return false;
	}

	glVertexAttrib3fARB = (PFNGLVERTEXATTRIB3FARB) wglGetProcAddress("glVertexAttrib3fARB");
	if(!glVertexAttrib3fARB)
	{
		return false;
	}

	glVertexAttrib3dARB = (PFNGLVERTEXATTRIB3DARB) wglGetProcAddress("glVertexAttrib3dARB");
	if(!glVertexAttrib3dARB)
	{
		return false;
	}

	glVertexAttrib4sARB = (PFNGLVERTEXATTRIB4SARB) wglGetProcAddress("glVertexAttrib4sARB");
	if(!glVertexAttrib4sARB)
	{
		return false;
	}

	glVertexAttrib4fARB = (PFNGLVERTEXATTRIB4FARB) wglGetProcAddress("glVertexAttrib4fARB");
	if(!glVertexAttrib4fARB)
	{
		return false;
	}

	glVertexAttrib4dARB = (PFNGLVERTEXATTRIB4DARB) wglGetProcAddress("glVertexAttrib4dARB");
	if(!glVertexAttrib4dARB)
	{
		return false;
	}

	glVertexAttrib4NubARB = (PFNGLVERTEXATTRIB4NUBARB) wglGetProcAddress("glVertexAttrib4NubARB");
	if(!glVertexAttrib4NubARB)
	{
		return false;
	}


	glVertexAttrib1svARB = (PFNGLVERTEXATTRIB1SVARB) wglGetProcAddress("glVertexAttrib1svARB");
	if(!glVertexAttrib1svARB)
	{
		return false;
	}

	glVertexAttrib1fvARB = (PFNGLVERTEXATTRIB1FVARB) wglGetProcAddress("glVertexAttrib1fvARB");
	if(!glVertexAttrib1fvARB)
	{
		return false;
	}

	glVertexAttrib1dvARB = (PFNGLVERTEXATTRIB1DVARB) wglGetProcAddress("glVertexAttrib1dvARB");
	if(!glVertexAttrib1dvARB)
	{
		return false;
	}

	glVertexAttrib2svARB = (PFNGLVERTEXATTRIB2SVARB) wglGetProcAddress("glVertexAttrib2svARB");
	if(!glVertexAttrib2svARB)
	{
		return false;
	}

	glVertexAttrib2fvARB = (PFNGLVERTEXATTRIB2FVARB) wglGetProcAddress("glVertexAttrib2fvARB");
	if(!glVertexAttrib2fvARB)
	{
		return false;
	}

	glVertexAttrib2dvARB = (PFNGLVERTEXATTRIB2DVARB) wglGetProcAddress("glVertexAttrib2dvARB");
	if(!glVertexAttrib2dvARB)
	{
		return false;
	}

	glVertexAttrib3svARB = (PFNGLVERTEXATTRIB3SVARB) wglGetProcAddress("glVertexAttrib3svARB");
	if(!glVertexAttrib3svARB)
	{
		return false;
	}

	glVertexAttrib3fvARB = (PFNGLVERTEXATTRIB3FVARB) wglGetProcAddress("glVertexAttrib3fvARB");
	if(!glVertexAttrib3fvARB)
	{
		return false;
	}

	glVertexAttrib3dvARB = (PFNGLVERTEXATTRIB3DVARB) wglGetProcAddress("glVertexAttrib3dvARB");
	if(!glVertexAttrib3dvARB)
	{
		return false;
	}

	glVertexAttrib4bvARB = (PFNGLVERTEXATTRIB4BVARB) wglGetProcAddress("glVertexAttrib4bvARB");
	if(!glVertexAttrib4bvARB)
	{
		return false;
	}

	glVertexAttrib4svARB = (PFNGLVERTEXATTRIB4SVARB) wglGetProcAddress("glVertexAttrib4svARB");
	if(!glVertexAttrib4svARB)
	{
		return false;
	}

	glVertexAttrib4ivARB = (PFNGLVERTEXATTRIB4IVARB) wglGetProcAddress("glVertexAttrib4ivARB");
	if(!glVertexAttrib4ivARB)
	{
		return false;
	}

	glVertexAttrib4ubvARB = (PFNGLVERTEXATTRIB4UBVARB) wglGetProcAddress("glVertexAttrib4ubvARB");
	if(!glVertexAttrib4ubvARB)
	{
		return false;
	}

	glVertexAttrib4usvARB = (PFNGLVERTEXATTRIB4USVARB) wglGetProcAddress("glVertexAttrib4usvARB");
	if(!glVertexAttrib4usvARB)
	{
		return false;
	}

	glVertexAttrib4uivARB = (PFNGLVERTEXATTRIB4UIVARB) wglGetProcAddress("glVertexAttrib4uivARB");
	if(!glVertexAttrib4uivARB)
	{
		return false;
	}

	glVertexAttrib4fvARB = (PFNGLVERTEXATTRIB4FVARB) wglGetProcAddress("glVertexAttrib4fvARB");
	if(!glVertexAttrib4fvARB)
	{
		return false;
	}

	glVertexAttrib4dvARB = (PFNGLVERTEXATTRIB4DVARB) wglGetProcAddress("glVertexAttrib4dvARB");
	if(!glVertexAttrib4dvARB)
	{
		return false;
	}

	glVertexAttrib4NbvARB = (PFNGLVERTEXATTRIB4NBVARB) wglGetProcAddress("glVertexAttrib4NbvARB");
	if(!glVertexAttrib4NbvARB)
	{
		return false;
	}

	glVertexAttrib4NsvARB = (PFNGLVERTEXATTRIB4NSVARB) wglGetProcAddress("glVertexAttrib4NsvARB");
	if(!glVertexAttrib4NsvARB)
	{
		return false;
	}

	glVertexAttrib4NivARB = (PFNGLVERTEXATTRIB4NIVARB) wglGetProcAddress("glVertexAttrib4NivARB");
	if(!glVertexAttrib4NivARB)
	{
		return false;
	}

	glVertexAttrib4NubvARB = (PFNGLVERTEXATTRIB4NUBVARB) wglGetProcAddress("glVertexAttrib4NubvARB");
	if(!glVertexAttrib4NubvARB)
	{
		return false;
	}

	glVertexAttrib4NusvARB = (PFNGLVERTEXATTRIB4NUSVARB) wglGetProcAddress("glVertexAttrib4NusvARB");
	if(!glVertexAttrib4NusvARB)
	{
		return false;
	}

	glVertexAttrib4NuivARB = (PFNGLVERTEXATTRIB4NUIVARB) wglGetProcAddress("glVertexAttrib4NuivARB");
	if(!glVertexAttrib4NuivARB)
	{
		return false;
	}


	glVertexAttribPointerARB = (PFNGLVERTEXATTRIBPOINTERARB) wglGetProcAddress("glVertexAttribPointerARB");
	if(!glVertexAttribPointerARB)
	{
		return false;
	}

	glEnableVertexAttribArrayARB = (PFNGLENABLEVERTEXATTRIBARRAYARB) wglGetProcAddress("glEnableVertexAttribArrayARB");
	if(!glEnableVertexAttribArrayARB)
	{
		return false;
	}

	glDisableVertexAttribArrayARB = (PFNGLDISABLEVERTEXATTRIBARRAYARB) wglGetProcAddress("glDisableVertexAttribArrayARB");
	if(!glDisableVertexAttribArrayARB)
	{
		return false;
	}

	glProgramStringARB = (PFNGLPROGRAMSTRINGARB) wglGetProcAddress("glProgramStringARB");
	if(!glProgramStringARB)
	{
		return false;
	}

	glBindProgramARB = (PFNGLBINDPROGRAMARB) wglGetProcAddress("glBindProgramARB");
	if(!glBindProgramARB)
	{
		return false;
	}

	glDeleteProgramsARB = (PFNGLDELETEPROGRAMSARB) wglGetProcAddress("glDeleteProgramsARB");
	if(!glDeleteProgramsARB)
	{
		return false;
	}

	glGenProgramsARB = (PFNGLGENPROGRAMSARB) wglGetProcAddress("glGenProgramsARB");
	if(!glGenProgramsARB)
	{
		return false;
	}

	glProgramEnvParameter4dARB = (PFNGLPROGRAMENVPARAMETER4DARB) wglGetProcAddress("glProgramEnvParameter4dARB");
	if(!glProgramEnvParameter4dARB)
	{
		return false;
	}

	glProgramEnvParameter4dvARB = (PFNGLPROGRAMENVPARAMETER4DVARB) wglGetProcAddress("glProgramEnvParameter4dvARB");
	if(!glProgramEnvParameter4dvARB)
	{
		return false;
	}

	glProgramEnvParameter4fARB = (PFNGLPROGRAMENVPARAMETER4FARB) wglGetProcAddress("glProgramEnvParameter4fARB");
	if(!glProgramEnvParameter4fARB)
	{
		return false;
	}

	glProgramEnvParameter4fvARB = (PFNGLPROGRAMENVPARAMETER4FVARB) wglGetProcAddress("glProgramEnvParameter4fvARB");
	if(!glProgramEnvParameter4fvARB)
	{
		return false;
	}

	glProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARB) wglGetProcAddress("glProgramLocalParameter4dARB");
	if(!glProgramLocalParameter4dARB)
	{
		return false;
	}

	glProgramLocalParameter4dvARB = (PFNGLPROGRAMLOCALPARAMETER4DVARB) wglGetProcAddress("glProgramLocalParameter4dvARB");
	if(!glProgramLocalParameter4dvARB)
	{
		return false;
	}

	glProgramLocalParameter4fARB = (PFNGLPROGRAMLOCALPARAMETER4FARB) wglGetProcAddress("glProgramLocalParameter4fARB");
	if(!glProgramLocalParameter4fARB)
	{
		return false;
	}

	glProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARB) wglGetProcAddress("glProgramLocalParameter4fvARB");
	if(!glProgramLocalParameter4fvARB)
	{
		return false;
	}

	glGetProgramEnvParameterdvARB = (PFNGLGETPROGRAMENVPARAMETERDVARB) wglGetProcAddress("glGetProgramEnvParameterdvARB");
	if(!glGetProgramEnvParameterdvARB)
	{
		return false;
	}

	glGetProgramEnvParameterfvARB = (PFNGLGETPROGRAMENVPARAMETERFVARB) wglGetProcAddress("glGetProgramEnvParameterfvARB");
	if(!glGetProgramEnvParameterfvARB)
	{
		return false;
	}

	glGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARB) wglGetProcAddress("glGetProgramLocalParameterdvARB");
	if(!glGetProgramLocalParameterdvARB)
	{
		return false;
	}

	glGetProgramLocalParameterfvARB = (PFNGLGETPROGRAMLOCALPARAMETERFVARB) wglGetProcAddress("glGetProgramLocalParameterfvARB");
	if(!glGetProgramLocalParameterfvARB)
	{
		return false;
	}

	glGetProgramivARB = (PFNGLGETPROGRAMIVARB) wglGetProcAddress("glGetProgramivARB");
	if(!glGetProgramivARB)
	{
		return false;
	}

	glGetProgramStringARB = (PFNGLGETPROGRAMSTRINGARB) wglGetProcAddress("glGetProgramStringARB");
	if(!glGetProgramStringARB)
	{
		return false;
	}

	glGetVertexAttribdvARB = (PFNGLGETVERTEXATTRIBDVARB) wglGetProcAddress("glGetVertexAttribdvARB");
	if(!glGetVertexAttribdvARB)
	{
		return false;
	}

	glGetVertexAttribfvARB = (PFNGLGETVERTEXATTRIBFVARB) wglGetProcAddress("glGetVertexAttribfvARB");
	if(!glGetVertexAttribfvARB)
	{
		return false;
	}

	glGetVertexAttribivARB = (PFNGLGETVERTEXATTRIBIVARB) wglGetProcAddress("glGetVertexAttribivARB");
	if(!glGetVertexAttribivARB)
	{
		return false;
	}

	glGetVertexAttribPointervARB = (PFNGLGETVERTEXATTRIBPOINTERVARB) wglGetProcAddress("glGetVertexAttribPointervARB");
	if(!glGetVertexAttribPointervARB)
	{
		return false;
	}

	glIsProgramARB = (PFNGLISPROGRAMARB) wglGetProcAddress("glIsProgramARB");
	if(!glIsProgramARB)
	{
		return false;
	}



	return true;
}