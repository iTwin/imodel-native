/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(WallAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

enum class WallType : int
    {
    Movable = 0,
    Parapet = 1,
    Partitioning = 2,
    PlumbingWall = 3,
    Shear = 4,
    SolidWall = 5,
    Standard = 6,
    Polygonal = 7,
    ElementedWall = 8,
    UserDefined = 9,
    NotDefined = 10,
    };

struct WallAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_WallAspect, QuantityTakeoffAspect);

    private:
        WallType m_type = WallType::NotDefined;

        friend struct WallAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT WallAspect();
        WallAspect(WallType type);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a WallAspect
        //! @param[in]  type  Embedment depth
        //! @return  Created WallAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static WallAspectPtr Create(WallType type);

        //! Query WallAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query WallAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only WallAspect pointer from the element
        //! @param[in]  el  Element that contains a WallAspect
        //! @return  Retrieved WallAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static WallAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a WallAspect pointer from the element
        //! @param[in]  el  Element that contains a WallAspect
        //! @return  Retrieved WallAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static WallAspectP GetP(Dgn::DgnElementR el);

        //! Get WallAspect's type
        //! @return  Wall type
        QUANTITYTAKEOFFSASPECTS_EXPORT WallType GetType() const;

        //! Set WallAspect's wall type
        //! @param[in]  newtype  Wall type
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetType(WallType newtype);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
