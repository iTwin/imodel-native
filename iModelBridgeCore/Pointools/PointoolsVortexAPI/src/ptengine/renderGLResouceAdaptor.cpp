#include <gl/glew.h>
#include <ptengine/renderGLResourceAdaptor.h>
#include <ptgl/glshader.h>

using namespace pointsengine;


/*****************************************************************************/
/**
* @brief
* @return 
*/
/*****************************************************************************/
OpenGLResourceAdaptor::OpenGLResourceAdaptor() 
{

}
/*****************************************************************************/
/**
* @brief
* @param gradientID
* @return PTRES_TEXID
*/
/*****************************************************************************/
Texture1D		*OpenGLResourceAdaptor::createGradientTexture( int gradientID ) const
{
	ColourGradient *gr = RenderResourceManager::instance()->
		gradientManager()->getGradientByIndex( gradientID );

	Texture1D *tex = new Texture1D;
	tex->m_width = gr->m_gradient.imgWidth();
	tex->m_height = gr->m_gradient.imgHeight();
	tex->m_imgData = gr->m_gradient.img();
	
	setupGradientTexture( tex );

	return tex;
}
/*****************************************************************************/
/**
* @brief
* @param tex
* @return void
*/
/*****************************************************************************/
void		OpenGLResourceAdaptor::setupGradientTexture( Texture1D *tex ) const
{
	/* Generate a texture with the associative texture ID stored in the array*/ 
	GLuint texid=0;
	glGenTextures(1, &texid);
	tex->m_texid = (PTRES_TEXID)texid;

	/* Bind the texture to the index and init the texture*/ 
	glBindTexture(GL_TEXTURE_1D, tex->m_texid);

	/* Build Mipmaps for LOD*/ 
	gluBuild1DMipmaps(GL_TEXTURE_1D, 3, tex->m_width, /*height,*/ GL_BGRA, GL_UNSIGNED_BYTE, tex->m_imgData);

	/*texture quality		*/ 
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_1D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
}
/*****************************************************************************/
/**
* @brief
* @param name
* @return PTRES_TEXID
*/
/*****************************************************************************/
Texture1D		*OpenGLResourceAdaptor::createGradientTexture( const char* name ) const
{
	ColourGradient *gr = RenderResourceManager::instance()->
		gradientManager()->getGradientByName( pt::String(name) );

	Texture1D *tex = new Texture1D;
	tex->m_width = gr->m_gradient.imgWidth();
	tex->m_height = gr->m_gradient.imgHeight();
	tex->m_imgData = gr->m_gradient.img();

	setupGradientTexture( tex );

	return tex;
}
/*****************************************************************************/
/**
* @brief
* @param name
* @return LPVOIDSHADER
*/
/*****************************************************************************/
LPVOIDSHADER	OpenGLResourceAdaptor::createShaderObj( const char* name, const char *precompiler ) const
{
	static ptgl::ShaderManager shaderMan;

	char *fprog = RenderResourceManager::instance()->getShaderScript( "pshader_frag" );
	char *vprog = RenderResourceManager::instance()->getShaderScript( "pshader_vert" );

	// add in version + precompiler
	pt::String fprog_s;
	fprog_s.format("#version 120\n%s\n%s", precompiler ? precompiler : " ", fprog);

	pt::String vprog_s;
	vprog_s.format("#version 120\n%s\n%s", precompiler ? precompiler : " ", vprog);

	ptgl::Shader *shader = shaderMan.loadfromMemory( vprog_s.c_str(), fprog_s.c_str() );
	return shader;
}
