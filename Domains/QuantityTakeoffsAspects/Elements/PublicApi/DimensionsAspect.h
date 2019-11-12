/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/Bentley.h>
#include <ECDb/ECDb.h>
#include <QuantityTakeoffsAspects/Domain/QuantityTakeoffsAspectsMacros.h>
#include "QuantityTakeoffAspect.h"

QUANTITYTAKEOFFSASPECTS_REFCOUNTED_PTR_AND_TYPEDEFS(DimensionsAspect)

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

struct DimensionsAspect : QuantityTakeoffAspect
    {
    DGNASPECT_DECLARE_MEMBERS(QUANTITYTAKEOFFSASPECTS_SCHEMA_NAME, QUANTITYTAKEOFFSASPECTS_CLASS_DimensionsAspect, QuantityTakeoffAspect);

    private:
        double m_length = 0.0f;
        double m_width = 0.0f;
        double m_height = 0.0f;

        friend struct DimensionsAspectHandler;

        QUANTITYTAKEOFFSASPECTS_EXPORT DimensionsAspect();
        DimensionsAspect(double length, double width, double height);

    protected:
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _UpdateProperties(Dgn::DgnElementCR, const BeSQLite::EC::ECCrudWriteToken*);
        QUANTITYTAKEOFFSASPECTS_EXPORT virtual Dgn::DgnDbStatus _LoadProperties(Dgn::DgnElementCR);

        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _GetPropertyValue(ECN::ECValueR, Utf8CP, Dgn::PropertyArrayIndex const&) const override;
        QUANTITYTAKEOFFSASPECTS_EXPORT Dgn::DgnDbStatus _SetPropertyValue(Utf8CP, ECN::ECValueCR, Dgn::PropertyArrayIndex const&) override;
    public:
        //! Create a DimensionsAspect
        //! @param[in]  length  Length
        //! @param[in]  width  Width
        //! @param[in]  height  Height
        //! @return  Created DimensionsAspect
        QUANTITYTAKEOFFSASPECTS_EXPORT static DimensionsAspectPtr Create(double length, double width, double height);

        //! Query DimensionsAspect's ECClassId from the database
        //! @param[in]  db  Reference to the database
        //! @return  Valid ECClassId or an invalid one if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassId QueryECClassId(Dgn::DgnDbR db);

        //! Query DimensionsAspect's ECClass from the database
        //! @param[in]  db  Reference to the database
        //! @return  Retrieved ECClass or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static ECN::ECClassCP QueryECClass(Dgn::DgnDbR db);

        //! Retrieve a read-only DimensionsAspect pointer from the element
        //! @param[in]  el  Element that contains a DimensionsAspect
        //! @return  Retrieved DimensionsAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static DimensionsAspectCP GetCP(Dgn::DgnElementCR el);

        //! Retrieve a DimensionsAspect pointer from the element
        //! @param[in]  el  Element that contains a DimensionsAspect
        //! @return  Retrieved DimensionsAspect or a nullptr if not found.
        QUANTITYTAKEOFFSASPECTS_EXPORT static DimensionsAspectP GetP(Dgn::DgnElementR el);

        //! Get DimensionsAspect's length
        //! @return  Length
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetLength() const;

        //! Set DimensionsAspect's length
        //! @param[in]  newLength  Length
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetLength(double newLength);

        //! Get DimensionsAspect's width
        //! @return  Width
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetWidth() const;

        //! Set DimensionsAspect's width
        //! @param[in]  newWidth  Width
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetWidth(double newWidth);

        //! Get DimensionsAspect's height
        //! @return  Height
        QUANTITYTAKEOFFSASPECTS_EXPORT double GetHeight() const;

        //! Set DimensionsAspect's height
        //! @param[in]  newHeight  Height
        QUANTITYTAKEOFFSASPECTS_EXPORT void SetHeight(double newHeight);
    };

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
