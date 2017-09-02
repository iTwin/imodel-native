ScalableMesh OpenGL display example
(C) Bentley Systems 2017, All Rights Reserved

The SM_DrawExample project is a simple example of how to open and display a 3sm file. 
You can specify the file to open on the command line

	Viewer -o filename.3sm 

or edit the viewer_config.ini file in the config folder to specify which files to open

The solution contains 2 projects. The 3smGL project shows how to use the ScalableMesh SDK
to render a mesh in OpenGL. The Viewer project simply provides the OpenGL based Viewer 
framework for the example but does not have any ScalableMesh specfic code. 




