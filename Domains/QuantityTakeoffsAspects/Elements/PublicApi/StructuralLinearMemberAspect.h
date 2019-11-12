/*--------------------------------------------------------------------------------------+length
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(StructuralLinearMemberAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

enum class StructuralFramingType : int
{
    Beam = 0,
    Column = 1,
    VerticalBrace = 2,
    HorizontalBrace = 3,
    Pier = 4,
    Pile = 5,
    Purlin = 6,
    Cladding = 7
};

struct StructuralLinearMemberAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_StructuralLinearMemberAspect, QuantityTakeoffAspect);

    private:
        double m_crossSectionalArea = 0.0f;
        Utf8String m_sectionName = "";
        StructuralFramingType m_type = StructuralFramingType::Beam;

        friend struct StructuralLinearMemberAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT StructuralLinearMemberAspect();
        StructuralLinearMemberAspect(double crossSectionalArea, Utf8StringCR sectionName, StructuralFramingType type);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a StructuralLinearMemberAspect
        //! @param[in]  crossSectionalArea  Cross sectional area
        //! @param[in]  sectionName  Section name
        //! @param[in]  type  Type
        //! @return  Created StructuralLinearMemberAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static StructuralLinearMemberAspectPtr Create(double crossSectionalArea, Utf8StringCR sectionName, StructuralFramingType type);

        //! Query StructuralLinearMemberAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query StructuralLinearMemberAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only StructuralLinearMemberAspect pointer from the element
        //! @param[in]  el  Element that contains a StructuralLinearMemberAspect
        //! @return  Retrieved StructuralLinearMemberAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static StructuralLinearMemberAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a StructuralLinearMemberAspect pointer from the element
        //! @param[in]  el  Element that contains a StructuralLinearMemberAspect
        //! @return  Retrieved StructuralLinearMemberAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static StructuralLinearMemberAspectP GetP(Dgn::DgnElementR el);

        //! Get StructuralLinearMemberAspect's cross sectional area
        //! @return  Cross sectional area
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetCrossSectionalArea() const;

        //! Set StructuralLinearMemberAspect's cross sectional area
        //! @param[in]  newCrossSectionalArea  Cross sectional area
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetCrossSectionalArea(double newCrossSectionalArea);

        //! Get StructuralLinearMemberAspect's section name
        //! @return  Section name
        QUANTITYTAKEOFFSASPECTS_EXPORT Utf8StringCP GetSectionName() const;

        //! Set StructuralLinearMemberAspect's section name
        //! @param[in]  newSectionName  Section name
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetSectionName(Utf8StringCR newSectionName);

        //! Get StructuralLinearMemberAspect's type
        //! @return  Type
        QUANTITYTAKEOFFSASPECTS_EXPORT StructuralFramingType GetType() const;

        //! Set StructuralLinearMemberAspect's type
        //! @param[in]  newType  Type
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetType(StructuralFramingType newType);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
