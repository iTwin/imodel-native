/******************************************************************************

Pointools Vortex API Examples

clashTool.h

Demonstrates clash capabilities of VortexFeatures library

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_CLASH_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_CLASH_TOOL_H_

#include "VortexExampleApp.h"
#include <VortexFeatureExtract.h>
#include "DiagnosticRenderer.h"
#include <Vortex/IClashObject.h>
#include <GeomTypes.h>
#include <map>


class ClashTool : public Tool, public vortex::IClashObjectCallback
{
public:
	ClashTool();	

	enum
	{		
		CmdGenerateClashTree = 1201
	};

	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );
	void	drawPostDisplay();

	// From vortex::IClashObjectCallback, to allow clash tree generation info to be received
	PTvoid	clashTreeGenerationProgress(PTint pcentComplete);

private:
	void computeVertices(float* extents, double* center, float* xAxis, float* yAxis, float* zAxis, vortex::Vector3d* vertex);

	DiagnosticRenderer	m_renderer;
	
	typedef std::map<PThandle, vortex::IClashObject*> ClashObjectMap;
	ClashObjectMap m_clashObjects;

	bool m_displayBoxes;

	std::string		m_clashInfoString;

	GLUI_TextBox*	m_infoTextBox;
};

#endif

