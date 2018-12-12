/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudColorDef.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

// POINTCLOUD_WIP_GR06_ColorDef. We created this class because DgnPlatform does not provide any RGB color definition (i.e. RGB, no alpha). And we need
//                               this because Vortex returns colors as RGB channels. We'll be able to delete this class once we have a similar service
//                               in platform.

//=======================================================================================
//! RGB values for a color
//! @bsiclass
//=======================================================================================
struct PointCloudColorDef
{
private:
    Byte    m_red;
    Byte    m_green;
    Byte    m_blue;

public:
    void SetColors (Byte r, Byte g, Byte b) {m_red = r; m_green = g; m_blue = b;}
    void SetAllColors (Byte val) {m_red = m_green = m_blue = val;}
    void SetRed(Byte v) {m_red=v;}
    void SetGreen(Byte v) {m_green=v;}
    void SetBlue(Byte v) {m_blue=v;}
    Byte GetRed() const {return m_red;}
    Byte GetGreen() const {return m_green;}
    Byte GetBlue() const {return m_blue;}

    PointCloudColorDef () {}
    PointCloudColorDef (Byte red, Byte green, Byte blue) {SetColors (red,green,blue);}
};


END_BENTLEY_BEPOINTCLOUD_NAMESPACE
