#include "PointoolsVortexAPIInternal.h"
#include <ptengine/colourRamps.h>

using namespace pt;

typedef std::map < String, ColourGradient* > GradientsByName;

/*****************************************************************************/
/**
* @brief	Constructor, does nothing for now
*/
/*****************************************************************************/
ColourGradientManager::ColourGradientManager()
{
}

/*****************************************************************************/
/**
* @brief	Returns number of colour gradients
*/
/*****************************************************************************/
int ColourGradientManager::numColourGradients() const 
{ 
    return static_cast<int>(m_gradients.size());
}

/*****************************************************************************/
/**
* @brief		Returns gradient object by its name
* @param name	The name of the gradient object
* @return ColourGradient *	
*/
/*****************************************************************************/
ColourGradient *ColourGradientManager::getGradientByName( const pt::String &name )
{
	GradientsByName::iterator i = 
		m_gradientsByName.find( name );

	if (i == m_gradientsByName.end()) return 0;
	return i->second;
}
/*****************************************************************************/
/**
* @brief
* @param index
* @return const pt::String &
*/
/*****************************************************************************/
const pt::String &ColourGradientManager::gradientName( int index ) const
{
	static pt::String none("");
	return index < (int)m_gradients.size() ? m_gradients[index]->m_name : none;	
}
	
/*****************************************************************************/
/**
* @brief
* @param index
* @return ColourGradient *
*/
/*****************************************************************************/
ColourGradient *ColourGradientManager::getGradientByIndex( int index )
{
	return index < (int) m_gradients.size() ? m_gradients[index] : 0;
}

/*****************************************************************************/
/**
* @brief
* @param name
* @return const ColourGradient *
*/
/*****************************************************************************/
const ColourGradient *ColourGradientManager::getGradientByName( const pt::String &name ) const
{
	GradientsByName::const_iterator i = m_gradientsByName.find( name );

	if (i == m_gradientsByName.end()) return 0;
	return i->second;
}
	
/*****************************************************************************/
/**
* @brief
* @param index
* @return const ColourGradient *
*/
/*****************************************************************************/
const ColourGradient *ColourGradientManager::getGradientByIndex( int index ) const
{
	return index < (int) m_gradients.size() ? m_gradients[index] : 0;
}


/*****************************************************************************/
/**
* @brief			Adds a gradient object to the manager
* @param gradient	Pointer to the gradient. Lifetime will be managed by the gradient manager. *DO NOT DELETE*
*/
/*****************************************************************************/
void ColourGradientManager::addGradient( ColourGradient *gradient )
{
	m_gradientsByName.insert( GradientsByName::value_type( gradient->m_name, gradient ));
	m_gradients.push_back( gradient );
}

