/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ITiledRaster.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_RENDER_NAMESPACE

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ITiledRaster : IRefCounted
{
    virtual void _DrawRaster(OutputR viewOutput) = 0;
    virtual void _PrintRaster(OutputR viewOutput) = 0;
};

END_BENTLEY_RENDER_NAMESPACE
