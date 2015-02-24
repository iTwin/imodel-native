/******************************************************************************

Pointools Vortex API Examples

FeatureExtractTool.h

Demonstrates plane and cylinder extraction capabilities of VortexFeatures library

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_FEATURE_EXTRACT_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_FEATURE_EXTRACT_TOOL_H_

#include "VortexExampleApp.h"
#include "VortexFeatureExtract.h"
#include "DiagnosticRenderer.h"

class FeatureExtractTool : public Tool
{
public:
	FeatureExtractTool();

	enum
	{
		CmdFitCylinder= 500,
		CmdFitPlane = 501
	};

	void	buildUserInterface(GLUI_Node *parent);
	void	command( int cmdId );
	void	drawPostDisplay();

private:
	vortex::Cylinderf	cylinder;
	vortex::Point3f		planeCorners[4];

	DiagnosticRenderer	m_renderer;
	bool				hasCylinder;
	bool				hasPlane;
};

#endif