/*****************************************************************************/
/**
* @brief			Create the default set of gradients. Does not happen on construction
					This must be called to setup the default gradients.
* @return void
*/
/*****************************************************************************/
int ColourGradientManager::createDefaultGradients()
{
	int size = this->numColourGradients();

	// hue ramp
	ColourGradient *gradient = new ColourGradient("Hue");
	
	Color col;
	col.hsl(360, 1.0f, 0.5f);

	gradient->m_gradient.addKey( 1.0, col, GradColHSL );
	
	col.hsl(0, 1.0f, 0.5f);
	gradient->m_gradient.addKey( 0, col, GradColHSL );

	addGradient( gradient );

	// soft hue ramp	-----------------------------------
	gradient = new ColourGradient("Soft Hue");
	
	col.hsl(360, 0.5f, 0.5f);
	gradient->m_gradient.addKey( 1.0, col, GradColHSL );
	
	col.hsl(0, 0.5f, 0.5f);
	gradient->m_gradient.addKey( 0, col, GradColHSL );

	addGradient( gradient );

	// greyscale ramp	-----------------------------------
	gradient = new ColourGradient("Greyscale");
	col.rgb(0, 0, 0);

	gradient->m_gradient.addKey( 0.0, col, GradColRGB );
	
	col.rgb(255,255,255);
	gradient->m_gradient.addKey( 1.0, col, GradColRGB );

	addGradient( gradient );


	// topo ramp	--------------------------------------
	gradient = new ColourGradient("Land Topo Smooth");
	
	col.hsv(199, 0.148f, 1.0f);	
	gradient->m_gradient.addKey( 0, col, GradColHSL );
	
	col.hsv(111, 0.207f, 0.741f);	
	gradient->m_gradient.addKey( 25, col, GradColHSL );

	col.hsv(109.0f, 0.273f, 0.682f);	
	gradient->m_gradient.addKey( 200, col, GradColHSL );

	col.hsv(92, 0.277f, 0.706f);	
	gradient->m_gradient.addKey( 400, col, GradColHSL );

	col.hsv(77, 0.270f, 0.729f);	
	gradient->m_gradient.addKey( 600, col, GradColHSL );

	col.hsv(63, 0.204f, 0.812f);	
	gradient->m_gradient.addKey( 800, col, GradColHSL );

	col.hsv(50, 0.255f, 0.753f);	
	gradient->m_gradient.addKey( 1000, col, GradColHSL );

	col.hsv(45, 0.357f, 0.722f);	
	gradient->m_gradient.addKey( 1200, col, GradColHSL );

	col.hsv(41, 0.451f, 0.694f);	
	gradient->m_gradient.addKey( 1400, col, GradColHSL );

	col.hsv(39, 0.514f, 0.659f);	
	gradient->m_gradient.addKey( 1600, col, GradColHSL );

	col.hsv(36, 0.514f, 0.608f);	
	gradient->m_gradient.addKey( 1800, col, GradColHSL );

	gradient->m_gradient.normalise();
	gradient->m_gradient.setSize( 1024 );
	addGradient( gradient );

	// topo ramp	--------------------------------------
	gradient = new ColourGradient("Land Topo Stepped");
	
	col.hsv(199, 0.148f, 1.0f);	
	gradient->m_gradient.addKey( 0, col, GradColHSL );
	
	col.hsv(111, 0.207f, 0.741f);	
	gradient->m_gradient.addKey( 25, col, GradColHSL );

	col.hsv(109, 0.273f, 0.682f);	
	gradient->m_gradient.addKey( 200, col, GradColHSL );

	col.hsv(92, 0.277f, 0.706f);	
	gradient->m_gradient.addKey( 400, col, GradColHSL );

	col.hsv(77, 0.270f, 0.729f);	
	gradient->m_gradient.addKey( 600, col, GradColHSL );

	col.hsv(63, 0.204f, 0.812f);	
	gradient->m_gradient.addKey( 800, col, GradColHSL );

	col.hsv(50, 0.255f, 0.753f);	
	gradient->m_gradient.addKey( 1000, col, GradColHSL );

	col.hsv(45, 0.357f, 0.722f);	
	gradient->m_gradient.addKey( 1200, col, GradColHSL );

	col.hsv(41, 0.451f, 0.694f);	
	gradient->m_gradient.addKey( 1400, col, GradColHSL );

	col.hsv(39, 0.514f, 0.659f);	
	gradient->m_gradient.addKey( 1600, col, GradColHSL );

	col.hsv(36, 0.514f, 0.608f);	
	gradient->m_gradient.addKey( 1800, col, GradColHSL );

	gradient->m_gradient.normalise();
	gradient->m_gradient.setSize( 1024 );
	gradient->m_gradient.setInterpolation( GradInterpStep );
	gradient->m_use = ColourGradientPlane;

	addGradient( gradient );

	// topo ramp	--------------------------------------
	gradient = new ColourGradient("Sea-Mountain Topo");
	
	col.hsv(185, 1.0f, 0.522f);	//0
	gradient->m_gradient.addKey( 0, col, GradColHSL );
	
	col.hsv(164, 0.957f, 0.647f);	// 50
	gradient->m_gradient.addKey( 25, col, GradColHSL );

	col.hsv(112, 0.745f, 0.756f);	//100
	gradient->m_gradient.addKey( 50, col, GradColHSL );

	col.hsv(90, 0.839f, 0.820f);	//250
	gradient->m_gradient.addKey( 250, col, GradColHSL );

	col.hsv(59, 0.545f, 0.933f);	//500
	gradient->m_gradient.addKey( 500, col, GradColHSL );

	col.hsv(17, 0.561f, 0.756f);	//700
	gradient->m_gradient.addKey( 750, col, GradColHSL );

	col.hsv(260, 0.161f, 0.690f);	//900
	gradient->m_gradient.addKey( 1000, col, GradColHSL );

	col.hsv(0, 0, 1.0f);	//1100
	gradient->m_gradient.addKey( 1250, col, GradColHSL );

	gradient->m_gradient.normalise();
	gradient->m_gradient.setSize( 1024 );
	gradient->m_use = ColourGradientPlane;

	addGradient( gradient );


	// red / blue ramp	-----------------------------------
	gradient = new ColourGradient("Blue / Red");
	
	col.rgb(0,0,255); // blue
	gradient->m_gradient.addKey( 0, col, GradColRGB );
	
	col.rgb(255,0,0);	//red
	gradient->m_gradient.addKey( 1.0f, col, GradColRGB );

	addGradient( gradient );


	// flat ramp		-----------------------------------
	gradient = new ColourGradient("Striped");
	
	col.rgb(153,153,255); // blue 
	gradient->m_gradient.addKey( 0, col, GradColRGB );
	
	col.rgb(255,255,255);	//white
	gradient->m_gradient.addKey( 0.5f, col, GradColRGB );

	gradient->m_gradient.setInterpolation( GradInterpStep );
	gradient->m_use = ColourGradientPlane;

	addGradient( gradient );

	return (numColourGradients() - size);
}