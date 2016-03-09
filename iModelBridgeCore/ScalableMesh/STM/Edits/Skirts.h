#pragma once
#include <Bentley/bvector.h>
#include <Geom/GeomApi.h>
#include <ScalableMesh/IScalableMesh.h>
#include "DifferenceSet.h"
#include <TerrainModel\TerrainModel.h>
#include <array>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class SkirtBuilder
    {
    private:
        DTMPtr m_dtm;
    public:
        SkirtBuilder(BcDTMPtr& dtmP);
       void BuildSkirtMesh(bvector<PolyfaceHeaderPtr>& meshParts, bvector<bvector<DPoint3d>>& targetLines);
    };
END_BENTLEY_SCALABLEMESH_NAMESPACE
