/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(EnergyPerformanceAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct EnergyPerformanceAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_EnergyPerformanceAspect, QuantityTakeoffAspect);

    private:
        double m_rating = 0.0f;

        friend struct EnergyPerformanceAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT EnergyPerformanceAspect();
        EnergyPerformanceAspect(double rating);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a EnergyPerformanceAspect
        //! @param[in]  rating  Rating
        //! @return  Created EnergyPerformanceAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static EnergyPerformanceAspectPtr Create(double rating);

        //! Query EnergyPerformanceAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query EnergyPerformanceAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only EnergyPerformanceAspect pointer from the element
        //! @param[in]  el  Element that contains a EnergyPerformanceAspect
        //! @return  Retrieved EnergyPerformanceAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static EnergyPerformanceAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a EnergyPerformanceAspect pointer from the element
        //! @param[in]  el  Element that contains a EnergyPerformanceAspect
        //! @return  Retrieved EnergyPerformanceAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static EnergyPerformanceAspectP GetP(Dgn::DgnElementR el);

        //! Get EnergyPerformanceAspect's rating
        //! @return  Rating
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetRating() const;

        //! Set EnergyPerformanceAspect's rating
        //! @param[in]  newRating  Rating
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetRating(double newRating);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
