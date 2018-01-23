/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/HDRImage.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#include <zlib/zip/unzip.h>

#define  MINELEN	8                       // minimum scanline length for encoding
#define  MAXELEN	0x7fff			// maximum scanline length for encoding

struct RGBE 
    { 
    uint8_t     m_r;
    uint8_t     m_g;
    uint8_t     m_b;
    uint8_t     m_exponent;
    };


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
static bool rawLoadHDRScanline(RGBE* scanline, uint8_t const*& srcData, uint8_t const* srcEnd, uint32_t length)
    {
    int i;
    int rshift = 0;

    while (length > 0) 
        {
        scanline[0].m_r = *srcData++;
        scanline[0].m_g = *srcData++;
        scanline[0].m_b = *srcData++;
        scanline[0].m_exponent = *srcData++;
        if (srcData > srcEnd)
            return ERROR;

        if (scanline[0].m_r == 1 &&
            scanline[0].m_g == 1 &&
	    scanline[0].m_b == 1) 
            {
	    for (i = scanline[0].m_exponent << rshift; i > 0; i--) 
                {
		memcpy(&scanline[0], &scanline[-1], 4);
		scanline++;
		length--;
                }
            rshift += 8;
	    }
        else 
            {
	    scanline++;
	    length--;
	    rshift = 0;
	    }
	}
    return SUCCESS;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
static bool loadHDRScanline(RGBE* scanline, uint8_t const*& srcData, uint8_t const* srcEnd, uint32_t length)
    {
    if (length < MINELEN || length > MAXELEN || *srcData != 2)
        return rawLoadHDRScanline(scanline, srcData, srcEnd, length);

    srcData++;

    scanline[0].m_g = *srcData++;
    scanline[0].m_b = *srcData++;

    int     e = *srcData++;

    if (scanline[0].m_g != 2 || scanline[0].m_b & 128) 
        {
        scanline[0].m_r = 2;
	scanline[0].m_exponent = e;
	return rawLoadHDRScanline(scanline + 1, srcData, srcEnd, length - 1);
        }

    uint8_t*     pScanline = &scanline->m_r;
    for (uint32_t i = 0; i < 4; i++) 
        {
        for (uint32_t j = 0; j < length; ) 
            {
            uint8_t code = *srcData++;
            if (code > 128) 
                { // run
                code &= 127;
		uint8_t val = *srcData++;
                while (code--)
                    pScanline[i + 4 * j++] = val;
                }
            else  
                {	// non-run
                while(code--)
                    pScanline[i + 4 * j++] = *srcData++;
		}
            BeAssert(j <= length);
            }
        }
    return srcData <= srcEnd ? SUCCESS : ERROR;
    }


/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   parseValue(uint32_t& value, char& sign, char& dimension, char const*& srcData)
    {
    char const*     pSeparator;
    char            string[1024];

    sign = *srcData++;
    dimension = *srcData++;

    if ((sign != '-' && sign != '+') ||
        (dimension!= 'X' && dimension != 'Y') ||
        *srcData++ != ' ')
        return ERROR;

    int             i=0;
    while (std::isdigit(*srcData))
        string[i++] = *srcData++;

    string[i++] = 0;
    
    if (*srcData == ' ' || *srcData == '\n')
        srcData++;
    
    return (1 == std::sscanf(string, "%u", &value)) ? SUCCESS : ERROR;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
HDRImage HDRImage::FromHDR(uint8_t const* pData, uint32_t srcLen, Encoding encoding)
    {  
    char const*     srcCharData = reinterpret_cast<char const*> (pData);
    char const*     srcCharEnd = srcCharData + srcLen;

    if (0 == memcmp(srcCharData, "#?RADIANCE", 10)) 
        srcCharData += 10;
    else if (0 == memcmp(srcCharData, "#?RGBE", 6))
        srcCharData += 6;
    else
        {
        BeAssert(false && "Invalid HDR header");
        return HDRImage();;
        }

    // skip forward to consecutive linefeeds.
    while (*srcCharData != '\n' || *(srcCharData+1) != '\n' && srcCharData < srcCharEnd)
        srcCharData++;

    if (srcCharData >= srcCharEnd)
        {
        BeAssert (false && "Consecutive linefeeds not found\n");
        return HDRImage();;
        }
    srcCharData += 2;       // Skip linefeeds.

    char        firstSign, firstDimension, secondSign, secondDimension;
    uint32_t    firstValue, secondValue;

    if (SUCCESS != parseValue(firstValue, firstSign, firstDimension, srcCharData) ||
        SUCCESS != parseValue(secondValue, secondSign, secondDimension, srcCharData))
        {
        BeAssert(false);
        return HDRImage();;
        }
    HDRImage    image;
    image.m_encoding = Encoding::RGBE;
    image.m_format = Image::Format::Rgba;       // Not really... alpha == exponent.
    image.m_width  = (firstDimension == 'X') ? firstValue : secondValue;
    image.m_height = (firstDimension == 'Y') ? firstValue : secondValue; 

    size_t      rowBytes = 4 * image.m_width;
    image.m_image.resize(rowBytes * image.m_height);

    uint8_t const*  srcData = reinterpret_cast<uint8_t const*> (srcCharData);
    uint8_t const*  srcEnd  = pData + srcLen;

    for (uint32_t i=0; i < image.m_height; i++)
        {
        if (SUCCESS != loadHDRScanline ((RGBE*) (image.m_image.data() + i * rowBytes), srcData, srcEnd, image.m_width))
            {
            BeAssert(false && "Scanline error");
            return HDRImage();
            }
        }
    if (Encoding::RGBE != encoding)
        image.Encode(encoding);

    return image;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HDRImage::Encode(Encoding encoding) 
    {
    if (encoding == m_encoding)
        return;     // Nothing to do...

    if (Encoding::RGBM != encoding)
        {
        BeAssert(false && "encoding not implemented");
        return;
        }
    for (uint8_t *imageData = m_image.data(), *imageDataEnd = imageData + m_width * m_height * 4; imageData < imageDataEnd; imageData += 4)
        {
        constexpr   float s_maxScale = 5.0;
        int         exponent = imageData[3] - 128;  

        float   d = (float) pow(2.0f, exponent) / 256.0f;
        uint8_t maxByte = std::max (imageData[0], std::max(imageData[1], imageData[2]));
        float   maxValue = std::min (s_maxScale, d * maxByte);
        float   multiplier = maxValue / s_maxScale;
        float   scale = 255.0 * d / (s_maxScale * maxValue);

        imageData[0] = (uint8_t) (std::min (255.0f, scale * (float) imageData[0]));
        imageData[1] = (uint8_t) (std::min (255.0f, scale * (float) imageData[1]));
        imageData[2] = (uint8_t) (std::min (255.0f, scale * (float) imageData[2]));
        imageData[3] = (int8_t) (255.0 * multiplier);
        }
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<float> HDRImage::Decode() const
    { 
    bvector<float>  decoded;        // Not current used/tested.

    if (!IsValid())
        {
        BeAssert(false);
        return decoded;
        }
    size_t      nPixels = m_width * m_height;

    decoded.resize(nPixels*3);
    uint8_t     const*  in = m_image.data();
    float*      out = decoded.data();

    for (size_t i=0; i<nPixels; i++, in+=4, out+=3)
        {
        if (0 == in[3])
            {
            out[0] = out[1] = out[2] = 0.0;
            }
        else
            {
            int     exponent = in[3] - 128;   
            float   d = (float) pow(2.0f, exponent) / 256.0f;

            out[0] = in[0] * d;
            out[1] = in[1] * d;
            out[2] = in[2] * d;
            }
        }

    
    return decoded;
    }
    