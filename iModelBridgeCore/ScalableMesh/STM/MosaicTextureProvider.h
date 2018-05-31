#pragma once

#include <ScalableMesh/ITextureProvider.h>
#include "ImagePPHeaders.h"

USING_NAMESPACE_IMAGEPP
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct MosaicTextureProvider : virtual public ITextureProvider
    {
    private:
        HFCPtr<HIMMosaic> m_targetMosaic;
        HGF2DExtent       m_minExt;

    protected:

        virtual DPoint2d _GetMinPixelSize() override;

        virtual DRange2d _GetTextureExtent() override;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area) override;
    public:

        BENTLEY_SM_EXPORT MosaicTextureProvider(HFCPtr<HIMMosaic>& mosaic);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE
