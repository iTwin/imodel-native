/****************************************************************************\
base on CColor by Christian Rodemeyer
\****************************************************************************/
#include <pt/ptstring.h>
#include <pt/classes.h>

#ifndef __COLOR_H
#define __COLOR_H

#define COLORVAL unsigned int

namespace pt
{
class CCLASSES_API Color
{
public:

  Color(COLORVAL cr = Black);
  operator COLORVAL() const;

  void red(int red);     // 0..255
  void green(int green); // 0..255
  void blue(int blue);   // 0..255
  void rgb(int red, int green, int blue);

  int red() const;   // 0..255
  int green() const; // 0..255
  int blue() const;  // 0..255

  void hue(float hue);               // 0.0 .. 360.0
  void luminance(float luminance);   // 0.0 .. 1.0
  void saturation(float saturation); // 0.0 .. 1.0
  void hsl(float hue, float saturation, float luminance);
  void hsv(float hue, float lightness, float value);

  float hue() const;        // 0.0 .. 360.0
  float luminance() const;  // 0.0 .. 1.0
  float saturation() const; // 0.0 .. 1.0

  String	string() const;					// Liefert String im Format RRGGBB
  bool		string(const char *str);		// Erwartet String im Format RRGGBB

  String	name() const;

  static Color	fromString(const char* pcColor);
  static const char* nameFromIndex(int i);
  static Color	colorFromIndex(int i);
  static Color  colorFromName(const char* name);
  static int    numNames() {return numNamedColors;}

  enum ENamedColor // Named Colors, could be used as COLORVAL
  {
    None                 = 0x7FFFFFFF,   // keine Farbe
    Aliceblue            = 0x00FFF8F0,   // RGB(0xF0, 0xF8, 0xFF)
    Antiquewhite         = 0x00D7EBFA,   // RGB(0xFA, 0xEB, 0xD7)
    Aqua                 = 0x00FFFF00,   // RGB(0x00, 0xFF, 0xFF)
    Aquamarine           = 0x00D4FF7F,   // RGB(0x7F, 0xFF, 0xD4)
    Azure                = 0x00FFFFF0,   // RGB(0xF0, 0xFF, 0xFF)
    Beige                = 0x00DCF5F5,   // RGB(0xF5, 0xF5, 0xDC)
    Bisque               = 0x00C4E4FF,   // RGB(0xFF, 0xE4, 0xC4)
    Black                = 0x00000000,   // RGB(0x00, 0x00, 0x00)
    Blanchedalmond       = 0x00CDEBFF,   // RGB(0xFF, 0xEB, 0xCD)
    Blue                 = 0x00FF0000,   // RGB(0x00, 0x00, 0xFF)
    Blueviolet           = 0x00E22B8A,   // RGB(0x8A, 0x2B, 0xE2)
    Brown                = 0x002A2AA5,   // RGB(0xA5, 0x2A, 0x2A)
    Burlywood            = 0x0087B8DE,   // RGB(0xDE, 0xB8, 0x87)
    Cadetblue            = 0x00A09E5F,   // RGB(0x5F, 0x9E, 0xA0)
    Chartreuse           = 0x0000FF7F,   // RGB(0x7F, 0xFF, 0x00)
    Chocolate            = 0x001E69D2,   // RGB(0xD2, 0x69, 0x1E)
    Coral                = 0x00507FFF,   // RGB(0xFF, 0x7F, 0x50)
    Cornflower           = 0x00ED9564,   // RGB(0x64, 0x95, 0xED)
    Cornsilk             = 0x00DCF8FF,   // RGB(0xFF, 0xF8, 0xDC)
    Crimson              = 0x003C14DC,   // RGB(0xDC, 0x14, 0x3C)
    Cyan                 = 0x00FFFF00,   // RGB(0x00, 0xFF, 0xFF)
    Darkblue             = 0x008B0000,   // RGB(0x00, 0x00, 0x8B)
    Darkcyan             = 0x008B8B00,   // RGB(0x00, 0x8B, 0x8B)
    Darkgoldenrod        = 0x000B86B8,   // RGB(0xB8, 0x86, 0x0B)
    Darkgray             = 0x00A9A9A9,   // RGB(0xA9, 0xA9, 0xA9)
    Darkgreen            = 0x00006400,   // RGB(0x00, 0x64, 0x00)
    Darkkhaki            = 0x006BB7BD,   // RGB(0xBD, 0xB7, 0x6B)
    Darkmagenta          = 0x008B008B,   // RGB(0x8B, 0x00, 0x8B)
    Darkolivegreen       = 0x002F6B55,   // RGB(0x55, 0x6B, 0x2F)
    Darkorange           = 0x00008CFF,   // RGB(0xFF, 0x8C, 0x00)
    Darkorchid           = 0x00CC3299,   // RGB(0x99, 0x32, 0xCC)
    Darkred              = 0x0000008B,   // RGB(0x8B, 0x00, 0x00)
    Darksalmon           = 0x007A96E9,   // RGB(0xE9, 0x96, 0x7A)
    Darkseagreen         = 0x008BBC8F,   // RGB(0x8F, 0xBC, 0x8B)
    Darkslateblue        = 0x008B3D48,   // RGB(0x48, 0x3D, 0x8B)
    Darkslategray        = 0x004F4F2F,   // RGB(0x2F, 0x4F, 0x4F)
    Darkturquoise        = 0x00D1CE00,   // RGB(0x00, 0xCE, 0xD1)
    Darkviolet           = 0x00D30094,   // RGB(0x94, 0x00, 0xD3)
    Deeppink             = 0x009314FF,   // RGB(0xFF, 0x14, 0x93)
    Deepskyblue          = 0x00FFBF00,   // RGB(0x00, 0xBF, 0xFF)
    Dimgray              = 0x00696969,   // RGB(0x69, 0x69, 0x69)
    Dodgerblue           = 0x00FF901E,   // RGB(0x1E, 0x90, 0xFF)
    Firebrick            = 0x002222B2,   // RGB(0xB2, 0x22, 0x22)
    Floralwhite          = 0x00F0FAFF,   // RGB(0xFF, 0xFA, 0xF0)
    Forestgreen          = 0x00228B22,   // RGB(0x22, 0x8B, 0x22)
    Fuchsia              = 0x00FF00FF,   // RGB(0xFF, 0x00, 0xFF)
    Gainsboro            = 0x00DCDCDC,   // RGB(0xDC, 0xDC, 0xDC)
    Ghostwhite           = 0x00FFF8F8,   // RGB(0xF8, 0xF8, 0xFF)
    Gold                 = 0x0000D7FF,   // RGB(0xFF, 0xD7, 0x00)
    Goldenrod            = 0x0020A5DA,   // RGB(0xDA, 0xA5, 0x20)
    Grey                 = 0x00808080,   // RGB(0x80, 0x80, 0x80)
    Green                = 0x00008000,   // RGB(0x00, 0x80, 0x00)
    Greenyellow          = 0x002FFFAD,   // RGB(0xAD, 0xFF, 0x2F)
    Honeydew             = 0x00F0FFF0,   // RGB(0xF0, 0xFF, 0xF0)
    Hotpink              = 0x00B469FF,   // RGB(0xFF, 0x69, 0xB4)
    Indianred            = 0x005C5CCD,   // RGB(0xCD, 0x5C, 0x5C)
    Indigo               = 0x0082004B,   // RGB(0x4B, 0x00, 0x82)
    Ivory                = 0x00F0FFFF,   // RGB(0xFF, 0xFF, 0xF0)
    Khaki                = 0x008CE6F0,   // RGB(0xF0, 0xE6, 0x8C)
    Lavender             = 0x00FAE6E6,   // RGB(0xE6, 0xE6, 0xFA)
    Lavenderblush        = 0x00F5F0FF,   // RGB(0xFF, 0xF0, 0xF5)
    Lawngreen            = 0x0000FC7C,   // RGB(0x7C, 0xFC, 0x00)
    Lemonchiffon         = 0x00CDFAFF,   // RGB(0xFF, 0xFA, 0xCD)
    Lightblue            = 0x00E6D8AD,   // RGB(0xAD, 0xD8, 0xE6)
    Lightcoral           = 0x008080F0,   // RGB(0xF0, 0x80, 0x80)
    Lightcyan            = 0x00FFFFE0,   // RGB(0xE0, 0xFF, 0xFF)
    Lightgoldenrodyellow = 0x00D2FAFA,   // RGB(0xFA, 0xFA, 0xD2)
    Lightgreen           = 0x0090EE90,   // RGB(0x90, 0xEE, 0x90)
    Lightgrey            = 0x00D3D3D3,   // RGB(0xD3, 0xD3, 0xD3)
    Lightpink            = 0x00C1B6FF,   // RGB(0xFF, 0xB6, 0xC1)
    Lightsalmon          = 0x007AA0FF,   // RGB(0xFF, 0xA0, 0x7A)
    Lightseagreen        = 0x00AAB220,   // RGB(0x20, 0xB2, 0xAA)
    Lightskyblue         = 0x00FACE87,   // RGB(0x87, 0xCE, 0xFA)
    Lightslategray       = 0x00998877,   // RGB(0x77, 0x88, 0x99)
    Lightsteelblue       = 0x00DEC4B0,   // RGB(0xB0, 0xC4, 0xDE)
    Lightyellow          = 0x00E0FFFF,   // RGB(0xFF, 0xFF, 0xE0)
    Lime                 = 0x0000FF00,   // RGB(0x00, 0xFF, 0x00)
    Limegreen            = 0x0032CD32,   // RGB(0x32, 0xCD, 0x32)
    Linen                = 0x00E6F0FA,   // RGB(0xFA, 0xF0, 0xE6)
    Magenta              = 0x00FF00FF,   // RGB(0xFF, 0x00, 0xFF)
    Maroon               = 0x00000080,   // RGB(0x80, 0x00, 0x00)
    Mediumaquamarine     = 0x00AACD66,   // RGB(0x66, 0xCD, 0xAA)
    Mediumblue           = 0x00CD0000,   // RGB(0x00, 0x00, 0xCD)
    Mediumorchid         = 0x00D355BA,   // RGB(0xBA, 0x55, 0xD3)
    Mediumpurple         = 0x00DB7093,   // RGB(0x93, 0x70, 0xDB)
    Mediumseagreen       = 0x0071B33C,   // RGB(0x3C, 0xB3, 0x71)
    Mediumslateblue      = 0x00EE687B,   // RGB(0x7B, 0x68, 0xEE)
    Mediumspringgreen    = 0x009AFA00,   // RGB(0x00, 0xFA, 0x9A)
    Mediumturquoise      = 0x00CCD148,   // RGB(0x48, 0xD1, 0xCC)
    Mediumvioletred      = 0x008515C7,   // RGB(0xC7, 0x15, 0x85)
    Midnightblue         = 0x00701919,   // RGB(0x19, 0x19, 0x70)
    Mintcream            = 0x00FAFFF5,   // RGB(0xF5, 0xFF, 0xFA)
    Mistyrose            = 0x00E1E4FF,   // RGB(0xFF, 0xE4, 0xE1)
    Moccasin             = 0x00B5E4FF,   // RGB(0xFF, 0xE4, 0xB5)
    Navajowhite          = 0x00ADDEFF,   // RGB(0xFF, 0xDE, 0xAD)
    Navy                 = 0x00800000,   // RGB(0x00, 0x00, 0x80)
    Oldlace              = 0x00E6F5FD,   // RGB(0xFD, 0xF5, 0xE6)
    Olive                = 0x00008080,   // RGB(0x80, 0x80, 0x00)
    Olivedrab            = 0x00238E6B,   // RGB(0x6B, 0x8E, 0x23)
    Orange               = 0x0000A5FF,   // RGB(0xFF, 0xA5, 0x00)
    Orangered            = 0x000045FF,   // RGB(0xFF, 0x45, 0x00)
    Orchid               = 0x00D670DA,   // RGB(0xDA, 0x70, 0xD6)
    Palegoldenrod        = 0x00AAE8EE,   // RGB(0xEE, 0xE8, 0xAA)
    Palegreen            = 0x0098FB98,   // RGB(0x98, 0xFB, 0x98)
    Paleturquoise        = 0x00EEEEAF,   // RGB(0xAF, 0xEE, 0xEE)
    Palevioletred        = 0x009370DB,   // RGB(0xDB, 0x70, 0x93)
    Papayawhip           = 0x00D5EFFF,   // RGB(0xFF, 0xEF, 0xD5)
    Peachpuff            = 0x00B9DAFF,   // RGB(0xFF, 0xDA, 0xB9)
    Peru                 = 0x003F85CD,   // RGB(0xCD, 0x85, 0x3F)
    Pink                 = 0x00CBC0FF,   // RGB(0xFF, 0xC0, 0xCB)
    Plum                 = 0x00DDA0DD,   // RGB(0xDD, 0xA0, 0xDD)
    Powderblue           = 0x00E6E0B0,   // RGB(0xB0, 0xE0, 0xE6)
    Purple               = 0x00800080,   // RGB(0x80, 0x00, 0x80)
    Red                  = 0x000000FF,   // RGB(0xFF, 0x00, 0x00)
    Rosybrown            = 0x008F8FBC,   // RGB(0xBC, 0x8F, 0x8F)
    Royalblue            = 0x00E16941,   // RGB(0x41, 0x69, 0xE1)
    Saddlebrown          = 0x0013458B,   // RGB(0x8B, 0x45, 0x13)
    almon               = 0x007280FA,   // RGB(0xFA, 0x80, 0x72)
    Sandybrown           = 0x0060A4F4,   // RGB(0xF4, 0xA4, 0x60)
    Seagreen             = 0x00578B2E,   // RGB(0x2E, 0x8B, 0x57)
    Seashell             = 0x00EEF5FF,   // RGB(0xFF, 0xF5, 0xEE)
    Sienna               = 0x002D52A0,   // RGB(0xA0, 0x52, 0x2D)
    Silver               = 0x00C0C0C0,   // RGB(0xC0, 0xC0, 0xC0)
    Skyblue              = 0x00EBCE87,   // RGB(0x87, 0xCE, 0xEB)
    Slateblue            = 0x00CD5A6A,   // RGB(0x6A, 0x5A, 0xCD)
    Slategray            = 0x00908070,   // RGB(0x70, 0x80, 0x90)
    Snow                 = 0x00FAFAFF,   // RGB(0xFF, 0xFA, 0xFA)
    Springgreen          = 0x007FFF00,   // RGB(0x00, 0xFF, 0x7F)
    Steelblue            = 0x00B48246,   // RGB(0x46, 0x82, 0xB4)
    Tan                  = 0x008CB4D2,   // RGB(0xD2, 0xB4, 0x8C)
    Teal                 = 0x00808000,   // RGB(0x00, 0x80, 0x80)
    Thistle              = 0x00D8BFD8,   // RGB(0xD8, 0xBF, 0xD8)
    Tomato               = 0x004763FF,   // RGB(0xFF, 0x63, 0x47)
    Turquoise            = 0x00D0E040,   // RGB(0x40, 0xE0, 0xD0)
    Violet               = 0x00EE82EE,   // RGB(0xEE, 0x82, 0xEE)
    Wheat                = 0x00B3DEF5,   // RGB(0xF5, 0xDE, 0xB3)
    White                = 0x00FFFFFF,   // RGB(0xFF, 0xFF, 0xFF)
    Whitesmoke           = 0x00F5F5F5,   // RGB(0xF5, 0xF5, 0xF5)
    Yellow               = 0x0000FFFF,   // RGB(0xFF, 0xFF, 0x00)
    Yellowgreen          = 0x0032CD9A,   // RGB(0x9A, 0xCD, 0x32)
  };

