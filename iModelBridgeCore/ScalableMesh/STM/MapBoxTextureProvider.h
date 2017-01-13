#pragma once

#include <ScalableMesh\ITextureProvider.h>
#include "ImagePPHeaders.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct MapBoxTextureProvider : virtual public ITextureProvider
    {
    private:
        HGF2DExtent m_minExt;
        DRange3d m_totalExt;

    protected:

        virtual DPoint2d _GetMinPixelSize() override;

        virtual DRange2d _GetTextureExtent() override;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area) override;
    public:

        MapBoxTextureProvider(WString url, DRange3d totalExt, GeoCoordinates::BaseGCSCPtr targetCS);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE