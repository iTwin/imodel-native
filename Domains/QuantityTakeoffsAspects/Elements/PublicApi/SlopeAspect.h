/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(SlopeAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct SlopeAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SlopeAspect, QuantityTakeoffAspect);

    private:
        double m_slope = 0;

        friend struct SlopeAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT SlopeAspect();
        SlopeAspect(double slope);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a SlopeAspect
        //! @param[in]  slope  Slope
        //! @return  Created SlopeAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlopeAspectPtr Create(double slope);

        //! Query SlopeAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query SlopeAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only SlopeAspect pointer from the element
        //! @param[in]  el  Element that contains a SlopeAspect
        //! @return  Retrieved SlopeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlopeAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a SlopeAspect pointer from the element
        //! @param[in]  el  Element that contains a SlopeAspect
        //! @return  Retrieved SlopeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlopeAspectP GetP(Dgn::DgnElementR el);

        //! Get SlopeAspect's slope
        //! @return  Slope
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetSlope() const;

        //! Set SlopeAspect's slope
        //! @param[in]  newSlope  Slope
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetSlope(double newSlope);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
