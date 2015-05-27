/******************************************************************************

Pointools Vortex API Examples

ShaderTool.h

Demonstrates a number of shading options available in Vortex

Copyright (c) 2015 Bentley Systems, Incorporated. All rights reserved.

*******************************************************************************/
#include "ShaderTool.h"
#include "ClassificationRender.h"

//-----------------------------------------------------------------------------
ShaderTool::ShaderTool() : Tool(CmdShaderUpdate, CmdIncIntensity)
//-----------------------------------------------------------------------------
{
	m_lastRenderer=0;
	m_classificationRenderer=0;
}

//-----------------------------------------------------------------------------
void ShaderTool::command( int cmdId )
//-----------------------------------------------------------------------------
{
	if (cmdId == CmdShaderUpdate)
	{
		m_shader.updateVortex();

		handleClassificationShader();

		viewRedraw();
	}
	else if (cmdId == CmdIncIntensity)
	{
		int numRamps = ptNumRamps();
		m_shader.intensityRamp++;
		if (m_shader.intensityRamp >= numRamps)
			m_shader.intensityRamp = 1;

		m_shader.updateVortex();

		handleClassificationShader();

		viewRedraw();
	}
}
//-----------------------------------------------------------------------------
void	ShaderTool::drawPreDisplay()
//-----------------------------------------------------------------------------
{
	m_shader.updateVortex();
}
//-----------------------------------------------------------------------------
ShaderOptions::ShaderOptions()
//-----------------------------------------------------------------------------
{
	useRGB = 1;
	useIntensity = 0;
	usePlane = 0;
	useLighting = 0;
	useClassification = 0;

	mixer = 0;
	selectMode = 0;

	contrast = 50.0f;
	brightness = 180.0f;

	adaptivePointSize = 1;
	frontBias = 0;

	intensityRamp = 2;
	planeRamp = 3;
	planeAxis = 2;

	fps = 15.0f;
	off = 0;
	dist = 10.0f;
}
//-----------------------------------------------------------------------------
bool ShaderOptions::initialize()
//-----------------------------------------------------------------------------
{
	loadRamps();

	GLubyte selCol[] = { 0, 255, 255 };
	ptSetSelectionDrawColor( selCol );

	return true; // nothing much to go wrong
}
//-----------------------------------------------------------------------------
void ShaderOptions::loadRamps()
//-----------------------------------------------------------------------------
{
	int numRamps = ptNumRamps();
	
	int i;
	int items = 0;

	for (i=0; i<numRamps; i++)
	{
		PTenum rtype;
		const wchar_t* wname =	ptRampInfo( i, &rtype );
		std::string name( wcToAscii(wname) );

		if (rtype & PT_INTENSITY_RAMP_TYPE)
		{
			intensityRamps.push_back(name);
		}
	}
}

//-----------------------------------------------------------------------------
void ShaderOptions::updateVortex()
//-----------------------------------------------------------------------------
{
	if ( useRGB ) ptEnable( PT_RGB_SHADER );
	else ptDisable( PT_RGB_SHADER );

	if ( useIntensity ) ptEnable( PT_INTENSITY_SHADER );
	else ptDisable( PT_INTENSITY_SHADER );

	if ( frontBias ) ptEnable( PT_FRONT_BIAS );
	else ptDisable( PT_FRONT_BIAS ); 

	if ( adaptivePointSize ) ptEnable( PT_ADAPTIVE_POINT_SIZE );
	else ptDisable( PT_ADAPTIVE_POINT_SIZE ); 

	if (usePlane) ptEnable( PT_PLANE_SHADER );
	else ptDisable( PT_PLANE_SHADER );

	if (useLighting) ptEnable( PT_LIGHTING );
	else ptDisable( PT_LIGHTING );

	ptShaderOptionf( PT_INTENSITY_SHADER_BRIGHTNESS, brightness );
	ptShaderOptionf( PT_INTENSITY_SHADER_CONTRAST, contrast );

	ptShaderOptioni( PT_INTENSITY_SHADER_RAMP, intensityRamp-1 );
	ptShaderOptioni( PT_PLANE_SHADER_RAMP, planeRamp-1 );
	ptShaderOptionf( PT_PLANE_SHADER_DISTANCE, dist );
	ptShaderOptionf( PT_PLANE_SHADER_OFFSET, off );

	float axis [] = {0,0,0};
	axis[ planeAxis-1 ] = 1.0f;
	ptShaderOptionfv( PT_PLANE_SHADER_VECTOR, axis ); 

	ptDynamicFrameRate( fps );
}
//-----------------------------------------------------------------------------
void ShaderTool::handleClassificationShader()
//-----------------------------------------------------------------------------
{
	// classification uses query renderer
	if (m_shader.useClassification)
	{
		if (!m_classificationRenderer)
			m_classificationRenderer = new ClassificationRenderer((int) 1e5);

		Renderer* currentRenderer = VortexExampleApp::instance()->getRenderer();
		if (currentRenderer != m_classificationRenderer)
		{
			m_lastRenderer = currentRenderer;
			VortexExampleApp::instance()->setRenderer(m_classificationRenderer);
		}
	}
	else
	{
		if (VortexExampleApp::instance()->getRenderer()==m_classificationRenderer)
		{
			VortexExampleApp::instance()->setRenderer(m_lastRenderer);
		}
	}
}

//-----------------------------------------------------------------------------
void ShaderTool::buildUserInterface(GLUI_Node *parent)
//-----------------------------------------------------------------------------
{
	m_shader.initialize();

	//// shader Rollout
	GLUI_Rollout *rolloutShader = new GLUI_Rollout( parent, "Shader", false );
	rolloutShader->set_w( PANEL_WIDTH );

		GLUI_StaticText * spacer = new GLUI_StaticText( rolloutShader, "" );
		spacer->set_w( PANEL_WIDTH );
	
		GLUI_Panel *ch = new GLUI_Panel( rolloutShader, "Channels" );

		// Switches
		new GLUI_Checkbox( ch, "RGB", &m_shader.useRGB, CmdShaderUpdate, &Tool::dispatchCmd );
		new GLUI_Column ( ch, false );
		new GLUI_Checkbox( ch, "Intensity", &m_shader.useIntensity, CmdShaderUpdate, &Tool::dispatchCmd );
		new GLUI_Column ( ch, false );
		new GLUI_Checkbox( ch, "Lighting", &m_shader.useLighting, CmdShaderUpdate, &Tool::dispatchCmd );
		new GLUI_Column ( ch, false );
		new GLUI_Checkbox( ch, "Class", &m_shader.useClassification, CmdShaderUpdate, &Tool::dispatchCmd );

		// intensity 
		GLUI_Panel *inten = new GLUI_Panel( rolloutShader, "Intensity" );
		inten->set_w( PANEL_WIDTH );
		new GLUI_StaticText( inten, " ");
		new GLUI_StaticText( inten, "Constrast");
		new GLUI_StaticText( inten, "Brightness");

		GLUI_Column *col = new GLUI_Column( inten, false );
		col->set_w(1);

		GLUI_Scrollbar *sb;
		sb = new GLUI_Scrollbar( inten, "Contrast", GLUI_SCROLL_HORIZONTAL, &m_shader.contrast, CmdShaderUpdate ,&Tool::dispatchCmd);
		sb->set_float_limits(0,360);

		sb = new GLUI_Scrollbar( inten, "Brightness", GLUI_SCROLL_HORIZONTAL, &m_shader.brightness, CmdShaderUpdate ,&Tool::dispatchCmd);
		sb->set_float_limits(0,360);

		// ramps
		new GLUI_StaticText( inten, "" );
		
		// GLUI_Listbox causes a crash with the current x64 build that is linking to freeglut
#ifndef _M_X64
		GLUI_Listbox *rampList = new GLUI_Listbox( inten, "Ramp ", &m_shader.intensityRamp, CmdShaderUpdate, &Tool::dispatchCmd );

		int i;
		for (i=0; i< (int)m_shader.intensityRamps.size(); i++)
		{
			rampList->add_item( i+1, m_shader.intensityRamps[i].c_str() );
		}
#else
		GLUI_Button* but = new GLUI_Button( inten, "Next Ramp", CmdIncIntensity, &Tool::dispatchCmd );
#endif // _M_X64

	//// Options Roll out
	GLUI_Rollout *rolloutOptions = new GLUI_Rollout(parent, "Options", true );
	rolloutOptions->set_w( PANEL_WIDTH );

		spacer = new GLUI_StaticText( rolloutOptions, "" );
		spacer->set_w( PANEL_WIDTH);

		GLUI_Panel *dynamic = new GLUI_Panel( rolloutOptions, "Dynamic Rendering" );
		dynamic->set_w( PANEL_WIDTH );
		new GLUI_Checkbox( dynamic, "Adaptive point size", &m_shader.adaptivePointSize );
		new GLUI_Column( dynamic, false );
		new GLUI_Checkbox( dynamic, "Front bias", &m_shader.frontBias );

		new GLUI_StaticText( rolloutOptions, "    FPS");
		new GLUI_Column( dynamic, false );
		sb = new GLUI_Scrollbar( rolloutOptions, "Frame Rate", GLUI_SCROLL_HORIZONTAL, &m_shader.fps );

		sb->set_float_limits(1,30);
}


