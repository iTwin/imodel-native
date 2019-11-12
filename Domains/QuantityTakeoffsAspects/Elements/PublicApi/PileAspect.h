/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(PileAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct PileAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_PileAspect, QuantityTakeoffAspect);

    private:
        double m_embedmentDepth = 0;

        friend struct PileAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT PileAspect();
        PileAspect(double embedmentDepth);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a PileAspect
        //! @param[in]  embedmentDepth  Embedment depth
        //! @return  Created PileAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static PileAspectPtr Create(double embedmentDepth);

        //! Query PileAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query PileAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only PileAspect pointer from the element
        //! @param[in]  el  Element that contains a PileAspect
        //! @return  Retrieved PileAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PileAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a PileAspect pointer from the element
        //! @param[in]  el  Element that contains a PileAspect
        //! @return  Retrieved PileAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static PileAspectP GetP(Dgn::DgnElementR el);

        //! Get PileAspect's embedment depth
        //! @return  Embedment depth
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetEmbedmentDepth() const;

        //! Set PileAspect's embedment depth
        //! @param[in]  newEmbedmentDepth  Embedment depth
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetEmbedmentDepth(double newEmbedmentDepth);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
