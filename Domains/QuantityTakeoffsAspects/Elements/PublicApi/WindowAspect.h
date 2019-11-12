/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(WindowAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

enum class WindowType : int
    {
    Casement = 0,
    SingleHung = 1,
    DoubleHung = 2,
    Awning = 3,
    Hopper = 4,
    Sliding = 5,
    };

struct WindowAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_WindowAspect, QuantityTakeoffAspect);

    private:
        WindowType m_type = WindowType::Casement;

        friend struct WindowAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT WindowAspect();
        WindowAspect(WindowType type);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a WindowAspect
        //! @param[in]  type  Embedment depth
        //! @return  Created WindowAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static WindowAspectPtr Create(WindowType type);

        //! Query WindowAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query WindowAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only WindowAspect pointer from the element
        //! @param[in]  el  Element that contains a WindowAspect
        //! @return  Retrieved WindowAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static WindowAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a WindowAspect pointer from the element
        //! @param[in]  el  Element that contains a WindowAspect
        //! @return  Retrieved WindowAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static WindowAspectP GetP(Dgn::DgnElementR el);

        //! Get WindowAspect's type
        //! @return  Window type
        QUANTITYTAKEOFFSASPECTS_EXPORT WindowType GetType() const;

        //! Set WindowAspect's wall type
        //! @param[in]  newtype  Window type
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetType(WindowType newtype);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
