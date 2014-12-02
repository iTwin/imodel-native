/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/ElementCreateHelpers.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <DgnPlatform/DgnCore/DgnFileIOApi.h>
#include <UnitTests/BackDoor/DgnProject/BackDoor.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

namespace DPT_CreateElement
    {
    enum ElementType   {Line,
                        Cone,
                        Curve,
                        nEllipse,
                        nArc,
                        LineString,
                        PointString,
                        nDimension,
                        Mesh,
                        nSharedCell,
                        nShape,
                        Complex,
                        BSpline};
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateMesh00 (EditElementHandleR eeh, DgnModelR model);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds      08/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool MatchedIndexVectors (BlockedVectorIntR vector0, BlockedVectorIntR vector1);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedCellSetUp (EditElementHandleR m_eeh, DgnModelR model, bool is3d);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateDef (DgnModelR model, bool is3d);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void InitPoints (DPoint3dP points, size_t size);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void CreateElement(DPT_CreateElement::ElementType type, EditElementHandleR m_eeh, DgnModelR model, bool is3d);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KyleDeeds       08/09
+---------------+---------------+---------------+---------------+---------------+------*/
void isPerp(DVec3dR a, DPoint3dR b);

/*---------------------------------------------------------------------------------**//**
* @param[in] indices 0-terminated indices.  All faces must have terminator.
* @para [in] count total index count.
* @param [in] padToMaxFaceSize If true, pad all faces to max size. If false, copy with terminators.
* @return value to set as numPerFace in mesh (i.e. 0 if 0-terminated)
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PushIndices (bvector<int>&dest, int *indices, size_t count, bool padToMaxFaceSize);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateCube (bool variableSize, double x0, double y0, double z0, double a, double b, double c);

/*---------------------------------------------------------------------------------**//**
* Sweep a polygon to a mesh.
*   orient basePoints CCW with sweepVector pointing at eye.
* @bsimethod                                                    EarlinLutz      04/09
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr CreateSweep (DPoint3d basePoints[], int numBasePoints, DVec3dR sweepVector);

// Create a mesh on first quadrant integer grid points.
PolyfaceHeaderPtr CreateUnitGrid (size_t numXPoint, size_t numYPoint, bool triangulate);

END_DGNDB_UNIT_TESTS_NAMESPACE