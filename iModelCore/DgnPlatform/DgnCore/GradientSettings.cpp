/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GradientSettings.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Image GradientSymb::GetImage(uint32_t width, uint32_t height) const
    {
    double                  cosA = cos(GetAngle()), sinA = sin(GetAngle());
    double                  d, f, r, x, y, xr, yr, xs, ys;
    double                  dMin, dMax;
    double                  shift = std::min(1.0, fabs(GetShift()));
    ByteStream              imageBytes(4 * width * height);
    uint32_t*               pImage = (uint32_t*) imageBytes.data();

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
                    *pImage++ = MapColor(f).GetValue();
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
                    *pImage++ = MapColor(f).GetValue();
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
                    *pImage++ = MapColor(f).GetValue();
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
                    *pImage++ = MapColor(f).GetValue();
                    }
                }
            break;
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
    m_angle = json["angle"].asDouble(); 
    m_tint  = json["tint"].asDouble();
    m_shift = json["shift"].asDouble(); 
    
    m_nKeys = std::min((uint32_t) json["keys"].size(), (uint32_t) MAX_GRADIENT_KEYS);

    for (uint32_t i=0; i<m_nKeys; i++)
        {
        m_colors[i] = ColorDef(json["keys"][i]["color"].asUInt());
        m_values[i] = json["keys"][i]["value"].asDouble();
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value     GradientSymb::ToJson () const
    {
    Json::Value     value;

    value["mode"]  = (int) GetMode();
    value["flags"] = (int) GetFlags();
    value["angle"] = GetAngle(); 
    value["tint"] = GetTint();
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
    
    return value;
    }


