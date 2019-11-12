/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(MaterialAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct MaterialAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_MaterialAspect, QuantityTakeoffAspect);

    private:
        Utf8String m_material = "";
        double m_materialDensity = 0.0f;
        double m_weight = 0.0f;

        friend struct MaterialAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT MaterialAspect();
        MaterialAspect(Utf8StringCR material, double materialDensity, double weight);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a MaterialAspect
        //! @param[in]  material  Material
        //! @param[in]  materialDensity  Material density
        //! @param[in]  weight  Weight
        //! @return  Created MaterialAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static MaterialAspectPtr Create(Utf8StringCR material, double materialDensity, double weight);

        //! Query MaterialAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query MaterialAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only MaterialAspect pointer from the element
        //! @param[in]  el  Element that contains a MaterialAspect
        //! @return  Retrieved MaterialAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static MaterialAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a MaterialAspect pointer from the element
        //! @param[in]  el  Element that contains a MaterialAspect
        //! @return  Retrieved MaterialAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static MaterialAspectP GetP(Dgn::DgnElementR el);

        //! Get MaterialAspect's material
        //! @return  Material
        QUANTITYTAKEOFFSASPECTS_EXPORT Utf8StringCP GetMaterial() const;

        //! Set MaterialAspect's material
        //! @param[in]  newMaterial  Material
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetMaterial(Utf8StringCR newMaterial);

        //! Get MaterialAspect's material density
        //! @return  Material density
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetMaterialDensity() const;

        //! Set MaterialAspect's material density
        //! @param[in]  newMaterialDensity  Material density
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetMaterialDensity(double newMaterialDensity);

        //! Get MaterialAspect's weight
        //! @return  Weight
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetWeight() const;

        //! Set MaterialAspect's weight
        //! @param[in]  newWeight  Weight
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetWeight(double newWeight);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
