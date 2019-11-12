/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(SideAreasAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct SideAreasAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SideAreasAspect, QuantityTakeoffAspect);

    public:
        DEFINE_POINTER_SUFFIX_TYPEDEFS(SideAreas);

        struct SideAreas {
            double rightSide = 0.0f;
            double leftSide = 0.0f;
            double top = 0.0f;
            double bottom = 0.0f;
        };

    private:
        SideAreas m_netAreas;
        SideAreas m_grossAreas;

        friend struct SideAreasAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT SideAreasAspect();
        SideAreasAspect(SideAreasCR netAreas, SideAreasCR grossAreas);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a SideAreasAspect
        //! @param[in]  netAreas  Object containing net areas of right, left, top and bottom sides
        //! @param[in]  grossAreas  Object containing gross areas of right, left, top and bottom sides
        //! @return  Created SideAreasAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static SideAreasAspectPtr Create(SideAreasCR netAreas, SideAreasCR grossAreas);

        //! Query SideAreasAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query SideAreasAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only SideAreasAspect pointer from the element
        //! @param[in]  el  Element that contains a SideAreasAspect
        //! @return  Retrieved SideAreasAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SideAreasAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a SideAreasAspect pointer from the element
        //! @param[in]  el  Element that contains a SideAreasAspect
        //! @return  Retrieved SideAreasAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SideAreasAspectP GetP(Dgn::DgnElementR el);

        //! Get net areas of right, left, top and bottom sides
        //! @return  Side net areas object 
        QUANTITYTAKEOFFSASPECTS_EXPORT SideAreasCP GetNetAreas() const;

        //! Get gross areas of right, left, top and bottom sides
        //! @return  Side gross areas object  
        QUANTITYTAKEOFFSASPECTS_EXPORT SideAreasCP GetGrossAreas() const;

        //! Set net areas of right, left, top and bottom sides
        //! @param[in]  newThickness  Side net areas 
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetNetAreas(SideAreasCR newAreas);

        //! Set right side net area
        //! @param[in]  area  New right side net area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetRightSideNetArea(double area);

        //! Set left side net area
        //! @param[in]  area  New left side net area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetLeftSideNetArea(double area);

        //! Set top net area
        //! @param[in]  area  New top net area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetTopNetArea(double area);

        //! Set bottom net area
        //! @param[in]  area  New bottom net area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetBottomNetArea(double area);

        //! Set gross areas of right, left, top and bottom sides
        //! @param[in]  newThickness  Side gross areas 
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetGrossAreas(SideAreasCR newAreas);

        //! Set right side gross area
        //! @param[in]  area  New right side gross area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetRightSideGrossArea(double area);

        //! Set left side gross area
        //! @param[in]  area  New left side gross area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetLeftSideGrossArea(double area);

        //! Set top gross area
        //! @param[in]  area  New top gross area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetTopGrossArea(double area);

        //! Set bottom gross area
        //! @param[in]  area  New bottom gross area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetBottomGrossArea(double area);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
