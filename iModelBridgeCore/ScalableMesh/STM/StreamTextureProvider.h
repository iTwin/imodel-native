/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/ITextureProvider.h>
#include "ImagePPHeaders.h"
#include "RasterUtilities.h"


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct StreamTextureProvider : virtual public ITextureProvider
    {
    private:
        HGF2DExtent m_minExt;
        DRange3d m_totalExt;

    protected:

        virtual DPoint2d _GetMinPixelSize() override;

        virtual DRange2d _GetTextureExtent() override;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area) override;
    public:

        StreamTextureProvider(HFCPtr<HRARASTER>& textureSource, DRange3d totalExt);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE