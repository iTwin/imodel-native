/****************************************************************************

Simple Example to demonstrate use of ScalableMesh library

* SIMPLE EXAMPLE *

Copyright (c) 2017 Bentley Systems, Incorporated. All rights reserved.

These example uses GLUT and GLUI to set up a simple UI and demonstrate
the use of the ScalableMesh SDK to display a 3sm file in OpenGL. The Viewer
Project does not directly call ScalableMesh, this is done through the 
3smGL helper library that is part of the solution. 


CONFIDENTIAL 
--------------------------
This source code is provided as confidential material. License to use this
source code is provided for the purpose of evaluation only. 

This source code may not be passed onto any third party without prior consent 
from Bentley Systems


USE OF GLUI
--------------------------
GLUI is used under the LGPL license. 

****************************************************************************/

// functionality provided by tool classes
#include "..\include\NavigationTool.h"	// camera navigation
#include "DemoTool.h"
#include "SimpleSky.h"

// Main Entry Point
int main(int argc, char* argv[])
{ 
	ExampleApp example( std::string("Bentley Reality Modeling | Viewer") );

	if (example.initializeApplication(argc, argv, true))
	{ 
		example.addTool(new SimpleSky);
		example.addTool(new SM_DemoTool);				
		example.addTool(new CameraNavigation);			// must be last so it receives events last

		example.initializeTools(argc, argv);

		example.runApplication();	
	}
	return EXIT_SUCCESS;
}