  enum ENamedColorIndex
  {
    i_aliceblue, i_antiquewhite, i_aqua, i_aquamarine, i_azure, i_beige, i_bisque, i_black,
    i_blanchedalmond, i_blue, i_blueviolet, i_brown, i_burlywood, i_cadetblue, i_chartreuse,
    i_chocolate, i_coral, i_cornflower, i_cornsilk, i_crimson, i_cyan, i_darkblue, i_darkcyan,
    i_darkgoldenrod, i_darkgray, i_darkgreen, i_darkkhaki, i_darkmagenta, i_darkolivegreen,
    i_darkorange, i_darkorchid, i_darkred, i_darksalmon, i_darkseagreen, i_darkslateblue,
    i_darkslategray, i_darkturquoise, i_darkviolet, i_deeppink, i_deepskyblue, i_dimgray,
    i_dodgerblue, i_firebrick, i_floralwhite, i_forestgreen, i_fuchsia, i_gainsboro,
    i_ghostwhite, i_gold, i_goldenrod, i_gray, i_green, i_greenyellow, i_honeydew, i_hotpink,
    i_indianred, i_indigo, i_ivory, i_khaki, i_lavender, i_lavenderblush, i_lawngreen,
    i_lemonchiffon, i_lightblue, i_lightcoral, i_lightcyan, i_lightgoldenrodyellow,
    i_lightgreen, i_lightgrey, i_lightpink, i_lightsalmon, i_lightseagreen, i_lightskyblue,
    i_lightslategray, i_lightsteelblue, i_lightyellow, i_lime, i_limegreen, i_linen,
    i_magenta, i_maroon, i_mediumaquamarine, i_mediumblue, i_mediumorchid, i_mediumpurple,
    i_mediumseagreen, i_mediumslateblue, i_mediumspringgreen, i_mediumturquoise,
    i_mediumvioletred, i_midnightblue, i_mintcream, i_mistyrose, i_moccasin, i_navajowhite,
    i_navy, i_oldlace, i_olive, i_olivedrab, i_orange, i_orangered, i_orchid, i_palegoldenrod,
    i_palegreen, i_paleturquoise, i_palevioletred, i_papayawhip, i_peachpuff, i_peru, i_pink,
    i_plum, i_powderblue, i_purple, i_red, i_rosybrown, i_royalblue, i_saddlebrown, i_salmon,
    i_sandybrown, i_seagreen, i_seashell, i_sienna, i_silver, i_skyblue, i_slateblue,
    i_slategray, i_snow, i_springgreen, i_steelblue, i_tan, i_teal, i_thistle, i_tomato,
    i_turquoise, i_violet, i_wheat, i_white, i_whitesmoke, i_yellow, i_yellowgreen,
    numNamedColors
  };

private:

  // Konvertierung
  // -------------
  void toRGB(); // logisch konstant, nicht physikalisch
  void toHLS(); // logisch konstant, nicht physikalisch
  static unsigned char toRGB1(float rm1, float rm2, float rh);

  // Daten
  // -----
  union // Byteweiser Zugriff auf die COLORVAL Struktur
  {
    COLORVAL      m_COLORVAL;
    unsigned char m_color[4];
  };
  enum {c_red = 0, c_green = 1, c_blue = 2, c_null = 3}; // enum Hack für colorbyte-Index

  float m_hue;         // 0.0 .. 360.0  // Winkel
  float m_saturation;  // 0.0 .. 1.0    // Prozent
  float m_luminance;   // 0.0 .. 1.0    // Prozent

  // Flag für Lazy Evaluation
  bool m_bIsRGB;
  bool m_bIsHLS;

  // statische Konstanten für benannte Farben
  struct DNamedColor
  {
    COLORVAL color;
    const char*  name;
  };
  static const DNamedColor m_namedColor[numNamedColors];
};
}
#endif

