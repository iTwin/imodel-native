/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/PublicAPI/TMEditor.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel\TerrainModel.h>

TERRAINMODEL_TYPEDEFS (ITMEditor)
TERRAINMODEL_TYPEDEFS (ITMEditorDyanmics)

//ADD_BENTLEY_TYPEDEFS1(BENTLEY_NAMESPACE_NAME::TerrainModel, ITMEditor, TMEditor, struct)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<ITMEditor>                 TMEditorPtr;

struct ITMEditorDrawer
    {
    virtual StatusInt Draw (DTMFeatureType dtmFeatureType, DTMUserTag userTag, const DPoint3d* points, int numPoints) = 0;
    };

struct ITMEditor : IRefCounted
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

    virtual StatusInt _DrawDynamics (ITMEditorDrawer* drawer) = 0;
    virtual StatusInt _DrawSelection (ITMEditorDrawer* drawer) = 0;

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
    BENTLEYDTM_EXPORT static TMEditorPtr Make (BcDTMP dtm);

    BENTLEYDTM_EXPORT StatusInt StartDynamics();
    BENTLEYDTM_EXPORT StatusInt EndDynamics();

    BENTLEYDTM_EXPORT StatusInt DrawDynamics (ITMEditorDrawer* drawer);
    BENTLEYDTM_EXPORT StatusInt DrawSelection (ITMEditorDrawer* drawer);

    BENTLEYDTM_EXPORT void ClearSelection ();
    BENTLEYDTM_EXPORT StatusInt SelectVertex (const DPoint3d& pt);
    BENTLEYDTM_EXPORT StatusInt SelectLine (const DPoint3d& pt);
    BENTLEYDTM_EXPORT StatusInt SelectTriangle (const DPoint3d& pt);
    BENTLEYDTM_EXPORT StatusInt SelectFeature (const DPoint3d& pt, bool first);
    BENTLEYDTM_EXPORT DTMFeatureType GetSelectedFeatureType ();
    BENTLEYDTM_EXPORT StatusInt SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures = false);

    BENTLEYDTM_EXPORT SelectionState GetSelectionState();

    BENTLEYDTM_EXPORT StatusInt AddVertex (const DPoint3d& pt, bool useZ = true);
    BENTLEYDTM_EXPORT StatusInt MoveVertex (const DPoint3d& pt, bool updateZ = true);

    BENTLEYDTM_EXPORT StatusInt DeleteVertex ();
    BENTLEYDTM_EXPORT StatusInt SwapLine ();
    BENTLEYDTM_EXPORT StatusInt DeleteLine ();
    BENTLEYDTM_EXPORT StatusInt DeleteFeature ();
    BENTLEYDTM_EXPORT StatusInt DeleteTriangle();
    BENTLEYDTM_EXPORT StatusInt DeleteTrianglesByLine ();

    BENTLEYDTM_EXPORT bool CanDeleteVertex ();
    BENTLEYDTM_EXPORT bool CanDeleteLine ();
    BENTLEYDTM_EXPORT bool CanDeleteTriangle ();
    BENTLEYDTM_EXPORT bool CanSwapLine ();
    BENTLEYDTM_EXPORT bool CanDeleteTrianglesByLine ();

    BENTLEYDTM_EXPORT bool CanDeleteInternalTriangles();
    BENTLEYDTM_EXPORT void SetCanDeleteInternalTriangles (bool value);

};

END_BENTLEY_TERRAINMODEL_NAMESPACE
