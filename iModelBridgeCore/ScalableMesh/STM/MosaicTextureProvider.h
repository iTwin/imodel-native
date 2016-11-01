#pragma once

#include <ScalableMesh\ITextureProvider.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct MosaicTextureProvider : virtual public ITextureProvider
    {
    private:
        HIMMosaic* m_targetMosaic;

    protected:

        virtual DPoint2d _GetMinPixelSize() override;

        virtual DRange2d _GetTextureExtent() override;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d area) override;
    public:

        MosaicTextureProvider(HIMMosaic* mosaic);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE
