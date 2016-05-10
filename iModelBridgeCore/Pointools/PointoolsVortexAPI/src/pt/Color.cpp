/****************************************************************************\
Datei  : Color.cpp
Projekt: Farbverwaltung
Inhalt : Color Implementierung
Datum  : 10.01.1999
Autor  : Christian Rodemeyer
Hinweis: © 1999 by Christian Rodemeyer
         Info über HLS Konvertierungsfunktion
         - Foley and Van Dam: "Fundamentals of Interactive Computer Graphics"
         - MSDN: 'HLS Color Spaces'
         - MSDN: 'Converting Colors Between RGB and HLS'
\****************************************************************************/
#include "PointoolsVortexAPIInternal.h"
#include <assert.h>
#include <pt/color.h>
#include <cstdio>
#include <string.h>
#ifdef __INTEL_COMPILER
#pragma warning ( disable : 1572 ) /* floating-point equality and inequality comparisons are unreliable */ 
#endif
using namespace pt;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/****************************************************************************\
 Color: Implementierung
\****************************************************************************/
const Color::DNamedColor Color::m_namedColor[Color::numNamedColors] =
{
  {Aliceblue            , "aliceblue"},
  {Antiquewhite         , "antiquewhite"},
  {Aqua                 , "aqua"},
  {Aquamarine           , "aquamarine"},
  {Azure                , "azure"},
  {Beige                , "beige"},
  {Bisque               , "bisque"},
  {Black                , "black"},
  {Blanchedalmond       , "blanchedalmond"},
  {Blue                 , "blue"},
  {Blueviolet           , "blueviolet"},
  {Brown                , "brown"},
  {Burlywood            , "burlywood"},
  {Cadetblue            , "cadetblue"},
  {Chartreuse           , "chartreuse"},
  {Chocolate            , "chocolate"},
  {Coral                , "coral"},
  {Cornflower           , "cornflower"},
  {Cornsilk             , "cornsilk"},
  {Crimson              , "crimson"},
  {Cyan                 , "cyan"},
  {Darkblue             , "darkblue"},
  {Darkcyan             , "darkcyan"},
  {Darkgoldenrod        , "darkgoldenrod"},
  {Darkgray             , "darkgray"},
  {Darkgreen            , "darkgreen"},
  {Darkkhaki            , "darkkhaki"},
  {Darkmagenta          , "darkmagenta"},
  {Darkolivegreen       , "darkolivegreen"},
  {Darkorange           , "darkorange"},
  {Darkorchid           , "darkorchid"},
  {Darkred              , "darkred"},
  {Darksalmon           , "darksalmon"},
  {Darkseagreen         , "darkseagreen"},
  {Darkslateblue        , "darkslateblue"},
  {Darkslategray        , "darkslategray"},
  {Darkturquoise        , "darkturquoise"},
  {Darkviolet           , "darkviolet"},
  {Deeppink             , "deeppink"},
  {Deepskyblue          , "deepskyblue"},
  {Dimgray              , "dimgray"},
  {Dodgerblue           , "dodgerblue"},
  {Firebrick            , "firebrick"},
  {Floralwhite          , "floralwhite"},
  {Forestgreen          , "forestgreen"},
  {Fuchsia              , "fuchsia"},
  {Gainsboro            , "gainsboro"},
  {Ghostwhite           , "ghostwhite"},
  {Gold                 , "gold"},
  {Goldenrod            , "goldenrod"},
  {Grey                 , "grey"},
  {Green                , "green"},
  {Greenyellow          , "greenyellow"},
  {Honeydew             , "honeydew"},
  {Hotpink              , "hotpink"},
  {Indianred            , "indianred"},
  {Indigo               , "indigo"},
  {Ivory                , "ivory"},
  {Khaki                , "khaki"},
  {Lavender             , "lavender"},
  {Lavenderblush        , "lavenderblush"},
  {Lawngreen            , "lawngreen"},
  {Lemonchiffon         , "lemonchiffon"},
  {Lightblue            , "lightblue"},
  {Lightcoral           , "lightcoral"},
  {Lightcyan            , "lightcyan"},
  {Lightgoldenrodyellow , "lightgoldenrodyellow"},
  {Lightgreen           , "lightgreen"},
  {Lightgrey            , "lightgrey"},
  {Lightpink            , "lightpink"},
  {Lightsalmon          , "lightsalmon"},
  {Lightseagreen        , "lightseagreen"},
  {Lightskyblue         , "lightskyblue"},
  {Lightslategray       , "lightslategray"},
  {Lightsteelblue       , "lightsteelblue"},
  {Lightyellow          , "lightyellow"},
  {Lime                 , "lime"},
  {Limegreen            , "limegreen"},
  {Linen                , "linen"},
  {Magenta              , "magenta"},
  {Maroon               , "maroon"},
  {Mediumaquamarine     , "mediumaquamarine"},
  {Mediumblue           , "mediumblue"},
  {Mediumorchid         , "mediumorchid"},
  {Mediumpurple         , "mediumpurple"},
  {Mediumseagreen       , "mediumseagreen"},
  {Mediumslateblue      , "mediumslateblue"},
  {Mediumspringgreen    , "mediumspringgreen"},
  {Mediumturquoise      , "mediumturquoise"},
  {Mediumvioletred      , "mediumvioletred"},
  {Midnightblue         , "midnightblue"},
  {Mintcream            , "mintcream"},
  {Mistyrose            , "mistyrose"},
  {Moccasin             , "moccasin"},
  {Navajowhite          , "navajowhite"},
  {Navy                 , "navy"},
  {Oldlace              , "oldlace"},
  {Olive                , "olive"},
  {Olivedrab            , "olivedrab"},
  {Orange               , "orange"},
  {Orangered            , "orangered"},
  {Orchid               , "orchid"},
  {Palegoldenrod        , "palegoldenrod"},
  {Palegreen            , "palegreen"},
  {Paleturquoise        , "paleturquoise"},
  {Palevioletred        , "palevioletred"},
  {Papayawhip           , "papayawhip"},
  {Peachpuff            , "peachpuff"},
  {Peru                 , "peru"},
  {Pink                 , "pink"},
  {Plum                 , "plum"},
  {Powderblue           , "powderblue"},
  {Purple               , "purple"},
  {Red                  , "red"},
  {Rosybrown            , "rosybrown"},
  {Royalblue            , "royalblue"},
  {Saddlebrown          , "saddlebrown"},
  {Sandybrown           , "sandybrown"},
  {Seagreen             , "seagreen"},
  {Seashell             , "seashell"},
  {Sienna               , "sienna"},
  {Silver               , "silver"},
  {Skyblue              , "skyblue"},
  {Slateblue            , "slateblue"},
  {Slategray            , "slategray"},
  {Snow                 , "snow"},
  {Springgreen          , "springgreen"},
  {Steelblue            , "steelblue"},
  {Tan                  , "tan"},
  {Teal                 , "teal"},
  {Thistle              , "thistle"},
  {Tomato               , "tomato"},
  {Turquoise            , "turquoise"},
  {Violet               , "violet"},
  {Wheat                , "wheat"},
  {White                , "white"},
  {Whitesmoke           , "whitesmoke"},
  {Yellow               , "yellow"},
  {Yellowgreen          , "yellowgreen"}
};

