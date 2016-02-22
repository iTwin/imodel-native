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
    bvector<Byte>           m_tileBuffer;
    RasterFilePtr           m_rasterFilePtr;
    Point2d                 m_tileSize;
    HFCPtr<HCDPacket>       m_packetPtr;
    std::mutex              m_imageppLock;      //&&MM temp. imagepp is not thread safe. what about gcoord? where to put that lock?

private:
            RasterFileSource(Utf8StringCR resolvedName);
    virtual ~RasterFileSource(){};
			Byte*			CreateWorkingBuffer(size_t& bufferSize, const HRPPixelType& pi_rPixelType, uint32_t pi_Width, uint32_t pi_Height) const;


protected:
    virtual Render::ImagePtr _QueryTile(TileId const& id, bool request) override;

public:
    static  RasterSourcePtr Create(Utf8StringCR resolvedName);
    
};


END_BENTLEY_RASTERSCHEMA_NAMESPACE