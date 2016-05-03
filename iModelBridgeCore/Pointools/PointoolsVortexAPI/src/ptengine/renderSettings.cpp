#include "PointoolsVortexAPIInternal.h"

#include <ptengine/renderSettings.h>

using namespace pointsengine;

ubyte RenderSettings::m_selectionColour [] = {255,0,0};		// selection colour is same across contexts

RenderSettings::RenderSettings()
{
	setToDefault();
}
void RenderSettings::setToDefault()
{
	m_pointSize = 1.0f;
	m_dynMinOutput = 0.01f;
	m_minStaticOutput = 0.01f;

	m_overallDensity = 1.0f;
	m_dynFrontBias = false;

	m_intensityEnabled = false;
	m_rgbEnabled = true; 

	m_intensityBlack = 0.1388f;
	m_intensityWhite = 0.5f;
	m_intensityRampEdge = ClampEdge;
	m_intensityGradient = 2;

	m_geomShader = false;
	m_geomShaderShowScale = false;
	m_geomShaderGradient = 1;
	m_geomShaderEdge = RepeatEdge;

	m_lightEnabled = false;
	
	m_autoPointSize = true;
	m_allowInterrupt = false;
	m_cancelRender = false;

	m_userChannelRender = false;

	m_clippingEnabled = false;

	m_fps = 22;
	
}

void RenderSettings::selectionColour( const ubyte *col3bytes )
{
	m_selectionColour[0] = col3bytes[0];
	m_selectionColour[1] = col3bytes[1];
	m_selectionColour[2] = col3bytes[2];
	//m_selectionColour[3] = 255;//col4bytes[3];
}
const ubyte	* RenderSettings::selectionColour()
{
	return m_selectionColour;
}
const double	* RenderSettings::geomShaderParams() const
{
	return m_geomShaderParams;
}

void RenderSettings::geomShaderParams( int numValues, const double *val4 )
{
	for (int i=0; i<numValues; i++)
		m_geomShaderParams[i] = val4[i];
}

void RenderSettings::materialSpecular( const float *spec3 )
{
	m_lightSpecular[0] = spec3[0];
	m_lightSpecular[1] = spec3[1];
	m_lightSpecular[2] = spec3[2];
}

void RenderSettings::lightAmbient( const float *amb3 )
{
	m_lightAmbient[0] = amb3[0];
	m_lightAmbient[1] = amb3[1];
	m_lightAmbient[2] = amb3[2];
}

void RenderSettings::materialDiffuse( const float *diff3 )
{
	m_lightDiffuse[0] = diff3[0];
	m_lightDiffuse[1] = diff3[1];
	m_lightDiffuse[2] = diff3[2];
}

const			RenderSettings		& RenderSettings::operator=( const RenderSettings &settings )
{
	memcpy(this, &settings, sizeof(RenderSettings));
	return *this;
}

