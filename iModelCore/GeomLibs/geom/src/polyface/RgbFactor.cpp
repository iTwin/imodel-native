/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RgbFactor RgbFactor::From (DPoint3dCR data)
    {
    RgbFactor color;
    color.red     = data.x;
    color.green   = data.y;
    color.blue    = data.z;
    return color;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool RgbFactor::Equals (RgbFactor const &other) const
    {
    return red == other.red
        && green == other.green
        && blue  == other.blue;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool RgbFactor::EqualsInt (RgbFactor const &other) const
    {
    return ToIntColor () == other.ToIntColor ();
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RgbFactor RgbFactor::FromIntColor (int32_t source)
    {
    RgbFactor   rgbFactor;
//    double      scale = 1.0 / (double) UCHAR_MAX;
    uint8_t*    pByte = (uint8_t *) (&source);

    rgbFactor.red   = (double) pByte[0] / (double) UCHAR_MAX;
    rgbFactor.green = (double) pByte[1] / (double) UCHAR_MAX;
    rgbFactor.blue  = (double) pByte[2] / (double) UCHAR_MAX;

    return rgbFactor;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
int32_t RgbFactor::ToIntColor () const
    {
    int32_t     intColor = 0;
    double      scale = (double) UCHAR_MAX;
    uint8_t*    pByte = (uint8_t *) (&intColor);

    pByte[0] = (uint8_t) (red   * scale + 0.5);
    pByte[1] = (uint8_t) (green * scale + 0.5);
    pByte[2] = (uint8_t) (blue  * scale + 0.5);

    return intColor; 
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
RgbFactor RgbFactor::From (double red, double green, double blue)
    {
    RgbFactor color;
    color.red     = red;
    color.green   = green;
    color.blue    = blue;
    return color;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RgbFactor::AddInPlace (RgbFactor const &data)
    {
    red     += data.red;
    green   += data.green;
    blue    += data.blue;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void RgbFactor::ScaleInPlace (double s)
    {
    red     *= s;
    green   *= s;
    blue    *= s;
    }

END_BENTLEY_GEOMETRY_NAMESPACE