// precompiled header
#include "stdafx.h"

// application framework implementation. 
// Included as src to enable header file resolution
#include "..\src\vortexExampleApp.cpp"

// functionality provided by tool classes
#include "..\include\NavigationTool.h"
#include "..\include\FileTool.h"
#include "..\include\ShaderTool.h"
#include "..\include\EditTool.h"
#include "..\include\VortexRender.h"
#include "..\include\BitmapTool.h"

#include "FeatureExtractTest.h"

int main(int argc, char* argv[])
{
	VortexExampleApp example( std::string("Feature Extraction Test") );

	if (example.initializeApplication(argc, argv, false))
	{
		example.addTool(new FileTool);
		example.addTool(new ShaderTool);
		example.addTool(new EditTool(true));
		example.addTool(new FeatureExtractTool);
		example.addTool(new CameraNavigation);		// must be last so it receives events last

		if (!example.getRenderer())
			example.setRenderer( new VortexRender );	// default Vortex OpenGL render if no Tool has set renderer

		example.runApplication();
	}
	return EXIT_SUCCESS;
}
