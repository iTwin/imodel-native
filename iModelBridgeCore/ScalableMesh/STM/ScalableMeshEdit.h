/*--------------------------------------------------------------------------------------+
|       $Date: 2016/08/23 10:33:32 $
|     $Author:Elenie.Godzaridis $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <ScalableMesh/IScalableMeshEdit.h>
#include <ScalableMesh/IScalableMesh.h>
#include "ImagePPHeaders.h"
#include "SMMeshIndex.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

class ScalableMeshEdit : public IScalableMeshEdit
    {
    private:

        SMMeshIndex<DPoint3d, DRange3d>* m_smIndex;

    protected:
        virtual int _RemoveWithin(ClipVectorCP clipPlaneSet, const bvector<IScalableMeshNodePtr>& priorityNodes)  override;

        virtual void _SmoothNode(const DPlane3d& sourceGeometry, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node) override;

        virtual void _SmoothNode(const DPoint3d& center, double radius, const DVec3d& direction, double height, const bvector<size_t>& targetedIndices, IScalableMeshNodePtr& node) override;

        virtual void _Smooth(const DPlane3d& sourceGeometry) override;

    public:
        ScalableMeshEdit(SMMeshIndex<DPoint3d, DRange3d>* smIndex);
        ~ScalableMeshEdit();

        static ScalableMeshEdit* Create(SMMeshIndex<DPoint3d, DRange3d>* smIndex);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE