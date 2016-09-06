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

using BENTLEY_NAMESPACE_NAME::ScalableMesh::GeoCoords::GCS;
using BENTLEY_NAMESPACE_NAME::ScalableMesh::GeoCoords::LocalTransform;
using BENTLEY_NAMESPACE_NAME::ScalableMesh::GeoCoords::TransfoModel;
using BENTLEY_NAMESPACE_NAME::ScalableMesh::GeoCoords::TransfoMatrix;
using BENTLEY_NAMESPACE_NAME::ScalableMesh::GeoCoords::Unit;

inline const Transform& ToBSITransform (const TransfoMatrix& transform)
    {
    return reinterpret_cast<const Transform&>(transform);
    }

inline const TransfoMatrix& FromBSITransform (const Transform& transform)
    {
    return reinterpret_cast<const TransfoMatrix&>(transform);
    }

BENTLEY_SM_EXPORT GCS                   GetBSIElementGCSFromRootPerspective    (DgnModelRefP            modelRef);

GCS                                     ReinterpretModelGCSFromRootPerspective (const GCS&              elementAsSeenFromModelGCS,
                                                                                DgnModelRefP            elementModelRefP);

BENTLEY_SM_EXPORT GCS                   GetModelMasterGCS                      (DgnModelRefP            modelRef);

BENTLEY_SM_EXPORT GCS                   GetModelActiveGCS                      (DgnModelRefP            modelRef);

Unit                                    GetModelGlobalUnit                     (DgnModelRefP            modelRef);

Unit                                    GetModelMasterUnit                     (DgnModelRefP            modelRef);

Unit                                    GetModelUOR                            (DgnModelRefP            modelRef);

BENTLEY_SM_EXPORT bool                  IsModelAttachedReprojected             (DgnModelRefP            modelRef);

BENTLEY_SM_EXPORT bool                  IsModelGeoreferenced                   (DgnModelRefP            modelRef);



TransfoModel                            GetModelLocalToGlobalTransfoModel      (DgnModelRefP            modelRefP,
                                                                                const Unit&             localFrameUnit,
                                                                                const Unit&             globalFrameUnit);



TransfoMatrix                           GetModelDesignToGlobalTransfoMatrix    (DgnModelRefP            modelRefP,
                                                                                const Unit&             globalFrameUnit);

END_BENTLEY_SCALABLEMESH_NAMESPACE
