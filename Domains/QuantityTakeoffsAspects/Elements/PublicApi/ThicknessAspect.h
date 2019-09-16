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

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(ThicknessAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct ThicknessAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_ThicknessAspect, QuantityTakeoffAspect);

    private:
        double m_thickness = 0.0f;

        friend struct ThicknessAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT ThicknessAspect();
        ThicknessAspect(double thickness);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a ThicknessAspect
        //! @param[in]  thickness  Thickness
        //! @return  Created ThicknessAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static ThicknessAspectPtr Create(double thickness);

        //! Query ThicknessAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query ThicknessAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only ThicknessAspect pointer from the element
        //! @param[in]  el  Element that contains a ThicknessAspect
        //! @return  Retrieved ThicknessAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ThicknessAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a ThicknessAspect pointer from the element
        //! @param[in]  el  Element that contains a ThicknessAspect
        //! @return  Retrieved ThicknessAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ThicknessAspectP GetP(Dgn::DgnElementR el);

        //! Get ThicknessAspect's thickness 
        //! @return  Thickness 
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetThickness() const;

        //! Set ThicknessAspect's thickness 
        //! @param[in]  newThickness  Thickness 
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetThickness(double newThickness);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
