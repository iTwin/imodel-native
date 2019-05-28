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

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(SlabAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct SlabAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_SlabAspect, QuantityTakeoffAspect);

    private:
        DPoint2d m_slabDirection;

        friend struct SlabAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT SlabAspect();
        SlabAspect(DPoint2dCR slabDirection);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a SlabAspect
        //! @param[in]  slabDirection  Slab direction
        //! @return  Created SlabAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlabAspectPtr Create(DPoint2dCR slabDirection);

        //! Query SlabAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query SlabAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only SlabAspect pointer from the element
        //! @param[in]  el  Element that contains a SlabAspect
        //! @return  Retrieved SlabAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlabAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a SlabAspect pointer from the element
        //! @param[in]  el  Element that contains a SlabAspect
        //! @return  Retrieved SlabAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static SlabAspectP GetP(Dgn::DgnElementR el);

        //! Get SlabAspect's slab direction
        //! @return  Slab direction
        QUANTITYTAKEOFFSASPECTS_EXPORT DPoint2dCP GetSlabDirection() const;

        //! Set SlabAspect's slab direction
        //! @param[in]  newSlabDirection  Slab direction
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetSlabDirection(DPoint2dCR newSlabDirection);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
