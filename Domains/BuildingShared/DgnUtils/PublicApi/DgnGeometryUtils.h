/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Elonas.Seviakovas              03/2019
// GeometryUtils is a static class for doing various operations on geometry.
// This is also a wrapper for all parasolid related activity.
//=======================================================================================
struct DgnGeometryUtils
    {
    private:
        static Dgn::IBRepEntityPtr GetCutSheetBody(Dgn::IBRepEntityCR geometryToSlice, double elevation);

        static void ClearElementGeometry(Dgn::GeometricElement3dCR element);
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
        BUILDINGSHAREDDGNUTILS_EXPORT static void RotatePlacementXY(Placement3dR placement, double theta);

        //! Rotates placement around point by given angle in radians in XY plane
        //! @param[in/out] placement    placement to rotate
        //! @param[in] origin           point to rotate around
        //! @param[in] theta            angle in radians
        BUILDINGSHAREDDGNUTILS_EXPORT static void RotatePlacementAroundPointXY(Placement3dR placement, DPoint3d origin, double theta);

        //! Translates placement by given vector in XY plane
        //! @param[in/out] placement    placement to translate
        //! @param[in] translation      vector to translate by
        BUILDINGSHAREDDGNUTILS_EXPORT static void TranslatePlacementXY(Placement3dR placement, DVec3d translation);

        //! Translates placement by given vector
        //! @param[in/out] placement    placement to translate
        //! @param[in] translation      vector to translate by
        BUILDINGSHAREDDGNUTILS_EXPORT static void TranslatePlacement(Placement3dR placement, DVec3d translation);

        //! Finds angle in XY plane from placement
        //! @param[in] placement    placement with rotation angle
        //! @return                 rotation angle aroud z axis
        BUILDINGSHAREDDGNUTILS_EXPORT static double PlacementToAngleXY(Placement3d placement);

        //! Get's the cross section that is parallel to the XY plane at a given Z.
        //! @param[in] solid The body to get the cross section of
        //! @param[in] z     The Z coordinate of cross section
        //! @return the cross section of a given solid at a given Z.
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr GetXYCrossSection(Dgn::IBRepEntityCR solid, double z);

        //! Does the same thing as GetXYCrossSection but returns a BRepEntityPtr.
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::IBRepEntityPtr GetXYCrossSectionSheetBody(Dgn::IBRepEntityCR solid, double z);

        //! slice the body by Z elevations
        //! @param[out] slicedGeometry      solid slices paired with corresponding z elevations of the cuts
        //! @param[in]  geometryToSlice     geometry to slice
        //! @param[in]  zElevationVector    vector of Z elevations to slice at
        //! @return                         solid slices paired with corresponding bottom planes of the cuts
        BUILDINGSHAREDDGNUTILS_EXPORT static BentleyStatus SliceBodyByZElevations(bvector<bpair<Dgn::IBRepEntityPtr, double>>& slicedGeometry, Dgn::IBRepEntityCR geometryToSlice, bvector<double>& zElevationVector);

        //! Get slice of body limited by Z elevations
        //! @param[in]  geometryToSlice     geometry to slice
        //! @param[in]  bottomElevation     bottom elevation of slice. The result slice can have higher bottom elevation if the given elevation is not in range.
        //! @param[in]  topElevation        top elevation of slice. The result slice can have lower top elevation if the given elevation is not in range.
        //! @return                         solid slice of the geometryToSlice between given bottom and top elevations.
        //!                                 Returns sheet body when bottomElevation matches topElevation.
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::IBRepEntityPtr GetBodySlice(Dgn::IBRepEntityCR geometryToSlice, double bottomElevation, double topElevation);

        //! Cut off geometry below given elevation.
        //! @param[in]  geometryToSlice     geometry to slice.
        //! @param[in]  elevation           the lower bound of result geometry.
        //! @return                         solid slice of geometryToSlice with the geometry below given elevation cut off.
        //!                                 Returns sheet body when the given elevation is at the top of geometryToSlice.
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::IBRepEntityPtr GetUpwardSlice(Dgn::IBRepEntityCR geometryToSlice, double elevation);

        //! Cut off geometry above given elevation.
        //! @param[in]  geometryToSlice     geometry to slice.
        //! @param[in]  elevation           the upper bound of result geometry.
        //! @return                         solid slice of geometryToSlice with the geometry above given elevation cut off.
        //!                                 Returns sheet body when the given elevation is at the bottom of geometryToSlice.
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::IBRepEntityPtr GetDownwardSlice(Dgn::IBRepEntityCR geometryToSlice, double elevation);

        //! Gets spatial location element's base shape
        //! @param element                  element for which we want to get the base shape
        //! @return                         base shape curve vector
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr GetBaseShape(Dgn::GeometricElement3dCR element);

        //! Extracts spatial location element's bottom face shape
        //! @param element                  element for which we want to get the bottom face shape
        //! @return                         bottom face shape curve vector
        BUILDINGSHAREDDGNUTILS_EXPORT static CurveVectorPtr ExtractBottomFaceShape(Dgn::GeometricElement3dCR element);

        //---------------------------------------------------------------------------------------
        // Extrusion
        //---------------------------------------------------------------------------------------
        //! Initiates geometry with extrusion detail
        //! @param extrusion                extrusion geometry base
        //! @param pGeometryParameters      parameters for building extrusion geometry
        //! @param subCategoryId            subcategory id for extrusion geometry label
        //! @param detail                   extrusion detail that will be applied to geometry
        //! @param other                    additional geometry that will be applied to extrusion
        //! @param updatePlacementOrigin    change placement to 
        //! @return                         returns true if extrusion got initiated successfully
        BUILDINGSHAREDDGNUTILS_EXPORT static bool InitiateExtrusionGeometry(
            Dgn::GeometricElement3dR extrusion,
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            DgnExtrusionDetailCR detail,
            IGeometryPtr const &other = nullptr,
            bool updatePlacementOrigin = true);

        //! Initiates geometry with extrusion detail
        //! @param extrusion                extrusion geometry base
        //! @param pGeometryParameters      parameters for building extrusion geometry
        //! @param subCategoryId            subcategory id for extrusion geometry label
        //! @param base                     shape of extrusion
        //! @param height                   height of extrusion
        //! @param other                    additional geometry that will be applied to extrusion
        //! @param updatePlacementOrigin    change placement to 
        //! @return                         returns true if extrusion got initiated successfully
        BUILDINGSHAREDDGNUTILS_EXPORT static bool InitiateExtrusionGeometry(
            Dgn::GeometricElement3dR extrusion,
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            CurveVectorCPtr base,
            double height,
            IGeometryPtr const &other = nullptr,
            bool updatePlacementOrigin = true);

        //! Calculates and sets extrusion footprint area
        //! @param extrusion                extrusion geometry
        BUILDINGSHAREDDGNUTILS_EXPORT static void CalculateProperties(Dgn::GeometricElement3dR extrusion);

        //! Gets extrusion's extrusion detail
        //! @param[in]  elementWithExtrusion   geometric element that contains extrusion detail
        //! @param[out] extrusionDetail        extrusion detail of this extrusion
        //! @return                            true if there was no error in getting extrusion detail
        BUILDINGSHAREDDGNUTILS_EXPORT static bool GetDgnExtrusionDetail(Dgn::GeometricElement3dCR elementWithExtrusion, DgnExtrusionDetail& extrusionDetail);

        //! Sets extrusion's extrusion detail
        //! @param[in]  element             geometric element that will have it's extrusion detail set
        //! @param[in]  extrusionDetail     new extrusion detail for this extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static void SetDgnExtrusionDetail(
            Dgn::GeometricElement3dR element, 
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            DgnExtrusionDetail const &extrusionDetail);

        //! Sets extrusion's height
        //! @param[in]  newHeight           new height for the extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static void SetExtrusionHeight(
            Dgn::GeometricElement3dR extrusion, 
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            double newHeight);

        //! Gets extrusion's height
        //! @returns height of extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static double  GetExtrusionHeight(Dgn::GeometricElement3dCR extrusion);

        //! Returns base plane of this extrusion
        //! @return base plane of this extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static DPlane3d GetExtrusionBasePlane(Dgn::GeometricElement3dCR extrusion);

        //! Returns top plane of this extrusion
        //! @return top plane of this extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static DPlane3d GetExtrusionTopPlane(Dgn::GeometricElement3dCR extrusion);
        
        //! Returns geometry builder for specified extrusion
        //! @param extrusion                extrusion geometry for which a geometry builder will be created
        //! @param pGeometryParams          geometry parameters for the geometry builder
        //! @param pOrigin                  origin point for the new geometry
        //! @return geometry builder
        BUILDINGSHAREDDGNUTILS_EXPORT static Dgn::GeometryBuilderPtr GetExtrusionGeometryBuilder(Dgn::GeometricElement3dCR extrusion, Dgn::Render::GeometryParamsCP pGeometryParams, DPoint3dP pOrigin);

        BUILDINGSHAREDDGNUTILS_EXPORT static void SetExtrusionName(
            Dgn::GeometricElement3dR element,
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            Utf8CP name);

        //! Updates this extrusion's geometry with given base curve vector
        //! @param extrusion               extrusion that will have it's geometry updated
        //! @param pGeometryParams         geometry parameters for new geometry
        //! @param subCategoryId           subcategory id for extrusion geometry label
        //! @param base                    new base curve vector for this extrusion
        //! @param updatePlacementOrigin   true if origin of this extrusion should be updated
        //! @return                            true if there was no error in updating extrusion's geometry
        BUILDINGSHAREDDGNUTILS_EXPORT static bool UpdateExtrusionGeometry(
            Dgn::GeometricElement3dR extrusion, 
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            CurveVectorCPtr base, 
            bool updatePlacementOrigin = true);

        //! Updates this extrusion's geometry with given extrusion detail
        //! @param extrusion               extrusion that will have it's geometry updated
        //! @param pGeometryParams         geometry parameters for new geometry
        //! @param subCategoryId           subcategory id for extrusion geometry label
        //! @param extrusiondetail         new extrusion detail for this extrusion
        //! @param otherGeometry           any additional geometry for this extrusion
        //! @param updatePlacementOrigin   true if origin of this extrusion should be updated
        //! @return                        true if there was no error in updating extrusion's geometry
        BUILDINGSHAREDDGNUTILS_EXPORT static bool UpdateExtrusionGeometry(
            Dgn::GeometricElement3dR extrusion, 
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            DgnExtrusionDetailCR extrusionDetail, 
            IGeometryPtr const &otherGeometry = nullptr, 
            bool updatePlacementOrigin = true);
    
        //! Get the specified plane from the extrusion
        //! @param[in] extrusion           extrusion geometry
        //! @param[in] planeId             plane id: 0 - base; 1 - top
        //! @param[out] planeOut           extracted plane destination
        //! @return                        true if there was no error in getting extrusion's plane
        BUILDINGSHAREDDGNUTILS_EXPORT static bool GetExtrusionPlane(Dgn::GeometricElement3dCR extrusion, int geometryId, DPlane3dR planeOut);

        //! Strech extrusion to specified plane
        //! @param extrusion               extrusion geometry
        //! @param pGeometryParams         geometry parameters for new streched geometry
        //! @param subCategoryId           subcategory id for streched geometry label
        //! @param targetPlane             target plane that extrusion will be streched to
        //! @return                        true if there was no error while streching extrusion
        BUILDINGSHAREDDGNUTILS_EXPORT static bool StretchExtrusionToPlane(
            Dgn::GeometricElement3dR extrusion, 
            Dgn::Render::GeometryParamsCP pGeometryParameters,
            Dgn::DgnSubCategoryId subCategoryId,
            DPlane3dR targetPlane);

        //! Strech extrusion to specified plane
        //! @param builder                 geometry builder to which new label will be appended
        //! @param profile                 new geometry label shape
        //! @param name                    new geometry label name
        //! @param subCategoryId           new geometry label subcategory id
        BUILDINGSHAREDDGNUTILS_EXPORT static void AppendExtrusionLabel(Dgn::GeometryBuilderPtr& builder, CurveVectorCR profile, Utf8CP name, Dgn::DgnSubCategoryId subCategoryId);
    };

END_BUILDING_SHARED_NAMESPACE