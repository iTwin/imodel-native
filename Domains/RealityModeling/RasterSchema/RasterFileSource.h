/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "RasterSource.h"

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eric.Paquet     06/2015
//=======================================================================================
struct RasterFileSource : public RasterSource
{
private:
    RasterFileProperties    m_properties;
    RasterFilePtr           m_rasterFilePtr;

private:
            RasterFileSource(Utf8StringCR resolvedName);
    virtual ~RasterFileSource(){};

protected:
    virtual Render::Image _QueryTile(TileId const& id, bool& alphaBlend) override;

public:
    static  RasterSourcePtr Create(Utf8StringCR resolvedName);
    
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE