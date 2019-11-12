/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(DoorAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

enum class DoorType : int
    {
    Flush = 0,
    FullGlass = 1,
    HalfGlass = 2,
    TwoLite = 3,
    NarrowView = 4,
    View = 5,
    LouverFull = 6,
    LouverBottom = 7,
    LouverTop = 8,
    HalfGlassWithLouver = 9,
    NarrowViewWithLouver = 10,
    };

struct DoorAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_DoorAspect, QuantityTakeoffAspect);

    private:
        DoorType m_type = DoorType::Flush;

        friend struct DoorAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT DoorAspect();
        DoorAspect(DoorType type);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a DoorAspect
        //! @param[in]  type  Embedment depth
        //! @return  Created DoorAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static DoorAspectPtr Create(DoorType type);

        //! Query DoorAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query DoorAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only DoorAspect pointer from the element
        //! @param[in]  el  Element that contains a DoorAspect
        //! @return  Retrieved DoorAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static DoorAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a DoorAspect pointer from the element
        //! @param[in]  el  Element that contains a DoorAspect
        //! @return  Retrieved DoorAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static DoorAspectP GetP(Dgn::DgnElementR el);

        //! Get DoorAspect's type
        //! @return  Door type
        QUANTITYTAKEOFFSASPECTS_EXPORT DoorType GetType() const;

        //! Set DoorAspect's wall type
        //! @param[in]  newtype  Door type
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetType(DoorType newtype);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
