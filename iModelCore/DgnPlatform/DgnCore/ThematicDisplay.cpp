/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/ThematicDisplay.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/ThematicDisplay.h>
#include <DgnPlatform/ElementTileTree.h>


static double                   s_margin = .25, s_colorRange = 1.0 - 2. * s_margin;
static ColorDef const           s_defaultMarginColor = ColorDef(0x003f3f3f);

typedef bvector <int32_t>       IndexArray;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicColorSchemeProvider::ThematicColorSchemeProvider (ThematicColorScheme icse)   { m_icse = icse;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicColorSchemeProvider::KeysToArrays (size_t nKeys, const ThematicGradientKey* keys, double* values, RgbFactor* colors)
    {
    for(size_t i=0; i<nKeys;i++)
        {
        values[i]       = keys[i].value;
        colors[i].red   = static_cast <double> (keys[i].red)   / 255.0;
        colors[i].green = static_cast <double> (keys[i].green) / 255.0;
        colors[i].blue  = static_cast <double> (keys[i].blue)  / 255.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicColorSchemeProvider::ArraysToKeys (size_t nKeys, const double* values, const RgbFactor* colors, ThematicGradientKey* keys)
    {
    for(size_t i=0; i<nKeys;i++)
        {
        keys[i].value  = values[i];
        keys[i].red    = static_cast <uint8_t> (colors[i].red   * 255.0 + 0.5);
        keys[i].green  = static_cast <uint8_t> (colors[i].green * 255.0 + 0.5);
        keys[i].blue   = static_cast <uint8_t> (colors[i].blue  * 255.0 + 0.5);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicColorSchemeProvider::GradientKeys (size_t maxKeys, size_t* fetchedKeys, ThematicGradientKey* outKeys)
    {
    static const size_t                 s_nKeys[]   = { 5, 5, 2, 3, 6 };
    //Note: These colors are in RBG format to match the key structure (not RGB like most colors)
    static const ThematicGradientKey    s_keys[ThematicColorScheme_Max][GradientSymb::MAX_GRADIENT_KEYS] = 
        {  
           { {0.0, 0,   255, 0}, {0.25, 0,   255, 255}, {0.5, 0, 0, 255}, {0.75, 255, 0,   255}, {1.0, 255, 0,   0}},
           { {0.0, 255, 0,   0}, {0.25, 255, 0,   255}, {0.5, 0, 0, 255}, {0.75, 0,   255, 255}, {1.0, 0,   255, 0}},
           { {0.0, 0,   0,   0}, {1.0,  255, 255, 255}},

           //Based off of the topographic gradients in Point Clouds
           { {0.0, 152, 148, 188}, {0.5, 204, 160, 204}, {1.0, 152, 72, 128}},

           //Based off of the sea-mountain gradient in Point Clouds
           { {0.0, 0, 255, 0}, {0.2, 72, 96, 160}, {0.4, 152, 96, 160}, {0.6, 128, 32, 104}, {0.7, 148, 180, 128}, {1.0, 240, 240, 240}}};
       
    //If we're in custom mode, we should use whatever's in the settings. In practice we generally don't have to do anything. 
    //Return an error and let the caller decide what to do.
    if (m_icse >= ThematicColorScheme_Max)
        return ERROR;

    *fetchedKeys = MIN(s_nKeys[m_icse], MIN (GradientSymb::MAX_GRADIENT_KEYS, maxKeys));
    memcpy (outKeys, s_keys[m_icse], sizeof(s_keys[0][0]) * *fetchedKeys);       
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ThematicColorSchemeProvider::GradientArrays (size_t maxKeys, size_t* fetchedKeys, double* outValues, RgbFactor* outColors)
    {
    ThematicGradientKey keys[GradientSymb::MAX_GRADIENT_KEYS]; 

    if (SUCCESS != GradientKeys (maxKeys, fetchedKeys, keys))
        return ERROR;

    KeysToArrays (*fetchedKeys, keys, outValues, outColors);
        
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t    ThematicLegend::GetNEntries () const
    {
    if (m_parent->GetFlags ().m_valuesByStep && !m_parent->GetFlags ().m_customLegend)
        {
        double          min      = 0, max = 0, step = m_parent->GetLegendValueStep ();

        m_parent->GetRawRange (min, max);

        return static_cast <size_t> (ceil ((max-min)/step));
        }
    else
        return (m_keys.size () == 0) ? 10 : m_keys.size () - 1; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double              ThematicLegend::GetMinValue (size_t i, bool ordered) const
    { 
    if (ordered)
        i = OrderIndex (i);

    if (IsStatic ())
        return m_keys[i].m_value;
        
    double          min      = 0, max = 0;

    m_parent->GetRawRange (min, max);

    if (m_parent->GetFlags ().m_valuesByStep)
        {
        return MIN (max, min + i * m_parent->GetLegendValueStep ());
        }
    else
        {
        double          fraction = (double) i / (double)GetNEntries ();
        
        return min + fraction * (max - min);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ThematicLegend::GetVisible (size_t i, bool ordered) const
    {
    if (ordered)
        i = OrderIndex (i);

    if (IsStatic ())
        return !m_keys[i].m_flags.m_hidden;
    
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicLegend::SetVisible (size_t i, bool visible, bool ordered)
    {
    if (ordered)
        i = OrderIndex (i);

    if (0 == m_keys.size ())
        m_parent->RefreshLegend ();

    m_keys[i].m_flags.m_hidden = !visible;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ThematicLegend::IsStatic () const 
    { 
    return 0 != m_keys.size () && m_parent->GetFlags ().m_customLegend;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
double              ThematicLegend::GetMaxValue (size_t i, bool ordered) const
    { 
    if (ordered)
        i = OrderIndex (i);

    return GetMinValue (i+1, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t           ThematicLegend::OrderIndex (size_t i) const
    {
    return m_parent->GetFlags ().m_invertLegend ? (GetNEntries () - (i + 1)) : i; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ThematicLegend::GetColor (size_t i, bool ordered) const                        
    { 
    if (ordered)
        i = OrderIndex (i);

    if (IsStatic ())
        return m_keys[i].m_color;
    
    double          min      = 0, max = 0;
    
    m_parent->GetRawRange (min, max);
    size_t                  size       = m_parent->GetFlags ().m_valuesByStep ? static_cast <size_t> (ceil ((max-min)/m_parent->GetLegendValueStep ())) : GetNEntries ();

    return m_parent->GetColor ((double) i, 0.0, (double) size - 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+--------------+---------------+---------------+---------------+------*/
void        ThematicDisplaySettings::GetGradient (ThematicGradientKey* gradientKeys, size_t maxKeys, size_t* outNKeys) const
    {
    if(m_data.m_colorScheme == ThematicColorScheme_Custom)
        {
        *outNKeys = MIN(maxKeys, m_gradientKeys.size());
        memcpy (gradientKeys, &m_gradientKeys[0], *outNKeys * sizeof(ThematicGradientKey));
        }
    else
        ThematicColorSchemeProvider (m_data.m_colorScheme).GradientKeys (maxKeys, outNKeys, gradientKeys);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ThematicMeshColorMap::Init (ThematicDisplaySettingsCR settings, size_t size)
    {
    bvector <ColorDef>       falseColors (size);
    ColorDef                 keyColors[GradientSymb::MAX_GRADIENT_KEYS];
    size_t                  fetchedKeys;
    ThematicGradientKey     keys[GradientSymb::MAX_GRADIENT_KEYS];
    IndexArray              intKeyValues (GradientSymb::MAX_GRADIENT_KEYS);

    m_colors.resize (size);
    settings.GetGradient (keys, GradientSymb::MAX_GRADIENT_KEYS, &fetchedKeys);
PUSH_MSVC_IGNORE(4838)
    for(size_t i=0; i < fetchedKeys; i++)
        {
        ColorDef color = ColorDef (keys[i].red, keys[i].green, keys[i].blue);

        intKeyValues[i] = static_cast<int32_t> ((size-1) * keys[i].value);
        keyColors[i] = color;
        falseColors [intKeyValues [i]] = color;
        }
POP_MSVC_IGNORE

    for(size_t i=0; i+1 < fetchedKeys; i++)
        {
        ColorDefP  segmentStart = &falseColors [intKeyValues [i]];
        size_t        nColors      = (size_t) 1 + intKeyValues[i+1] - intKeyValues[i];

        ColorUtil::Interpolate (segmentStart, nColors, keyColors[i], keyColors[i+1]);
        }

    for (size_t i = 0; i < size; i++)
        m_colors[i] = falseColors[i];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ThematicMeshColorMap::Get (double value, double min, double max) const
    {
    if (value < min)
        value = min;
    else if (value > max)
        value = max;

    if (max == min) // Used when there's only one band
        return m_colors[0];

    if (max < min)
        return ColorDef(0);

    return m_colors[(size_t) ((double) (m_colors.size()-1) * (value - min) / (max - min))];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void      ThematicDisplaySettings::GetRange (double& min, double& max) const
    {
    ThematicLegendCR     legend = GetLegend ();

    if (legend.IsStatic ())
        {
        min = legend.GetMinValue (0, false);
        max = legend.GetMaxValue (legend.GetNEntries () - 1, false);
        }
    else
        m_data.m_range.Get (min, max);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicRange const&    ThematicDisplaySettings::GetRange () const
    { 
    static ThematicRange    s_range;
    ThematicLegendCR        legend = GetLegend ();

    s_range = m_data.m_range;

    if (legend.IsStatic ())
        {
        s_range.SetMin (legend.GetMinValue (0, false));
        s_range.SetMax (legend.GetMaxValue (legend.GetNEntries () - 1, false));
        }

    return s_range; 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void       ThematicDisplaySettings::RefreshLegend ()
    {    
    double                              min   = 0, max = 0;
    
    GetRawRange (min, max); //Use the user-supplied range and throw out the min / max values if we have them.
    
    size_t                          nKeys = GetFlags ().m_valuesByStep ? MAX (1, static_cast <size_t> (ceil ((max-min) / GetLegendValueStep ()))) : GetSeedNLegendEntries ();
    bvector <ThematicLegendKey>     keys;
        
    for (size_t i=0; i <= nKeys; i++)
        {
        double              value;
        
        if (GetFlags ().m_valuesByStep)
            {
            //Note: If the legend step is greater than the display range, this will select the minimum on iteration 0 and the maximum on iteration 1, giving us
            // a key that matches the display range.
            if (i == nKeys)
                value = max;
            else
                value = min + i*GetLegendValueStep ();
            }
        else
            value = min + ((double)i / (double) nKeys) * (max-min);

        ColorDef            color = GetColor ((double) i, 0.0, (double) (nKeys) - 1.0);    // Invalid for final key
        ThematicLegendKey   key      = { value, color };

        keys.push_back (key);
        }

    m_legend = ThematicLegend (keys, this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ThematicDisplaySettings::ThematicDisplaySettings () : m_pMaterial (NULL)
    {
    m_marginColor = s_defaultMarginColor;
    m_legendTransparency = 0;
    m_legendValueStep = 0.0; // Special value meaning "Generate a reasonable default"
    m_legend = ThematicLegend (this);
    memset (&m_data, 0, sizeof (m_data)); 
    m_colorMap.Init (*this, 512);
    m_data.m_range.Set(0.0, 1.0);
    
    RefreshLegend();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    ThematicDisplaySettings::SetColorScheme (ThematicColorScheme scheme)  
    { 
    m_data.m_colorScheme = scheme; 
    m_colorMap.Init (*this, 512);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ThematicDisplaySettings::GetColor (double value, double min, double max) const 
    {
    return m_colorMap.Get (value, min, max); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef    ThematicDisplaySettings::GetLegendColor (double value) const 
    {
   ThematicLegendCR         legend      = GetLegend ();
   size_t                   legendSize  = legend.GetNEntries ();

    
    for (size_t i=0; i < legendSize; i++)
        if (value >= legend.GetMinValue (i, false) && value <= legend.GetMaxValue(i, false))
            return legend.GetColor (i, false);

    return GetMarginColor();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Image      ThematicDisplaySettings::GetImage()  const
    {
    int                     marginSize = (int) ((double) THEMATIC_TEXTURE_SIZE * s_margin);
    size_t                  colorSize = THEMATIC_TEXTURE_SIZE - 2 * marginSize;
    // unused - static double           s_defaultDiffuse = .6, s_defaultAmbient = 0.1, s_defaultSbmapecularExponent = .9, s_defaultSpecular = .05, s_defaultFinish = 0.1;

    ThematicLegendCR        legend      = GetLegend ();
    size_t                  legendSize  = legend.GetNEntries ();
    double                  legendMin   = legend.GetMinValue (0, false), legendMax = legend.GetMaxValue (legendSize - 1, false);
    
    for (int i=0; i<marginSize; i++)
        m_texturePixels[i] = m_texturePixels[THEMATIC_TEXTURE_SIZE - i - 1] = ColorDef(GetMarginColor(), m_data.m_flags.m_outOfRangeTransparent ? 0xff : 0);

    // Note: In Isolines mode we need the "stepped" material to display the "compressed" legend (when the full legend doesn't fit in the view),
    // Accurate, Fast, and Fast + Isolines need it for obvious reasons, which leaves only None to use the smooth texture.
    if (ThematicSteppedDisplay_None != GetFlags().m_steppedDisplay && 0 != legendSize)
        {
        double          pixelsPerValue = colorSize / (legendMax - legendMin);

        size_t          currentPixel = marginSize;

        for (size_t i=0; i < legendSize; i++)
            {
            size_t minPix = static_cast <size_t> (pixelsPerValue * (legend.GetMinValue (i, false) - legendMin));
            size_t maxPix = static_cast <size_t> (pixelsPerValue * (legend.GetMaxValue (i, false) - legendMin));

            for (size_t j=minPix; j < maxPix; j++, currentPixel++)
                m_texturePixels[currentPixel] = ColorDef(legend.GetColor (i, false), legend.GetVisible (i, false) ?  0 : 0xff);
            }

        if (ThematicSteppedDisplay_FastWithIsolines == GetFlags().m_steppedDisplay)
            {
            for (size_t i=0; i<legendSize; i++)
                {
                if (legend.GetVisible (i, false))
                    {
                    double  fractionMin = (legend.GetMinValue (i, false) - legendMin) / (legendMax - legendMin);
                    double  fractionMax = (legend.GetMaxValue (i, false) - legendMin) / (legendMax - legendMin);

                    m_texturePixels[marginSize + (int)(fractionMin * colorSize)] = m_texturePixels[marginSize + (int)(fractionMax * colorSize)] = ColorDef::Black();
                    }
                }
            }
        }
    else
        {
        for (size_t i=0; i<colorSize; i++)
            m_texturePixels[marginSize + i] = GetColor ((double) i, 0.0, (double) (colorSize - 1));
        }

    for (auto& texturePixel : m_texturePixels)
        texturePixel.SetAlpha(255);

    return Image(1, THEMATIC_TEXTURE_SIZE, ByteStream(reinterpret_cast<uint8_t*> (&m_texturePixels[0]), sizeof(m_texturePixels)), Image::Format::Rgba);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint2d    computeTextureParam (double value)
    {
    static double       s_paramEpsilon = .00002; 
    static double       s_minMargin = s_margin, s_maxMargin = 1.0 - s_margin;
    static double       s_minLimit = -s_margin + s_paramEpsilon, s_maxLimit = 1.0 + s_minMargin - s_paramEpsilon;
    
    double              textureValue = s_margin + s_colorRange * value; 
            
    if (textureValue < s_minLimit)
        textureValue = s_minLimit;
    else if (textureValue > s_maxLimit)
        textureValue = s_maxLimit;
    else if (fabs (textureValue - s_minMargin) < s_paramEpsilon)
        textureValue += s_paramEpsilon;
    else if (fabs (textureValue - s_maxMargin) < s_paramEpsilon)
        textureValue -= s_paramEpsilon;
    
    return DPoint2d::From (.5, 1.0 - textureValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ElementTileTree::ThematicMeshBuilder::ThematicMeshBuilder(Utf8StringCR channel, SystemCR system, DgnDbR db, ThematicDisplaySettingsCR settings, ThematicCookedRangeCR cookedRange) : m_channel(channel), m_cookedRange(cookedRange)
    {
    TexturePtr                  texture = system._CreateTexture(settings.GetImage(), db);
    
    m_textureMapping = TextureMapping(*texture, TextureMapping::Params());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool   ElementTileTree::ThematicMeshBuilder::DoThematicDisplay(PolyfaceHeaderR mesh, TextureMappingR textureMapping) const  
    {
    PolyfaceAuxChannelCPtr        channel;

    if (! mesh.GetAuxDataCP().IsValid() ||
        ! (channel = mesh.GetAuxDataCP()->GetChannel(m_channel.c_str())).IsValid())
        return false;

    textureMapping = m_textureMapping;
    mesh.ParamIndex().clear();
    mesh.Param().clear();

    mesh.ParamIndex().SetActive(true);
    mesh.ParamIndex().resize(mesh.PointIndex().size());
    memcpy (mesh.ParamIndex().data(), mesh.GetAuxDataCP()->GetIndices().data(), mesh.PointIndex().size() * sizeof(int32_t));

    auto& values = channel->GetData().front()->GetValues(); 

    mesh.Param().reserve(values.size());
    mesh.Param().SetActive(true);
    
    for (auto value : values)
        mesh.Param().push_back(computeTextureParam(m_cookedRange.GetNormalizedValueFromRaw((double) value)));

    return true;
    }


