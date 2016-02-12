/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/DGNModelGeoref.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

BENTLEYSTM_EXPORT bool                  IsModelAttachedReprojected             (DgnModelRefP            modelRef);

BENTLEYSTM_EXPORT bool                  IsModelGeoreferenced                   (DgnModelRefP            modelRef);



TransfoModel                            GetModelLocalToGlobalTransfoModel      (DgnModelRefP            modelRefP,
                                                                                const Unit&             localFrameUnit,
                                                                                const Unit&             globalFrameUnit);



TransfoMatrix                           GetModelDesignToGlobalTransfoMatrix    (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

END_BENTLEY_SCALABLEMESH_NAMESPACE
