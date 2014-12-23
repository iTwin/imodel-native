/****************************************************************************

Pointools Vortex API usage example

* SNAPPING EXAMPLE *

(c) Copyright 2008-11 Pointools Ltd

These example uses GLUT and GLUI to set up a simple UI and demonstrate
the use of the Pointools Vortex API those a series of examples


CONFIDENTIAL 
--------------------------
This source code is provided as confidential material. License to use this
source code is provided for the purpose of evaluation only. 

This source code may not be passed onto any third party without prior consent 
from Pointools Ltd


LICENSE
--------------------------
License to use this source code is granted for the purpose of evaluation 
only provided that:

1. All marks and software identification including but not
   limited to logos, application title and text are not altered and no attempt
   is made to alter the apparent origin or author of the work.

2. The source code and complied binary application is treated as confidential

No transfer of ownership, intellectual property rights or software usage rights
whatsoever is implied by the provision of this source code.


USE OF GLUI
--------------------------
GLUI is used under the LGPL license. The modified version of the source code 
of GLUI will be provided on request - contact vortex@pointools.com if you need
this.

This Example
--------------------------
This example demonstrates the implementation of point snapping using ray 
intersection and nearest point

****************************************************************************/

// functionality provided by tool classes
#include "..\include\NavigationTool.h"	// camera navigation
#include "..\include\FileTool.h"		// file open/close
#include "..\include\ShaderTool.h"		// shading options
#include "..\include\EditTool.h"		// shading options
#include "..\include\CloudVisibilityTool.h"		// shading options
#include "..\include\ClippingTool.h"
#include "..\include\VortexRender.h"	// Vortex OpenGL rendering
#include "SnapTest.h"					

// Main Entry Point
int main(int argc, char* argv[])
{
	VortexExampleApp example( std::string("Snapping Example") );

	if (example.initializeApplication(argc, argv, false))
	{
		example.addTool(new FileTool);
		example.addTool(new ShaderTool);
		example.addTool(new SnapTest);					// see code in SnapTest.cpp for implementation of snapping
		example.addTool(new EditTool(false));
		example.addTool(new CloudVisibilityTool);		// for switching the visibility of whole points clouds on and off
		example.addTool(new ClippingTool(false));				// for enabling clipping and enabling/disabling clipping planes
		example.addTool(new CameraNavigation);			// must be last so it receives events last

		if (!example.getRenderer())
			example.setRenderer( new VortexRender );	// default Vortex OpenGL render if no Tool has set renderer

		example.runApplication();	
	}
	return EXIT_SUCCESS;
}
