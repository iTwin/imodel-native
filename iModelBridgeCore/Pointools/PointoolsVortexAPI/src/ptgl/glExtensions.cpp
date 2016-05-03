#include "PointoolsVortexAPIInternal.h"
#include <ptgl/glExtensions.h>
#include <gl/gl.h>
#include <set>
#include <string>

#pragma warning ( disable : 4786 )

typedef std::set<std::string>  ExtensionSet;
static ExtensionSet s_extensionSet;
static const GLubyte* s_extensions = NULL;
static char error[256];

using namespace ptgl;

bool Ext::load_extensions()
{
    if (s_extensions==NULL)
    {
        // get the extension list from OpenGL.
        s_extensions = glGetString(GL_EXTENSIONS);
        if (s_extensions==NULL) return false;

        // insert the ' ' delimiated extensions words into the extensionSet.
        const char *startOfWord = (const char *)s_extensions;
        const char *endOfWord;
        while ((endOfWord = strchr(startOfWord,' '))!=NULL)
        {
            s_extensionSet.insert(std::string(startOfWord,endOfWord));
            startOfWord = endOfWord+1;
        }
        if (*startOfWord!=0) s_extensionSet.insert(std::string(startOfWord));            
		return true;
    }
	return true;
}
bool Ext::isSupported(const char *extension)
{
	load_extensions();
    return  s_extensionSet.find(extension)!=s_extensionSet.end();
}

PROC Ext::getEntryPoint(const char*extension, const char *function)
{
	if (isSupported(extension))
	{
		return wglGetProcAddress(function);;
	}
	return NULL;
}