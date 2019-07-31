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

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(SurfaceAreaAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct SurfaceAreaAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SurfaceAreaAspect, QuantityTakeoffAspect);

    private:
        double m_grossSurfaceArea = 0.0f;
        double m_netSurfaceArea = 0.0f;

        friend struct SurfaceAreaAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT SurfaceAreaAspect();
        SurfaceAreaAspect(double grossSurfaceArea, double netSurfaceArea);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a SurfaceAreaAspect
        //! @param[in]  grossSurfaceArea  Gross surface area
        //! @param[in]  netSurfaceArea  Net surface area
        //! @return  Created SurfaceAreaAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static SurfaceAreaAspectPtr Create(double grossSurfaceArea, double netSurfaceArea);

        //! Query SurfaceAreaAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query SurfaceAreaAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only SurfaceAreaAspect pointer from the element
        //! @param[in]  el  Element that contains a SurfaceAreaAspect
        //! @return  Retrieved SurfaceAreaAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SurfaceAreaAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a SurfaceAreaAspect pointer from the element
        //! @param[in]  el  Element that contains a SurfaceAreaAspect
        //! @return  Retrieved SurfaceAreaAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SurfaceAreaAspectP GetP(Dgn::DgnElementR el);

        //! Get SurfaceAreaAspect's gross surface area
        //! @return  GrossSurfaceArea
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetGrossSurfaceArea() const;

        //! Set SurfaceAreaAspect's gross surface area
        //! @param[in]  newGrossSurfaceArea  Gross surface area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetGrossSurfaceArea(double newGrossSurfaceArea);

        //! Get SurfaceAreaAspect's net surface area
        //! @return  Net surface area
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetNetSurfaceArea() const;

        //! Set SurfaceAreaAspect's net surface area
        //! @param[in]  newNetSurfaceArea  Net surface area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetNetSurfaceArea(double newNetSurfaceArea);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
