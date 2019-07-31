/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(StairsAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct StairsAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_StairsAspect, QuantityTakeoffAspect);

    private:
        uint32_t m_numberOfRisers = 0;
        double m_riserHeight = 0.0f;

        friend struct StairsAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT StairsAspect();
        StairsAspect(uint32_t numberOfRisers, double riserHeight);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a StairsAspect
        //! @param[in]  numberOfRisers  Number of risers
        //! @param[in]  riserHeight  Riser height
        //! @return  Created StairsAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static StairsAspectPtr Create(uint32_t numberOfRisers, double riserHeight);

        //! Query StairsAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query StairsAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only StairsAspect pointer from the element
        //! @param[in]  el  Element that contains a StairsAspect
        //! @return  Retrieved StairsAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static StairsAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a StairsAspect pointer from the element
        //! @param[in]  el  Element that contains a StairsAspect
        //! @return  Retrieved StairsAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static StairsAspectP GetP(Dgn::DgnElementR el);

        //! Get StairsAspect's number of risers
        //! @return  Number of risers
        QUANTITYTAKEOFFSASPECTS_EXPORT uint32_t GetNumberOfRisers() const;

        //! Set StairsAspect's number Of risers
        //! @param[in]  newNumberOfRisers  Number of risers
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetNumberOfRisers(uint32_t newNumberOfRisers);

        //! Get StairsAspect's riser height
        //! @return  Riser height
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetRiserHeight() const;

        //! Set StairsAspect's riser height
        //! @param[in]  newRiserHeight  Riser height
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetRiserHeight(double newRiserHeight);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
