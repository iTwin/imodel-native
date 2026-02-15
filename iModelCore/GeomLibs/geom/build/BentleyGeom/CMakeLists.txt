cmake_minimum_required(VERSION 3.20)
project(iTwinGeom CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

# GEOMLIBS_ROOT is the GeomLibs directory (parent of geom/, PublicAPI/, etc.)
get_filename_component(GEOMLIBS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

set(SOURCES
    # src
    ${GEOMLIBS_ROOT}/geom/src/DeprecatedFunctions.cpp
    # BRepEntity
    ${GEOMLIBS_ROOT}/geom/src/BRepEntity/FaceAttachment.cpp
    ${GEOMLIBS_ROOT}/geom/src/BRepEntity/TopologyPrimitive.cpp
    # CurvePrimitive
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CCBlend.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CCFillet.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveConstraint.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveCurveApproach.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveCurveIntersectRotated.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveCurveIntersection.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveCurveProcessor.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveGapOptions.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveLocationDetail.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveLocationDetailPair.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurvePlaneIntersection.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurvePrimitiveBsplineCurve.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurvePrimitiveId.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurvePrimitivePair.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurvePrimitivePartialCurve.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveTopologyId.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveVector.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveVectorWithDistanceIndex.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/CurveVectorWithXIndex.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/FacetEdgeLocateionDetail.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/GeometryNode.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/GeometryValidator.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/ICurvePrimitive.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/ICurvePrimitiveProcessor.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/IGeometryPtr.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/PathLocationDetail.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_arc.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_child.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_line.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_linestring.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_offsetCurve.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cp_spiralConstruction.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_AreaMoments.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_WireMoments.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_areaBooleans.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_areaFixup.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_chainFixup.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_clone.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_hatch.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_inout.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_offset.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_ops.cpp
    ${GEOMLIBS_ROOT}/geom/src/CurvePrimitive/cv_properties.cpp
    # IntegerTypes
    ${GEOMLIBS_ROOT}/geom/src/IntegerTypes/BSIRect.cpp
    # PlacementOnEarth
    ${GEOMLIBS_ROOT}/geom/src/PlacementOnEarth/Placement.cpp
    # SolidPrimitive
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/BoxFaces.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/SolidLocationDetail.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/SolidPrimitive.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_AreaMoments.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_ConstructiveFrame.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_CurveIntersection.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_ExtractFace.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_FaceIndices.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_Moments.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_Range.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_UVToXYZ.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_clone.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_closestPoint.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_getSection.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_isClosedVolume.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_maxUVLength.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_parameterizationSign.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_same.cpp
    ${GEOMLIBS_ROOT}/geom/src/SolidPrimitive/sp_silhouette.cpp
    # bezier
    ${GEOMLIBS_ROOT}/geom/src/bezier/bezeval.cpp
    ${GEOMLIBS_ROOT}/geom/src/bezier/bezierDPoint4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/bezier/bezroot.cpp
    ${GEOMLIBS_ROOT}/geom/src/bezier/bezrootConics.cpp
    ${GEOMLIBS_ROOT}/geom/src/bezier/implicitbezier.cpp
    ${GEOMLIBS_ROOT}/geom/src/bezier/ruledBezier.cpp
    # bspline
    ${GEOMLIBS_ROOT}/geom/src/bspline/BCurveSegment.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/BSurfPatch.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/Bspline1d.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/BsplineCurveFit.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/BsplineParam.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/LocalRange.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurveCreate.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurvePtr.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve_ByBezier.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve_Modify.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve_getSet.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve_ruledSurface.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineCurve_transform.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineSurface.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineSurfaceLofting.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineSurface_AddSurfaces.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSBsplineSurface_getSet.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/MSInterpolationCurve.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/TensorProducts.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/akima.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspCubicFit.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspLocal.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspSmoothPolyface.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bsp_mxspline.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bsp_offset.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspcciStubs.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspconv.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspcurv.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspcurvcurv.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspdcurv.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspdsurf.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspfit.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspicurv.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspmisc.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bsppolyface.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bsprsurf.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspsurf.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bspsurfacetrim.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/bsputil.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/sortutil.cpp
    ${GEOMLIBS_ROOT}/geom/src/bspline/tristrip.cpp
    # funcs
    ${GEOMLIBS_ROOT}/geom/src/funcs/Angle.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/BSIQuadrature.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/BinaryRangeHeap.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/ConvexHull.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/DPoint3dOps.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/DPoint3dOpsRangeHeap.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/DoubleOps.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/InsertPointsInPolylineContext.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/PolygonOps.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/PolylineIntersections.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/PolylineOffset.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/PolylineOps.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/TernaryRangeSort.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/TriDiagonalSolver.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/TriangleRangeIntersection.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/XYBucketSearch.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/XYZRangeTree.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/YawPitchRollAnglesInDegrees.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/analyticroots.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/ccctangent.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/cluster.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/eigensys3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/ellipticintegrals.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/linalg.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/memfuncs.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/newton.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/polygon3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/polyline3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/quadeqn.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/rotations.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/simpson.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/standardViews.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/svd.cpp
    ${GEOMLIBS_ROOT}/geom/src/funcs/trigfuncs.cpp
    # memory
    ${GEOMLIBS_ROOT}/geom/src/memory/UnionFind.cpp
    ${GEOMLIBS_ROOT}/geom/src/memory/embeddedintarray.cpp
    # mtg
    ${GEOMLIBS_ROOT}/geom/src/mtg/MTGGraph.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/MtgFacetPlanarMerge.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgbase1.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgchain.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgedgestar.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgfacet.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgmarkset.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgmask.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgstitch.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/jmdl_mtgswap.cpp
    # mtgSmoothNormals.cpp excluded — references removed DVec3d methods (stale)
    ${GEOMLIBS_ROOT}/geom/src/mtg/mtgSubdivision.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/mtgflowlines.cpp
    ${GEOMLIBS_ROOT}/geom/src/mtg/mtgprint.cpp
    # polyface
    ${GEOMLIBS_ROOT}/geom/src/polyface/AlternatingConvexClipTree.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/BlockedVector.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/ClipPlane.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/ClipPlaneSet.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/FacetFaceData.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/FacetLocationDetail.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/FacetOptions.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/IPolyfaceConstruction_Add.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/IPolyfaceConstruction_Dispatchers.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/IndexedClipEdge.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/MeshAnnotationVector.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/Polyface.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceAdd.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceAddSolidPrimitive.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceAddTriangulation.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceAuxData.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceClip.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceClip1.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceClipToPlaneSets.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceClusterDecimation.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceCoordinateMap.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceEdgeChain.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceFastNormalBuilder.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceIndexedStitch.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceIntersection.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceLightWeightBuilder.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceOps.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfacePartition.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceQuery.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceQueryCarrier.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceStitch.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceVectors.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceVisitor.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/PolyfaceVisitorSearch.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/RgbFactor.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/TaggedNumericData.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/TransformedDRange3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pfBoundary.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pfRegularPolyhedron.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_SearchContext.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_boreVolume.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_cutFill.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_drag.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_drapeLinestring.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_heal.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_indexedHeapRangeSearch.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_integrals.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_purgeDuplicates.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_singleSheetCutFill.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_sweepMeshToSolid.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_unifyIndices.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/pf_vertexCollapse.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/polyfaceAddNormals.cpp
    ${GEOMLIBS_ROOT}/geom/src/polyface/polyfaceToMTG.cpp
    # regions
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_extractCurveVector.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_fixface.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_gaps.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_geom.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_header.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_intersections.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_inwardparity.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_loops.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_merge.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_merge2.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_rangeTree.cpp
    ${GEOMLIBS_ROOT}/geom/src/regions/rg_windingNumber.cpp
    # rimsbs
    ${GEOMLIBS_ROOT}/geom/src/rimsbs/rimsbs_context.cpp
    ${GEOMLIBS_ROOT}/geom/src/rimsbs/rimsbs_eval.cpp
    ${GEOMLIBS_ROOT}/geom/src/rimsbs/rimsbs_intersect.cpp
    ${GEOMLIBS_ROOT}/geom/src/rimsbs/rimsbs_intersect1.cpp
    # structs
    ${GEOMLIBS_ROOT}/geom/src/structs/DBilinearPatch3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/DPoint3dDoubleArrays.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/DSegment1d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/DTriangle3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/Polynomials.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/catenary.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/dconic4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/dmap4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/drange1d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/dray3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/dspiral2dbase.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/graphicspoint.cpp
    # refmethods
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/biarc.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refOperatorOverload.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdellipse3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdmap4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdmatrix4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdplane3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdpoint2d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdpoint3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdpoint4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdrange2d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdrange3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdrange3dArrays.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdray3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdsegment3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdsegment4d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdvec2d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refdvec3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/reffrange3d.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refrotmatrix.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refrotmatrixInit.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refrotmatrixInvert.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refrotmatrixMultiply.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/refrotmatrixQuery.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/reftransform.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/reftransformInvert.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/reftransformIsXXX.cpp
    ${GEOMLIBS_ROOT}/geom/src/structs/cpp/refmethods/reftransformMultiply.cpp
    # vu (sources live at GeomLibs/vu/src/, NOT geom/src/vu/)
    ${GEOMLIBS_ROOT}/vu/src/vu.cpp
    ${GEOMLIBS_ROOT}/vu/src/vucoord.cpp
    ${GEOMLIBS_ROOT}/vu/src/vusmooth.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuRefineSurface.cpp
    ${GEOMLIBS_ROOT}/vu/src/vusubset.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuarray.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumod2.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuconvex.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuaddedges.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuunion.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuprint.cpp
    ${GEOMLIBS_ROOT}/vu/src/vupointin.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumerge.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumerge2.cpp
    ${GEOMLIBS_ROOT}/vu/src/vutriang.cpp
    ${GEOMLIBS_ROOT}/vu/src/vureg.cpp
    ${GEOMLIBS_ROOT}/vu/src/vustack.cpp
    ${GEOMLIBS_ROOT}/vu/src/vucluster.cpp
    ${GEOMLIBS_ROOT}/vu/src/vudangler.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuflood.cpp
    ${GEOMLIBS_ROOT}/vu/src/vugrid.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumaskops.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumem.cpp
    ${GEOMLIBS_ROOT}/vu/src/vumulticlip.cpp
    ${GEOMLIBS_ROOT}/vu/src/VuOps.cpp
    ${GEOMLIBS_ROOT}/vu/src/vupoly.cpp
    ${GEOMLIBS_ROOT}/vu/src/vupoly_bsplineBoundaryLoops.cpp
    ${GEOMLIBS_ROOT}/vu/src/VuPositionDetail.cpp
    ${GEOMLIBS_ROOT}/vu/src/vuSearch.cpp
    ${GEOMLIBS_ROOT}/vu/src/PolygonArray.cpp
    ${GEOMLIBS_ROOT}/vu/src/PolygonArrayOps.cpp
    ${GEOMLIBS_ROOT}/vu/src/XYVisibilitySplitter.cpp
    # vu/DrapeGraph
    ${GEOMLIBS_ROOT}/vu/src/DrapeGraph/DrapeGraph.cpp
    ${GEOMLIBS_ROOT}/vu/src/DrapeGraph/VuDebug.cpp
    # init
    ${CMAKE_CURRENT_SOURCE_DIR}/BentleyGeom_init.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinGeom STATIC ${SOURCES})
else()
    bb_add_library(iTwinGeom SHARED ${SOURCES})
endif()

# PCH
target_precompile_headers(iTwinGeom PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/bsibasegeomPCH.cpp")

target_include_directories(iTwinGeom PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${GEOMLIBS_ROOT}/PublicAPI"
    "${GEOMLIBS_ROOT}/geom/src"
    "${GEOMLIBS_ROOT}/geom/src/regions"
    "${GEOMLIBS_ROOT}/geom/src/bspline"
    "${GEOMLIBS_ROOT}/geom/src/structs"
    "${GEOMLIBS_ROOT}/geom/src/funcs"
    "${GEOMLIBS_ROOT}/PrivateAPI"
)

target_compile_definitions(iTwinGeom PRIVATE jmdlgeom_internal)

bb_target_include_subpart_api(iTwinGeom Geom)
bb_target_include_subpart_api(iTwinGeom Bentley)

bb_find_subpart_library(JSONCPP_LIB iTwinJsonCpp)
bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
target_link_libraries(iTwinGeom PRIVATE
    ${JSONCPP_LIB}
    ${BENTLEY_LIB}
)

# Resource delivery (replaces GeomLibs/prewire.mke)
bb_deliver_public_api("${GEOMLIBS_ROOT}/PublicAPI"
    Geom PlacementOnEarth GeomJsonWireFormat Mtg Vu Regions GeomSerialization
)

