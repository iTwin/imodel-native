/******************************************************************************

Pointools Vortex API Examples

ShaderTool.h

Demonstrates a number of shading options available in Vortex

(c) Copyright 2008-11 Pointools Ltd

*******************************************************************************/
#ifndef POINTOOLS_EXAMPLE_APP_SHADER_TOOL_H_
#define POINTOOLS_EXAMPLE_APP_SHADER_TOOL_H_

#include "VortexExampleApp.h"

//! VortexShaderOptions
//! Basic encapsulation of Vortex shader options 
class ShaderOptions
{
public:
	ShaderOptions();

	bool	initialize();
	void	updateVortex();

	int		useRGB;
	int		useIntensity;
	int		useLighting;
	int		useClassification;
	int		usePlane;

	float	contrast;
	float	brightness;
	float	fps;

	float	dist;
	float	off;

	/* generic use */ 
	float	mixer;

	int		adaptivePointSize;
	int		frontBias;

	int		intensityRamp;
	int		planeRamp;
	int		planeAxis;

	int		selectMode;

	std::vector< std::string > intensityRamps;
	std::vector< std::string > planeRamps;

private:
	void	loadRamps();
};

class ShaderTool : public Tool
{
public:
	enum
	{
		CmdShaderUpdate = 200
	};

	ShaderTool();
	
	void	buildUserInterface(GLUI_Node *parent);
	void	drawPreDisplay();
	void	command( int cmdId );

private:
	void	handleClassificationShader();

	ShaderOptions	m_shader;
	Renderer		*m_lastRenderer;
	Renderer		*m_classificationRenderer;
};

#endif

