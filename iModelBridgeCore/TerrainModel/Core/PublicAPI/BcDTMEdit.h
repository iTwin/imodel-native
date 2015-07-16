/*--------------------------------------------------------------------------------------+
|
|     $Source: src/unmanaged/DTM/civilDTMext/PublicAPI/BcDTMEdit.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "PublicAPI\bcdtmext.h"

CIVIL_TERRAINMODEL_TYPEDEFS(IBcDTMEdit)
CIVIL_TERRAINMODEL_TYPEDEFS(IBcDTMEditDyanmics)

ADD_BENTLEY_TYPEDEFS1 (Bentley::Civil::TerrainModel,IBcDTMEdit, BcDTMEdit, struct)

BEGIN_BENTLEY_CIVIL_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<IBcDTMEdit>                 BcDTMEditPtr;

struct IBcDTMEditDrawer
    {
    virtual StatusInt Draw (DTMFeatureType dtmFeatureType, DTMUserTag userTag, const DPoint3d* points, int numPoints) = 0;
    };

struct IBcDTMEdit : IRefCounted
{
public:
    enum SelectionState
        {
        None,
        Point,
        Line,
        Triangle,
        Feature,
        TrianglesByLine
        };
protected:
    virtual StatusInt _StartDynamics() = 0;
    virtual StatusInt _EndDynamics() = 0;

    virtual StatusInt _DrawDynamics (IBcDTMEditDrawer* drawer) = 0;
    virtual StatusInt _DrawSelection (IBcDTMEditDrawer* drawer) = 0;

    virtual void _ClearSelection () = 0;
    virtual StatusInt _SelectVertex (const DPoint3d& pt) = 0;
    virtual StatusInt _SelectLine (const DPoint3d& pt) = 0;
    virtual StatusInt _SelectTriangle (const DPoint3d& pt) = 0;
    virtual StatusInt _SelectFeature (const DPoint3d& pt, bool first) = 0;
    virtual DTMFeatureType _GetSelectedFeatureType () = 0;
    virtual StatusInt _SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures) = 0;
    virtual SelectionState _GetSelectionState() = 0;

    virtual StatusInt _AddVertex (const DPoint3d& pt, bool useZ = true) = 0;
    virtual StatusInt _MoveVertex (const DPoint3d& pt, bool updateZ = true) = 0;
    virtual StatusInt _DeleteVertex () = 0;
    virtual StatusInt _SwapLine () = 0;
    virtual StatusInt _DeleteLine () = 0;
    virtual StatusInt _DeleteFeature() = 0 ;
    virtual StatusInt _DeleteTriangle() = 0;
    virtual StatusInt _DeleteTrianglesByLine() = 0;


    virtual bool _CanDeleteVertex () = 0;
    virtual bool _CanDeleteLine () = 0;
    virtual bool _CanDeleteTriangle () = 0;
    virtual bool _CanSwapLine () = 0;
    virtual bool _CanDeleteInternalTriangles() = 0;
    virtual bool _CanDeleteTrianglesByLine () = 0;
    virtual void _SetCanDeleteInternalTriangles (bool value) = 0;

public:
    BCDTMEXT_EXPORT static BcDTMEditPtr Make (BcDTMP dtm);

    BCDTMEXT_EXPORT StatusInt StartDynamics();
    BCDTMEXT_EXPORT StatusInt EndDynamics();

    BCDTMEXT_EXPORT StatusInt DrawDynamics (IBcDTMEditDrawer* drawer);
    BCDTMEXT_EXPORT StatusInt DrawSelection (IBcDTMEditDrawer* drawer);

    BCDTMEXT_EXPORT void ClearSelection ();
    BCDTMEXT_EXPORT StatusInt SelectVertex (const DPoint3d& pt);
    BCDTMEXT_EXPORT StatusInt SelectLine (const DPoint3d& pt);
    BCDTMEXT_EXPORT StatusInt SelectTriangle (const DPoint3d& pt);
    BCDTMEXT_EXPORT StatusInt SelectFeature (const DPoint3d& pt, bool first);
    BCDTMEXT_EXPORT DTMFeatureType GetSelectedFeatureType ();
    BCDTMEXT_EXPORT StatusInt SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures = false);

    BCDTMEXT_EXPORT SelectionState GetSelectionState();

    BCDTMEXT_EXPORT StatusInt AddVertex (const DPoint3d& pt, bool useZ = true);
    BCDTMEXT_EXPORT StatusInt MoveVertex (const DPoint3d& pt, bool updateZ = true);

    BCDTMEXT_EXPORT StatusInt DeleteVertex ();
    BCDTMEXT_EXPORT StatusInt SwapLine ();
    BCDTMEXT_EXPORT StatusInt DeleteLine ();
    BCDTMEXT_EXPORT StatusInt DeleteFeature ();
    BCDTMEXT_EXPORT StatusInt DeleteTriangle();
    BCDTMEXT_EXPORT StatusInt DeleteTrianglesByLine ();

    BCDTMEXT_EXPORT bool CanDeleteVertex ();
    BCDTMEXT_EXPORT bool CanDeleteLine ();
    BCDTMEXT_EXPORT bool CanDeleteTriangle ();
    BCDTMEXT_EXPORT bool CanSwapLine ();
    BCDTMEXT_EXPORT bool CanDeleteTrianglesByLine ();

    BCDTMEXT_EXPORT bool CanDeleteInternalTriangles();
    BCDTMEXT_EXPORT void SetCanDeleteInternalTriangles (bool value);

};

END_BENTLEY_CIVIL_TERRAINMODEL_NAMESPACE
