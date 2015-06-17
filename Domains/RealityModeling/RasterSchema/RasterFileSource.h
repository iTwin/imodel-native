/*-------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/RasterFileSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    bvector<Byte>           m_tileBuffer;
    RasterFilePtr           m_rasterFilePtr;

private:
            RasterFileSource(RasterFileProperties const& properties);
    virtual ~RasterFileSource(){};


protected:
    virtual DisplayTilePtr _QueryTile(TileId const& id, bool request) override;

public:
    static  RasterSourcePtr Create(RasterFileProperties const& properties);
    
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE