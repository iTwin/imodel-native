/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(FoundationAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

enum class FoundationType : int
    {
    MatFoundation = 0,
    Pile = 1,
    PileCap = 2,
    SpreadFooting = 3,
    StripFooting = 4,
    };

struct FoundationAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_FoundationAspect, QuantityTakeoffAspect);

    private:
        FoundationType m_type = FoundationType::MatFoundation;

        friend struct FoundationAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT FoundationAspect();
        FoundationAspect(FoundationType type);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a FoundationAspect
        //! @param[in]  type  Embedment depth
        //! @return  Created FoundationAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static FoundationAspectPtr Create(FoundationType type);

        //! Query FoundationAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query FoundationAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only FoundationAspect pointer from the element
        //! @param[in]  el  Element that contains a FoundationAspect
        //! @return  Retrieved FoundationAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static FoundationAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a FoundationAspect pointer from the element
        //! @param[in]  el  Element that contains a FoundationAspect
        //! @return  Retrieved FoundationAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static FoundationAspectP GetP(Dgn::DgnElementR el);

        //! Get FoundationAspect's type
        //! @return  Foundation type
        QUANTITYTAKEOFFSASPECTS_EXPORT FoundationType GetType() const;

        //! Set FoundationAspect's wall type
        //! @param[in]  newtype  Foundation type
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetType(FoundationType newtype);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
