/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableMesh/IScalableMeshFeature.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct ScalableMeshFeature : public IScalableMeshFeature
    {
    private:
        size_t           m_type;
        vector<DPoint2d> m_points;
        uint32_t           m_refCount;   

    protected:
       
        virtual size_t   _GetType() const override;

        virtual size_t   _GetSize() const override;

        virtual DPoint2d _GetPoint(size_t idx) const override;

        virtual void     _SetType (const size_t type) override;

        virtual void     _AppendPoint (const DPoint2d& point) override;

    public:

        ScalableMeshFeature ();
        ScalableMeshFeature (const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints);
        ~ScalableMeshFeature ();
       
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