const char* Color::nameFromIndex(int i)
{
  assert(0 <= i && i < numNamedColors);
  return m_namedColor[i].name;
}

Color Color::colorFromIndex(int i)
{
  assert(0 <= i && i < numNamedColors);
  return m_namedColor[i].color;
}
Color Color::colorFromName(const char* name)
{
	for (int i=0; i<numNamedColors; i++)
	{
		if (m_namedColor[i].name && 
			strcmp(name, m_namedColor[i].name) == 0)
			return m_namedColor[i].color;
	}
	return Color();
}
Color Color::fromString(const char* pColor)
{
  Color t;
  t.string(pColor);
  return t;
}

Color::Color(COLORVAL cr)
: m_bIsRGB(true), m_bIsHLS(false), m_COLORVAL(cr)
{}

Color::operator COLORVAL() const
{
  const_cast<Color*>(this)->toRGB();
  return m_COLORVAL;
}

// RGB

void Color::red(int red)
{
  assert(0 <= red && red <= 255);
  toRGB();
  m_color[c_red] = static_cast<unsigned char>(red);
  m_bIsHLS = false;
}

void Color::green(int green)
{
  assert(0 <= green && green <= 255);
  toRGB();
  m_color[c_green] = static_cast<unsigned char>(green);
  m_bIsHLS = false;
}

void Color::blue(int blue)
{
  assert(0 <= blue && blue <= 255);
  toRGB();
  m_color[c_blue] = static_cast<unsigned char>(blue);
  m_bIsHLS = false;
}

void Color::rgb(int red, int green, int blue)
{
  assert(0 <= red && red <= 255);
  assert(0 <= green && green <= 255);
  assert(0 <= blue && blue <= 255);

  m_color[c_red]   = static_cast<unsigned char>(red);
  m_color[c_green] = static_cast<unsigned char>(green);
  m_color[c_blue]  = static_cast<unsigned char>(blue);
  m_bIsHLS = false;
  m_bIsRGB = true;
}

int Color::red() const
{
  const_cast<Color*>(this)->toRGB();
  return m_color[c_red];
}

int Color::green() const
{
  const_cast<Color*>(this)->toRGB();
  return m_color[c_green];
}

int Color::blue() const
{
  const_cast<Color*>(this)->toRGB();
  return m_color[c_blue];
}

// HSL

void Color::hue(float hue)
{
  assert(hue >= 0.0 && hue <= 360.0);

  toHLS();
  m_hue = hue;
  m_bIsRGB = false;
}

void Color::saturation(float saturation)
{
  assert(saturation >= 0.0 && saturation <= 1.0); // 0.0 ist undefiniert

  toHLS();
  m_saturation = saturation;
  m_bIsRGB = false;
}

