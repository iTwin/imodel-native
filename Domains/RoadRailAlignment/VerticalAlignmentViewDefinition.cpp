/*--------------------------------------------------------------------------------------+
|
|     $Source: VerticalAlignmentViewDefinition.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"

HANDLER_DEFINE_MEMBERS(VerticalAlignmentViewDefinitionHandler)


//---------------------------------------------------------------------------------------
// @bsimethod
// Adjusts the AspectRatio by changing the y-axis only
//---------------------------------------------------------------------------------------
void VerticalAlignmentViewDefinition::_AdjustAspectRatio(double windowAspect)
    {
    static bool s_useSuper = true;
    if (s_useSuper)
        {
        T_Super::_AdjustAspectRatio(windowAspect);
        return;
        }

    //&&AG needswork. Platform only calls this code when Fitting the view, but not when changing the aspect ratio skew
    // In that last case, it is handled by the DgnViewport::_AdjustAspectRatio code.
    DVec3d extents = GetExtents();
    const double viewAspect = extents.x / extents.y;

    windowAspect *= GetAspectRatioSkew();

    if (fabs(1.0 - (viewAspect / windowAspect)) < 1.0e-9)
        return;
    
    const DVec3d oldDelta = extents;
    if (viewAspect > windowAspect)
        extents.y = extents.x / windowAspect;

    DPoint3d origin = GetOrigin();
    DPoint3d newOrigin;
    GetRotation().Multiply(&newOrigin, &origin, 1);
    newOrigin.x += ((oldDelta.x - extents.x) / 2.0);
    newOrigin.y += ((oldDelta.y - extents.y) / 2.0);
    GetRotation().MultiplyTranspose(origin, newOrigin);
    SetOrigin(origin);
    SetExtents(extents);
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void VerticalAlignmentViewDefinition::_SetRotation(RotMatrixCR rot)
    {
    if (m_allowRotation)
        m_rotation = rot;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus VerticalAlignmentViewDefinition::_OnInsert()
    {
    BeAssert(!"VerticalAlignmentViewDefinition is not expected to be inserted in db");
    return DgnDbStatus::NotEnabled;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus VerticalAlignmentViewDefinition::_OnUpdate(DgnElementCR original)
    {
    BeAssert(!"VerticalAlignmentViewDefinition is not expected to be updated in db");
    return DgnDbStatus::NotEnabled;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
VerticalAlignmentViewDefinition::VerticalAlignmentViewDefinition(SpatialViewDefinitionR def):
    T_Super(*def.GetDefinitionModel(), def.GetName(), def.GetCategorySelector(), def.GetDisplayStyle3d(), def.GetModelSelector()),
    m_allowRotation(true)
    {
    m_elementId = DgnElementId(static_cast<uint64_t>(1));
    m_classId = QueryClassId(def.GetDgnDb());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        08/2017
//---------------------------------------------------------------------------------------
VerticalAlignmentViewDefinitionPtr VerticalAlignmentViewDefinition::Create(Dgn::SpatialViewDefinitionCR def)
    {
    SpatialViewDefinitionPtr defPtr = def.MakeCopy<SpatialViewDefinition>();
    if (!defPtr.IsValid())
        return nullptr;

    return new VerticalAlignmentViewDefinition(*defPtr);
    }
