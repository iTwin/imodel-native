/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/PublicApi/DgnGeometryUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
// GeometryUtils is a static class for doing various operations on geometry.
// This is also a wrapper for all parasolid related activity.
//=======================================================================================
struct DgnGeometryUtils
    {
    public:
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus    CreateBodyFromGeometricPrimitive(Dgn::IBRepEntityPtr& out, Dgn::GeometricPrimitiveCPtr primitive, bool assignIds = false);
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus    SliceBodyByPlanes(bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>& slicedGeometry, Dgn::IBRepEntityCR geometryToSlice,
                                                                            CurveVectorCR cuttingPlaneProfile, double sliceHeight);
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr   ExtractXYProfileFromSolid(Dgn::IBRepEntityCR solid, CurveVectorPtr* pTopProfile = nullptr);
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus    GetGeometricPrimitivesFromGeometricElement(bvector<Dgn::GeometricPrimitivePtr>& geometricPrimitivesOut, Dgn::GeometricElementCPtr geoElement);
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus    GetIBRepEntitiesFromGeometricElement(bvector<Dgn::IBRepEntityPtr>& brepsOut, Dgn::GeometricElementCPtr geoElement);
    
        //! transforms a curvevector onto zero plane
        //! @param[in]  solid  solid to extract top/bottom profiles from
        //! @param[out] bottomProfile bottom profile transformed onto zero plane
        //! @param[out] topProfile top profile transformed onto zero plane
        //! @return BSISUCCESS if profiles were computed
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus     GetTopBottomProfilesOnZeroPlane(Dgn::IBRepEntityCR solid, CurveVectorPtr& bottomProfile, CurveVectorPtr& topProfile);

        //! Rotates placement by given angle in radians in XY plane
        //! @param[in/out] placement    placement to rotate
        //! @param[in] theta            angle in radians
        BUILDINGSHAREDDGNUTILS_EXPORT static void RotatePlacementXY(Dgn::Placement3dR placement, double theta);

        //! Rotates placement around point by given angle in radians in XY plane
        //! @param[in/out] placement    placement to rotate
        //! @param[in] origin           point to rotate around
        //! @param[in] theta            angle in radians
        BUILDINGSHAREDDGNUTILS_EXPORT static void RotatePlacementAroundPointXY(Dgn::Placement3dR placement, DPoint3d origin, double theta);

        //! Translates placement by given vector in XY plane
        //! @param[in/out] placement    placement to translate
        //! @param[in] translation      vector to translate by
        BUILDINGSHAREDDGNUTILS_EXPORT static void TranslatePlacementXY(Dgn::Placement3dR placement, DVec3d translation);

        //! Translates placement by given vector
        //! @param[in/out] placement    placement to translate
        //! @param[in] translation      vector to translate by
        BUILDINGSHAREDDGNUTILS_EXPORT static void TranslatePlacement(Dgn::Placement3dR placement, DVec3d translation);

        //! Finds angle in XY plane from placement
        //! @param[in] placement    placement with rotation angle
        //! @return                 rotation angle aroud z axis
        BUILDINGSHAREDDGNUTILS_EXPORT static double PlacementToAngleXY(Dgn::Placement3d placement);

        //! Get's the cross section that is parallel to the XY plane at a given Z.
        //! @param[in] solid The body to get the cross section of
        //! @param[in] z     The Z coordinate of cross section
        //! @return the cross section of a given solid at a given Z.
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr GetXYCrossSection(Dgn::IBRepEntityCR solid, double z);

        //! slice the body by Z elevations
        //! @param[out] slicedGeometry      solid slices paired with corresponding z elevations of the cuts
        //! @param[in]  geometryToSlice     geometry to slice
        //! @param[in]  zElevationVector    vector of Z elevations to slice at
        //! @return                         solid slices paired with corresponding bottom planes of the cuts
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus SliceBodyByZElevations(bvector<bpair<Dgn::IBRepEntityPtr, double>>& slicedGeometry, Dgn::IBRepEntityCR geometryToSlice, bvector<double>& zElevationVector);

        //! Gets extrusion's extrusion detail
        //! @param[out] extDetail   extrusion detail of this extrusion
        //! @return                 true if there was no error in getting extrusion detail
        BUILDINGSHAREDDGNUTILS_EXPORT static bool GetDgnExtrusionDetail(Dgn::SpatialLocationElementCR extrusionThatIsSolid, DgnExtrusionDetail& extDetail);

        //! Gets spatial location element's base shape
        //! @param extrusionThatIsSolid   element for which we want to get the base shape
        //! @return                       base shape curve vector
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr GetBaseShape(Dgn::SpatialLocationElementCR extrusionThatIsSolid);

        //! Extracts spatial location element's bottom face shape
        //! @param extrusionThatIsSolid   element for which we want to get the bottom face shape
        //! @return                       bottom face shape curve vector
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr ExtractBottomFaceShape(Dgn::SpatialLocationElementCR extrusionThatIsSolid);

    };

END_BUILDING_SHARED_NAMESPACE