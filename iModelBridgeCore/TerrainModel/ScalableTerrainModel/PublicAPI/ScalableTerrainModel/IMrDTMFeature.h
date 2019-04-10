/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct IMrDTMFeature;
typedef RefCountedPtr<IMrDTMFeature> IMrDTMFeaturePtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IMrDTMFeature : RefCountedBase
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

        BENTLEYSTM_EXPORT static IMrDTMFeaturePtr Create ();

        BENTLEYSTM_EXPORT static IMrDTMFeaturePtr CreateFor (const size_t& type, const DPoint2d* pointsPtr, const size_t& numPoints);

        BENTLEYSTM_EXPORT size_t   GetType() const;

        BENTLEYSTM_EXPORT size_t   GetSize() const;

        BENTLEYSTM_EXPORT DPoint2d GetPoint(size_t idx) const;

        BENTLEYSTM_EXPORT void     SetType (const size_t type);

        BENTLEYSTM_EXPORT void     AppendPoint (const DPoint2d& point);
    };

END_BENTLEY_MRDTM_NAMESPACE