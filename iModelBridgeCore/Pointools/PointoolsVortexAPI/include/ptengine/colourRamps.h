#pragma once

#include <pt/typedefs.h>
#include <pt/ptstring.h>
#include <ptcloud2/gradient.h>
#include <vector>
#include <map>

enum ColourRampUse
{
	ColourGradientIntensity = 0x01,
	ColourGradientPlane		= 0x02
};

struct ColourGradient
{
	ColourGradient( const char *name ) 
		: m_use( ColourGradientIntensity |  ColourGradientPlane ),
		m_name( name )
	{
		memset( m_texID, 0, sizeof(int) * 16);
	}

	uint				m_use;
	pt::String			m_name;
	pt::Gradient		m_gradient;
	int					m_texID[16];
};

class ColourGradientManager
{
public:
	ColourGradientManager();

	int						numColourGradients() const;
	const pt::String		&gradientName( int index ) const;

	ColourGradient			*getGradientByName( const pt::String &name );
	ColourGradient			*getGradientByIndex( int index );

	const ColourGradient	*getGradientByName( const pt::String &name ) const;
	const ColourGradient	*getGradientByIndex( int index ) const;

	void					addGradient( ColourGradient *gradient );

	int						createDefaultGradients();

private:
	std::vector < ColourGradient* >				m_gradients;
	std::map < pt::String, ColourGradient* >	m_gradientsByName;
};