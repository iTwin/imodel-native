#include "PointoolsVortexAPIInternal.h"
#include <ptengine/renderResourceManager.h>
#include <pt/typedefs.h>
#include <set>

using namespace pointsengine;

namespace 
{ 
	RenderResourceManager *g_resManInstance=0;

	//embedded resources
	#include <pshader.frag.cpp>	// as generated from pshader.frag by encryptshader.exe
	#include <pshader.vert.cpp>	// as generated from pshader.vert by encryptshader.exe

	std::set<char*> g_scripts;

}
/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
RenderResourceManager::RenderResourceManager()
{
	createDefaultResources();
}
/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
void RenderResourceManager::createDefaultResources()
{
	if (!m_gradients.numColourGradients())			//test for existing resources
	{
		m_gradients.createDefaultGradients();	
	}
}
/*****************************************************************************/
/**
* @brief
* @return RenderResourceManager	*
*/
/*****************************************************************************/
RenderResourceManager	*RenderResourceManager::instance()
{
	if (!g_resManInstance)	g_resManInstance = new RenderResourceManager();			// internal static not good enough
	return g_resManInstance;														// in mt situations
}

/*****************************************************************************/
/**
* @brief
* @param name 
* @return const char*
*/
/*****************************************************************************/
char*				RenderResourceManager::getShaderScript( const char* name )
{
	static const char* hmask = "Vertex program initialization";

	struct decrypt
	{
		static int process(const ubyte *input, char *txt)
		{
			int i = 0;
			while ((unsigned char)input[i]< 0xaa)
			{
				txt[i]  = input[i] ^ hmask[i%29];
				++i;
			}
			txt[i] = '\n';
			return ++i;
		}
	};

	const ubyte* ct = 0;
	int ctlen = 0;

	if (strcmp(name, "pshader_frag" ) == 0)
	{
		ct = pshader_frag;
		ctlen = sizeof(pshader_frag);
	}
	else	if (strcmp(name, "pshader_vert" ) == 0)
	{
		ct = pshader_vert;
		ctlen = sizeof(pshader_vert);
	}
	else return 0;

	char *spt = new char[ ctlen+512 ];
	int i=0;

	for (int p=0; p<ctlen; p++)
		spt[i++] = ' ';//ct[p];

	i=0;
	int ii=0;
	while (i<ctlen)
	{
		int chars_processed = decrypt::process(&ct[i], &spt[ii]);
		i +=  chars_processed;
		ii += chars_processed;
	//	spt[++ii] = '\r';
	} 
	spt[--ii] = 0;
	spt[++ii] = 0;

	g_scripts.insert(spt);

	return spt;
}
/*****************************************************************************/
/**
* @brief
* @return ColourGradientManager	*
*/
/*****************************************************************************/
ColourGradientManager	*RenderResourceManager::gradientManager()
{
	return &m_gradients;
}
/*****************************************************************************/
/**
* @brief
* @param script
* @return void
*/
/*****************************************************************************/
void RenderResourceManager::releaseShaderScript( char *script )
{
	if (script)
	{
		std::set<char*>::iterator i = g_scripts.find(script);
		if (i!=g_scripts.end())
		{
			delete [] script;
			g_scripts.erase(i);
		}
	}
}
