/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/DGNModelGeoref.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


#include <system_error>

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
struct LocalTransform;
struct GCS;
struct TransfoMatrix;
struct TransfoModel;
struct Unit;
END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

using Bentley::ScalableMesh::GeoCoords::GCS;
using Bentley::ScalableMesh::GeoCoords::LocalTransform;
using Bentley::ScalableMesh::GeoCoords::TransfoModel;
using Bentley::ScalableMesh::GeoCoords::TransfoMatrix;
using Bentley::ScalableMesh::GeoCoords::Unit;

inline const Transform& ToBSITransform (const TransfoMatrix& transform)
    {
    return reinterpret_cast<const Transform&>(transform);
    }

inline const TransfoMatrix& FromBSITransform (const Transform& transform)
    {
    return reinterpret_cast<const TransfoMatrix&>(transform);
    }

BENTLEYSTM_EXPORT GCS                   GetBSIElementGCSFromRootPerspective    (DgnModelRefP            modelRef);

GCS                                     ReinterpretModelGCSFromRootPerspective (const GCS&              elementAsSeenFromModelGCS,
                                                                                DgnModelRefP            elementModelRefP);

BENTLEYSTM_EXPORT GCS                   GetModelMasterGCS                      (DgnModelRefP            modelRef);

BENTLEYSTM_EXPORT GCS                   GetModelActiveGCS                      (DgnModelRefP            modelRef);

Unit                                    GetModelGlobalUnit                     (DgnModelRefP            modelRef);

Unit                                    GetModelMasterUnit                     (DgnModelRefP            modelRef);

Unit                                    GetModelUOR                            (DgnModelRefP            modelRef);

double                                  GetModelDesignToMasterUnitScaleFactor  (DgnModelRefP            modelRef);

double                                  GetModelDesignToGlobalUnitScaleFactor  (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);
double                                  GetModelGlobalToDesignUnitScaleFactor  (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

double                                  GetSourceToTargetUnitScaleFactor       (const Unit&             source,
                                                                                const Unit&             target);

BENTLEYSTM_EXPORT double                GetModelDesignToGlobalUnitScaleFactor  (DgnModelRefP            modelRefP);
double                                  GetModelGlobalToDesignUnitScaleFactor  (DgnModelRefP            modelRefP);


BENTLEYSTM_EXPORT bool                  IsModelAttachedReprojected             (DgnModelRefP            modelRef);

BENTLEYSTM_EXPORT bool                  IsModelGeoreferenced                   (DgnModelRefP            modelRef);


BENTLEYSTM_EXPORT TransfoModel          GetModelGlobalToRootGlobalTransfoModelWReprojSupport
                                                                               (DgnModelRefP            modelRefP);

TransfoModel                            GetModelLocalToGlobalTransfoModel      (DgnModelRefP            modelRefP,
                                                                                const Unit&             localFrameUnit,
                                                                                const Unit&             globalFrameUnit);

TransfoModel                            GetModelGlobalToLocalTransfoModel      (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit,
                                                                                const Unit&             localFrameUnit);


BENTLEYSTM_EXPORT TransfoModel          GetModelDesignToGlobalTransfoModel     (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

TransfoModel                            GetModelGlobalToDesignTransfoModel     (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

TransfoMatrix                           GetModelDesignToGlobalTransfoMatrix    (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

TransfoMatrix                           GetModelGlobalToDesignTransfoMatrix    (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

BENTLEYSTM_EXPORT TransfoModel          GetModelDesignToGlobalTransfoModel     (DgnModelRefP            modelRefP);
BENTLEYSTM_EXPORT TransfoModel          GetModelGlobalToDesignTransfoModel     (DgnModelRefP            modelRefP);
BENTLEYSTM_EXPORT TransfoMatrix         GetModelDesignToGlobalTransfoMatrix    (DgnModelRefP            modelRefP);
TransfoMatrix                           GetModelGlobalToDesignTransfoMatrix    (DgnModelRefP            modelRefP);


END_BENTLEY_SCALABLEMESH_NAMESPACE
