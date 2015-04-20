/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnItem.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Base class for ElementItem handlers (which follow the singleton pattern).
//! Many handler methods take the ElementItemKey type so as not to require the loading
//! of a ElementItem into memory.
//! @note Domain developers are expected to subclass ElementItemHandler
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElementItemHandler : DgnDomain::Handler
{
    HANDLER_DECLARE_MEMBERS (DGN_CLASSNAME_ElementItem, ElementItemHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

//__PUBLISH_SECTION_END__
    friend struct ElementGeomTableHandler;

//__PUBLISH_SECTION_START__
protected:
    //! @note Called from InsertGeom and AssignGeometry
    BentleyStatus InsertElementGeomUsesParts(DgnDbR, DgnElementId, PhysicalGeometryCR);

public:

    virtual ElementItemHandlerP _ToElementItemHandler() override {return this;}     //!< dynamic_cast this Handler to a ElementItemHandler

    DGNPLATFORM_EXPORT DgnClassId GetItemClassId(DgnDbR db);

    DGNPLATFORM_EXPORT ElementItemHandler* GetItemHandler(DgnDbR db, DgnClassId itemClassId);

    //! Set the geometry and placement information for the input DgnElement from PhysicalGeometry.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom (DgnElementR eeh, PhysicalGeometryCR geom, DPoint3dCR origin, YawPitchRollAnglesCR angles);

    //! Set the geometry and placement information for the input DgnElement from the supplied ElementGeometry.
    //! @param[in,out] eeh The element to modify.
    //! @param[in] geom The element geometry.
    //! @param[in] subCategory Optional DgnSubCategoryId to control the appearance and visibility of this geometry. if invalid, the default sub-category for the element's category is used.
    //! @param[in] origin Optional placement origin. When not nullptr, geometry coordinates are assumed to be relative to this origin instead of world.
    //! @param[in] angles Optional placement angles. Pass nullptr for identity rotation or specific angles. Ignored when origin is nullptr.
    //! @return SUCCESS if element geometry could be set.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, ElementGeometryCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a CurveVector.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, CurveVectorCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a ICurvePrimitive.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, ICurvePrimitiveCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a ISolidPrimitive.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, ISolidPrimitiveCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a MSBsplineSurface.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, MSBsplineSurfaceCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a PolyfaceQuery.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, PolyfaceQueryCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

    //! Set the geometry and placement information for the input DgnElement from a ISolidKernelEntity.
    DGNPLATFORM_EXPORT BentleyStatus SetElementGeom(DgnElementR eeh, ISolidKernelEntityCR geom, DgnSubCategoryId subCategoryId=DgnSubCategoryId(), DPoint3dCP origin=nullptr, YawPitchRollAnglesCP angles=nullptr);

};

END_BENTLEY_DGNPLATFORM_NAMESPACE
