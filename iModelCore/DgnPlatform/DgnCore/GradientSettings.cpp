/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void GradientSymb::CopyFrom(GradientSymb const& other)
    {
    m_mode  = other.m_mode;
    m_flags = other.m_flags;
    m_nKeys = other.m_nKeys;
    m_angle = other.m_angle;
    m_tint  = other.m_tint;
    m_shift = other.m_shift;
    m_thematicSettings = other.m_thematicSettings;

    memcpy(m_colors, other.m_colors, m_nKeys * sizeof(m_colors[0]));
    memcpy(m_values, other.m_values, m_nKeys * sizeof(m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/07
+---------------+---------------+---------------+---------------+---------------+------*/
void GradientSymb::SetKeys(uint32_t nKeys, ColorDef const* pColors, double const* pValues)
    {
    m_nKeys = nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : nKeys;

    memcpy(m_colors, pColors, m_nKeys * sizeof(m_colors[0]));
    memcpy(m_values, pValues, m_nKeys * sizeof(m_values[0]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool GradientSymb::operator==(GradientSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_mode != m_mode)
        return false;

    if (rhs.m_flags != m_flags)
        return false;

    if (rhs.m_nKeys != m_nKeys)
        return false;

    if (rhs.m_angle != m_angle)
        return false;

    if (rhs.m_tint  != m_tint)
        return false;

    if (rhs.m_shift != m_shift)
        return false;

    int nKeys = m_nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : m_nKeys;

    for (int i=0; i<nKeys; ++i)
        {
        if (rhs.m_values[i] != m_values[i])
            return false;

        if (rhs.m_colors[i] != m_colors[i])
            return false;
        }
    if (m_mode == Mode::Thematic && ! (rhs.m_thematicSettings == m_thematicSettings))
        return false;


    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GradientSymb::operator<(GradientSymbCR rhs) const
    {
    if (&rhs == this)
        return false;
    else if (m_mode != rhs.m_mode)
        return m_mode < rhs.m_mode;
    else if (m_flags != rhs.m_flags)
        return m_flags < rhs.m_flags;
    else if (m_nKeys != rhs.m_nKeys)
        return m_nKeys < rhs.m_nKeys;
    else if (m_angle != rhs.m_angle)
        return m_angle < rhs.m_angle;
    else if (m_tint != rhs.m_tint)
        return m_tint < rhs.m_tint;
    else if (m_shift != rhs.m_shift)
        return m_shift < rhs.m_shift;

    int nKeys = m_nKeys > MAX_GRADIENT_KEYS ? MAX_GRADIENT_KEYS : m_nKeys;
    for (int i = 0; i < nKeys; i++)
        {
        if (m_values[i] != rhs.m_values[i])
            return m_values[i] < rhs.m_values[i];
        else if (m_colors[i] != rhs.m_colors[i])
            return m_colors[i].GetValue() < rhs.m_colors[i].GetValue();
        }

    if (m_mode == Mode::Thematic &&
        !(rhs.m_thematicSettings == m_thematicSettings))
        return m_thematicSettings < rhs.m_thematicSettings;
        

    return false;
    }

static Byte roundToByte(double f) { return (Byte) std::min (f + .5, 255.0); }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef GradientSymb::MapColor(double value) const
    {
    double  d, w0, w1;

    if (value < 0.0)
        value = 0.0;
    else if (value > 1.0)
        value = 1.0;

    if (0 != (m_flags & Flags::Invert))
        value = 1.0 - value;

    size_t      index = 0;
    if (m_nKeys <= 2)
        {
        w0 = 1.0 - value;
        w1 = value;
        }
    else // locate value in map, blend corresponding colors
        {
        while (index < m_nKeys - 2 && value > m_values[index + 1])
            index++;

        d = m_values[index + 1] - m_values[index];
        w1 = d < 0.0001 ? 0.0 : (value - m_values[index]) / d;
        w0 = 1.0 - w1;
        }

    ColorDefCR      color0 = m_colors[index], color1 = m_colors[index+1];
    double          red     = w0 * (double) color0.GetRed()   + w1 * (double) color1.GetRed();
    double          green   = w0 * (double) color0.GetGreen() + w1 * (double) color1.GetGreen();
    double          blue    = w0 * (double) color0.GetBlue()  + w1 * (double) color1.GetBlue();
    double          alpha   = w0 * (double) color0.GetAlpha() + w1 * (double) color1.GetAlpha();

    return ColorDef(roundToByte(red), roundToByte(green), roundToByte(blue), 0xff - roundToByte(alpha));
    }

/*---------------------------------------------------------------------------------**//**
* This code was more or less copied from QvTexture::initGradientTexture().
* That code produces the image flipped horizontally and vertically...texture mapping
* has identity transform...not clear how they end up rendering it correctly.
* I modified our version to write the image in "reverse".
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Image GradientSymb::GetImage(uint32_t width, uint32_t height) const
    {
    constexpr   uint32_t        s_thematicImageHeight = 8192;
    if (IsThematic())       // If Thematic override for maximum resolution --- TBD - Check on devices that require square texture.
        {
        width = 1;          
        height = s_thematicImageHeight;
        }
    double                  cosA = cos(GetAngle()), sinA = sin(GetAngle());
    double                  d, f, r, x, y, xr, yr, xs, ys;
    double                  dMin, dMax;
    double                  shift = std::min(1.0, fabs(GetShift()));
    ByteStream              imageBytes(4 * width * height);
    uint32_t*               pImage = reinterpret_cast<uint32_t*>(imageBytes.data()) + width*height;

    switch (GetMode())
        {
        case GradientSymb::Mode::Linear:
        case GradientSymb::Mode::Cylindrical:
            {
            xs = 0.5- 0.25 * shift * cosA;
            ys = 0.5- 0.25 * shift * sinA;
            dMin = dMax = 0.0;
            for (size_t j = 0; j < 2; j++)
                {
                for (size_t i = 0; i < 2; i++)
                     {
                     d = (i - xs) * cosA + (j - ys) * sinA;
                     if (d < dMin)
                         dMin = d;
                     if (d > dMax)
                         dMax = d;
                     }
                }
            for (size_t j = 0; j < height; j++)
                {
                y = (double) j / 255.0 - ys;
                for (size_t i = 0; i < width; i++)
                    {
                    x = (double) i / 255.0 - xs;
                    d = x * cosA + y * sinA;
                    if (GradientSymb::Mode::Linear == GetMode())
                        {
                        if (d > 0.0)
                            f = 0.5 + 0.5 * d / dMax;
                        else
                            f = 0.5 - 0.5 * d / dMin;
                        }
                    else
                        {
                        if (d > 0.0)
                            f = (double) (sin (msGeomConst_piOver2 * (1.0 - d / dMax)));
                        else
                            f = (double) (sin (msGeomConst_piOver2 * (1.0 - d / dMin)));
                        }
                    *--pImage = MapColor(f).GetValue();
                    }
                }
            break;
            }
        case GradientSymb::Mode::Curved:
            {
            xs = 0.5 + 0.5 * sinA - 0.25 * shift * cosA;
            ys = 0.5 - 0.5 * cosA - 0.25 * shift * sinA;
            for (size_t j = 0; j < height; j++)
                {
                y = (double) (j) / 255.0 - ys;
                for (size_t i = 0; i < width; i++)
                    {
                    x = (double) (i) / 255.0 - xs;
                    xr = 0.8f * (x * cosA + y * sinA);
                    yr = y * cosA - x * sinA;
                    f = (double) (sin (msGeomConst_piOver2 * (1.0 - sqrt (xr * xr + yr * yr))));
                    *--pImage = MapColor(f).GetValue();
                    }
                }

            break;
            }
        case GradientSymb::Mode::Spherical:
            {
            r = 0.5 + 0.125 * (double) (sin (2.0 * GetAngle()));
            xs = 0.5 * shift * (cosA + sinA) * r;
            ys = 0.5 * shift * (sinA - cosA) * r;
            for (size_t j = 0; j < height; j++)
                {
                y = ys + (double) (j) / 255.0 - 0.5;
                for (size_t i = 0; i < width; i++)
                    {
                    x = xs + (double) (i) / 255.0 - 0.5;
                    f = (double) (sin (msGeomConst_piOver2 * (1.0 - sqrt (x * x + y * y) / r)));
                    *--pImage = MapColor(f).GetValue();
                    }
                }
            break;
            }
        case GradientSymb::Mode::Hemispherical:
            {
            xs = 0.5 + 0.5 * sinA - 0.5 * shift * cosA;
            ys = 0.5 - 0.5 * cosA - 0.5 * shift * sinA;
            for (size_t j = 0; j < height; j++)
                {
                y = (double) (j) / 255.0 - ys;
                for (size_t i = 0; i < width; i++)
                    {
                    x = (double) (i) / 255.0 - xs;
                    f = (double) (sin (msGeomConst_piOver2 * (1.0 - sqrt (x * x + y * y))));
                    *--pImage = MapColor(f).GetValue();
                    }
                }
            break;
            }
    
        case GradientSymb::Mode::Thematic:
            {
            ThematicGradientSettingsCPtr    settings = GetThematicSettings();
                
            if (!settings.IsValid())
                {
                BeAssert(false);
                settings = new ThematicGradientSettings();
                }

            // TBD -- Stepped and isolines...
            for (size_t j = 0; j < height; j++)
                {
                f = 1.0 - (double) j / height;
                uint32_t    color = 0;

                if (f < settings->GetMargin() || f > 1.0 - settings->GetMargin())
                    {
                    color = settings->GetMarginColor().GetValue();
                    }
                else
                    {
                    f = (f - settings->GetMargin()) / (1.0 - 2.0 * settings->GetMargin());
                    switch (settings->GetMode())
                        {
                        case ThematicGradientSettings::Mode::SteppedWithDelimiter:
                        case ThematicGradientSettings::Mode::Stepped:
                            if (0 != settings->GetStepCount())
                                {
                                double fStep = floor (f * (double) settings->GetStepCount() + .99999) /  ((double) (settings->GetStepCount()));
                                static double  s_delimitFraction = 1.0 / 1024.0;

                                if (ThematicGradientSettings::Mode::SteppedWithDelimiter == settings->GetMode() && fabs(fStep - f) < s_delimitFraction)
                                    color = 0xff000000;
                                else
                                    color = MapColor(fStep).GetValue();
                                }
                            break;

                        case ThematicGradientSettings::Mode::Smooth:
                            color = MapColor (f).GetValue();
                            break;
                        }
                    }

                for (size_t i = 0; i < width; i++)
                    *--pImage = color;
                }
            }

        }
    Render::Image image (width, height, std::move(imageBytes), Image::Format::Rgba);

#ifdef  TEST_IMAGE 
    std::FILE*       pFile;
    if (nullptr != (pFile = fopen("d:\\t3\\tmp\\png", "wb")))
        {
        ImageSource imageSource(image, ImageSource::Format::Png);
        fwrite(imageSource.GetByteStream().GetDataP(), 1, imageSource.GetByteStream().GetSize(), pFile);
        fclose(pFile);
        }
#endif
    return image;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GradientSymb::HasTransparency() const
    {
    for (uint32_t i = 0; i < m_nKeys; i++)
        if (m_colors[i].GetAlpha() > 0)
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GradientSymb::GetKey(ColorDef& color, double& value, uint32_t iKey) const
    {
    if (iKey >= m_nKeys)
        return ERROR;

    color = m_colors[iKey];
    value = m_values[iKey];

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GradientSymb::FromJson(Json::Value const& json)
    {
    if (!json.isMember("mode") ||
        !json["keys"].isArray())
        return ERROR;
    
    m_mode  = (Mode) json["mode"].asUInt();
    m_flags = (Flags) json["flags"].asUInt();
    m_angle = JsonUtils::ToAngle(json["angle"]).Radians();
    m_tint  = json["tint"].asDouble();
    m_shift = json["shift"].asDouble(); 
    

    m_nKeys = std::min((uint32_t) json["keys"].size(), (uint32_t) MAX_GRADIENT_KEYS);

    for (uint32_t i=0; i<m_nKeys; i++)
        {
        m_colors[i] = ColorDef(json["keys"][i]["color"].asUInt());
        m_values[i] = json["keys"][i]["value"].asDouble();
        }

    if (m_mode == Mode::Thematic)
        {
        m_thematicSettings = new ThematicGradientSettings();
        m_thematicSettings->FromJson(json["thematicSettings"]);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GradientSymb::ToJson () const
    {
    Json::Value value;

    value["mode"] = (int) GetMode();

    if (Flags::None != GetFlags())
        value["flags"] = (int) GetFlags();

    if (0.0 != GetAngle())
        value["angle"] = JsonUtils::FromAngle(Angle::FromRadians(GetAngle()));

    if (0.0 != GetTint())
        value["tint"] = GetTint();

    if (0.0 != GetShift())
        value["shift"] = GetShift();

    ColorDef    color;
    double      keyValue;

    for (uint32_t i=0; SUCCESS == GetKey(color, keyValue, i); i++)
        {
        Json::Value keyJson;

        keyJson["value"] = keyValue;
        keyJson["color"] = color.GetValue();

        value["keys"].append(keyJson);
        }

    if (Mode::Thematic == GetMode() && m_thematicSettings.IsValid())
        value["thematicSettings"] = m_thematicSettings->ToJson();
    
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     ThematicGradientSettings::ToJson () const
    {
    Json::Value value;

    value["mode"] = (uint32_t) GetMode();
    value["stepCount"] = GetStepCount();
    value["marginColor"] = GetMarginColor().GetValue();
    value["colorScheme"] = (uint32_t) GetColorScheme();
    value["rangeLow"] = GetRange().low;
    value["rangeHigh"] = GetRange().high;

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void     ThematicGradientSettings ::FromJson (Json::Value const& value) 
    {
    m_stepCount = value["stepCount"].asUInt();
    m_marginColor = ColorDef(value["marginColor"].asUInt());
    m_mode = (Mode) value["mode"].asUInt();
    m_colorScheme = (ColorScheme) value["colorScheme"].asUInt();
    m_range.low = value["rangeLow"].asDouble();
    m_range.high = value["rangeHigh"].asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GradientSymb::GradientSymb(ThematicGradientSettingsR settings) : m_thematicSettings(&settings)
    {
    struct  SchemeKey
        {
        double          m_value;
        uint8_t         m_red;
        uint8_t         m_blue;
        uint8_t         m_green;
        };

    static const   bvector<bvector<SchemeKey>> s_keys =
        {  
           { {0.0, 0,   255, 0}, {0.25, 0,   255, 255}, {0.5, 0, 0, 255}, {0.75, 255, 0,   255}, {1.0, 255, 0,   0}},
           { {0.0, 255, 0,   0}, {0.25, 255, 0,   255}, {0.5, 0, 0, 255}, {0.75, 0,   255, 255}, {1.0, 0,   255, 0}},
           { {0.0, 0,   0,   0}, {1.0,  255, 255, 255}},

           //Based off of the topographic gradients in Point Clouds
           { {0.0, 152, 148, 188}, {0.5, 204, 160, 204}, {1.0, 152, 72, 128}},

           //Based off of the sea-mountain gradient in Point Clouds
           { {0.0, 0, 255, 0}, {0.2, 72, 96, 160}, {0.4, 152, 96, 160}, {0.6, 128, 32, 104}, {0.7, 148, 180, 128}, {1.0, 240, 240, 240}}
        };

    if (settings.GetColorScheme() < ThematicGradientSettings::ColorScheme::Custom)
        {
        auto&   schemeKeys = s_keys[(uint32_t) settings.GetColorScheme()];

        m_mode = Mode::Thematic;
        m_nKeys = static_cast<uint32_t>(schemeKeys.size());
    
        for (size_t i=0; i<schemeKeys.size(); i++)
            {
            m_colors[i] = ColorDef(schemeKeys[i].m_red, schemeKeys[i].m_green, schemeKeys[i].m_blue);
            m_values[i] = schemeKeys[i].m_value;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ThematicGradientSettings::operator==(ThematicGradientSettingsCR rhs) const
    {
    if (this == &rhs)
        return true;

    return m_stepCount      == rhs.m_stepCount &&
           m_marginColor    == rhs.m_marginColor &&
           m_mode           == rhs.m_mode &&
           m_colorScheme    == rhs.m_colorScheme &&
           m_range.IsEqualLowHigh(rhs.m_range);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool ThematicGradientSettings::operator<(ThematicGradientSettingsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (m_stepCount != rhs.m_stepCount)
        return m_stepCount < rhs.m_stepCount;

    if (m_marginColor.GetValue() != rhs.m_marginColor.GetValue())
        return m_marginColor.GetValue() != rhs.m_marginColor.GetValue();

    if (m_mode != rhs.m_mode)
        return m_mode < rhs.m_mode;

    return m_colorScheme < rhs.m_colorScheme;
    }
