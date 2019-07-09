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

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(VolumeAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct VolumeAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_VolumeAspect, QuantityTakeoffAspect);

    private:
        double m_grossVolume = 0.0f;
        double m_netVolume = 0.0f;

        friend struct VolumeAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT VolumeAspect();
        VolumeAspect(double grossVolume, double netVolume);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a VolumeAspect
        //! @param[in]  grossVolume  Gross volume
        //! @param[in]  netVolume  Net volume
        //! @return  Created VolumeAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static VolumeAspectPtr Create(double grossVolume, double netVolume);

        //! Query VolumeAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query VolumeAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only VolumeAspect pointer from the element
        //! @param[in]  el  Element that contains a VolumeAspect
        //! @return  Retrieved VolumeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static VolumeAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a VolumeAspect pointer from the element
        //! @param[in]  el  Element that contains a VolumeAspect
        //! @return  Retrieved VolumeAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static VolumeAspectP GetP(Dgn::DgnElementR el);

        //! Get VolumeAspect's gross volume
        //! @return  GrossVolume
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetGrossVolume() const;

        //! Set VolumeAspect's gross volume
        //! @param[in]  newGrossVolume  Gross volume
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetGrossVolume(double newGrossVolume);

        //! Get VolumeAspect's net volume
        //! @return  Net volume
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetNetVolume() const;

        //! Set VolumeAspect's net volume
        //! @param[in]  newNetVolume  Net volume
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetNetVolume(double newNetVolume);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