void Color::luminance(float luminance)
{
  assert(luminance >= 0.0 && luminance <= 1.0);

  toHLS();
  m_luminance = luminance;
  m_bIsRGB = false;
}

void Color::hsl(float hue, float saturation, float luminance )
{
  assert(hue >= 0.0 && hue <= 360.0);
  assert(luminance >= 0.0 && luminance <= 1.0);
  assert(saturation >= 0.0 && saturation <= 1.0); // 0.0 ist undefiniert

  m_hue = hue;
  m_luminance = luminance;
  m_saturation = saturation;
  m_bIsRGB = false;
  m_bIsHLS = true;
}

void Color::hsv(float hue, float saturation, float value )
{
	float h = hue;
	float l = (2 - saturation) * value;
	float s = saturation * value;

	if (value < 0.999f)
	{
		s /= (l <= 1) ? l : 2 - l;
	}
	l /= 2;

	hsl( h,s,l );
}
float Color::hue() const
{
  const_cast<Color*>(this)->toHLS();
  return m_hue;
}

float Color::saturation() const
{
  const_cast<Color*>(this)->toHLS();
  return m_saturation;
}

float Color::luminance() const
{
  const_cast<Color*>(this)->toHLS();
  return m_luminance;
}

// Konvertierung

void Color::toHLS()
{
  if (!m_bIsHLS)
  {
    // Konvertierung
    unsigned char minval = min(m_color[c_red], min(m_color[c_green], m_color[c_blue]));
    unsigned char maxval = max(m_color[c_red], max(m_color[c_green], m_color[c_blue]));
    float mdiff  = float(maxval) - float(minval);
    float msum   = float(maxval) + float(minval);

    m_luminance = msum / 510.0f;

    if (maxval == minval)
    {
      m_saturation = 0.0f;
      m_hue = 0.0f;
    }
    else
    {
      float rnorm = (maxval - m_color[c_red]  ) / mdiff;
      float gnorm = (maxval - m_color[c_green]) / mdiff;
      float bnorm = (maxval - m_color[c_blue] ) / mdiff;

      m_saturation = (m_luminance <= 0.5f) ? (mdiff / msum) : (mdiff / (510.0f - msum));

      if (m_color[c_red]   == maxval) m_hue = 60.0f * (6.0f + bnorm - gnorm);
      if (m_color[c_green] == maxval) m_hue = 60.0f * (2.0f + rnorm - bnorm);
      if (m_color[c_blue]  == maxval) m_hue = 60.0f * (4.0f + gnorm - rnorm);
      if (m_hue > 360.0f) m_hue = m_hue - 360.0f;
    }
    m_bIsHLS = true;
  }
}

void Color::toRGB()
{
  if (!m_bIsRGB)
  {
    if (m_saturation == 0.0) // Grauton, einfacher Fall
    {
      m_color[c_red] = m_color[c_green] = m_color[c_blue] = (unsigned char)(m_luminance * 255.0);
    }
    else
    {
      float rm1, rm2;

      if (m_luminance <= 0.5f) rm2 = m_luminance + m_luminance * m_saturation;
      else                     rm2 = m_luminance + m_saturation - m_luminance * m_saturation;
      rm1 = 2.0f * m_luminance - rm2;
      m_color[c_red]   = toRGB1(rm1, rm2, m_hue + 120.0f);
      m_color[c_green] = toRGB1(rm1, rm2, m_hue);
      m_color[c_blue]  = toRGB1(rm1, rm2, m_hue - 120.0f);
    }
    m_bIsRGB = true;
  }
}

unsigned char Color::toRGB1(float rm1, float rm2, float rh)
{
  if      (rh > 360.0f) rh -= 360.0f;
  else if (rh <   0.0f) rh += 360.0f;

  if      (rh <  60.0f) rm1 = rm1 + (rm2 - rm1) * rh / 60.0f;
  else if (rh < 180.0f) rm1 = rm2;
  else if (rh < 240.0f) rm1 = rm1 + (rm2 - rm1) * (240.0f - rh) / 60.0f;

  return static_cast<unsigned char>(rm1 * 255);
}

// Stringkonvertierung im Format RRGGBB

String Color::string() const
{
  String color;
  color.format("%02X%02X%02X", red(), green(), blue());
  return color;
}

bool Color::string(const char* pColor)
{
  assert(pColor);
  int r, g, b;
  if (sscanf(pColor, "%2x%2x%2x", &r, &g, &b) != 3)
  {
    m_color[c_red] = m_color[c_green] = m_color[c_blue] = 0;
    return false;
  }
  else
  {
    m_color[c_red]   = static_cast<unsigned char>(r);
    m_color[c_green] = static_cast<unsigned char>(g);
    m_color[c_blue]  = static_cast<unsigned char>(b);
    m_bIsRGB = true;
    m_bIsHLS = false;
    return true;
  }
}

String Color::name() const
{
  const_cast<Color*>(this)->toRGB();
  int i = numNamedColors;
  while (i-- && m_COLORVAL != m_namedColor[i].color);
  if (i < 0)
  {
    return string();
  }
  else return pt::String(m_namedColor[i].name);
}
