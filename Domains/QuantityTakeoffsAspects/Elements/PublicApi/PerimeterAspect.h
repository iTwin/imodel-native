/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(PerimeterAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct PerimeterAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_PerimeterAspect, QuantityTakeoffAspect);

    private:
        double m_perimeter = 0;

        friend struct PerimeterAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT PerimeterAspect();
        PerimeterAspect(double perimeter);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a PerimeterAspect
        //! @param[in]  perimeter  Perimeter
        //! @return  Created PerimeterAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static PerimeterAspectPtr Create(double perimeter);

        //! Query PerimeterAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query PerimeterAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only PerimeterAspect pointer from the element
        //! @param[in]  el  Element that contains a PerimeterAspect
        //! @return  Retrieved PerimeterAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PerimeterAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a PerimeterAspect pointer from the element
        //! @param[in]  el  Element that contains a PerimeterAspect
        //! @return  Retrieved PerimeterAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PerimeterAspectP GetP(Dgn::DgnElementR el);

        //! Get PerimeterAspect's perimeter
        //! @return  Perimeter
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetPerimeter() const;

        //! Set PerimeterAspect's perimeter
        //! @param[in]  newPerimeter  Perimeter
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetPerimeter(double newPerimeter);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
