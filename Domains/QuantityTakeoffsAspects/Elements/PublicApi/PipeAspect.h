/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(PipeAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct PipeAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_PipeAspect, QuantityTakeoffAspect);

    private:
        Utf8String m_schedule = "";
        double m_thickness = 0.0f;
        double m_length = 0.0f;
        double m_diameter = 0.0f;

        friend struct PipeAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT PipeAspect();
        PipeAspect(Utf8StringCR schedule, double thickness, double length, double diameter);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a PipeAspect
        //! @param[in]  schedule  Schedule
        //! @param[in]  thickness  Thickness
        //! @param[in]  length  Length
        //! @param[in]  diameter  Diameter
        //! @return  Created PipeAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static PipeAspectPtr Create(Utf8StringCR schedule, double thickness, double length, double diameter);

        //! Query PipeAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query PipeAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only PipeAspect pointer from the element
        //! @param[in]  el  Element that contains a PipeAspect
        //! @return  Retrieved PipeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PipeAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a PipeAspect pointer from the element
        //! @param[in]  el  Element that contains a PipeAspect
        //! @return  Retrieved PipeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PipeAspectP GetP(Dgn::DgnElementR el);

        //! Get PipeAspect's schedule
        //! @return  Schedule
        QUANTITYTAKEOFFSASPECTS_EXPORT Utf8StringCP GetSchedule() const;

        //! Set PipeAspect's schedule
        //! @param[in]  newSchedule  Schedule
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetSchedule(Utf8StringCR newSchedule);

        //! Get PipeAspect's thickness
        //! @return  Thickness
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetThickness() const;

        //! Set PipeAspect's thickness
        //! @param[in]  newThickness  Thickness
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetThickness(double newThickness);

        //! Get PipeAspect's length
        //! @return  Length
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetLength() const;

        //! Set PipeAspect's length
        //! @param[in]  newLength  Length
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetLength(double newLength);

        //! Get PipeAspect's diameter
        //! @return  Diameter
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetDiameter() const;

        //! Set PipeAspect's diameter
        //! @param[in]  newDiameter  Diameter
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetDiameter(double newDiameter);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
