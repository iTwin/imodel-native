/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IScalableMeshFeature;
typedef RefCountedPtr<IScalableMeshFeature> IScalableMeshFeaturePtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IScalableMeshFeature : RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/

    /*__PUBLISH_CLASS_VIRTUAL__*/

    protected:                         

        virtual size_t   _GetType() const = 0;

        virtual size_t   _GetSize() const = 0;

        virtual DPoint2d _GetPoint(size_t idx) const = 0;

        virtual void     _SetType (const size_t type) = 0;

        virtual void     _AppendPoint (const DPoint2d& point) = 0;


    /*__PUBLISH_SECTION_START__*/
    public:

        BENTLEYSTM_EXPORT static IScalableMeshFeaturePtr Create ();

        BENTLEYSTM_EXPORT static IScalableMeshFeaturePtr CreateFor (const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints);

        BENTLEYSTM_EXPORT size_t   GetType() const;

        BENTLEYSTM_EXPORT size_t   GetSize() const;

        BENTLEYSTM_EXPORT DPoint2d GetPoint(size_t idx) const;

        BENTLEYSTM_EXPORT void     SetType (const size_t type);

        BENTLEYSTM_EXPORT void     AppendPoint (const DPoint2d& point);
    };

END_BENTLEY_SCALABLEMESH_NAMESPACE
