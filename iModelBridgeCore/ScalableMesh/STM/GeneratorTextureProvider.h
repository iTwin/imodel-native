/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ScalableMesh/IScalableMeshTextureGenerator.h>
#include <ScalableMesh/ITextureProvider.h>


BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


struct GeneratorTextureProvider : virtual public ITextureProvider
    {
    private:
        IScalableMeshTextureGeneratorPtr m_generator;
        double m_minPixelSize;
        DRange3d m_extent;
        BeFileName m_dir;

    protected:

        virtual DPoint2d _GetMinPixelSize() override;

        virtual DRange2d _GetTextureExtent() override;

        virtual StatusInt _GetTextureForArea(bvector<uint8_t>& texData, int width, int height, DRange2d& area) override;
    public:

        GeneratorTextureProvider(IScalableMeshTextureGeneratorPtr& generator, DRange3d coveredExtent, double minPixelSize, BeFileName tempDirectory);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE
