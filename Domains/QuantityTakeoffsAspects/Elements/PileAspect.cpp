/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/PileAspect.h"

BEGIN_QUANTITYTAKEOFFSASPECTS_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PileAspect::PileAspect()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PileAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    return Dgn::DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PileAspect::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    return Dgn::DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PileAspect::_GetPropertyValue
(
ECN::ECValueR value, Utf8CP propertyName, 
Dgn::PropertyArrayIndex const& arrayIndex
) const
    {
    return Dgn::DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus PileAspect::_SetPropertyValue
(
Utf8CP propertyName,
ECN::ECValueCR value, Dgn::PropertyArrayIndex const& arrayIndex
)
    {
    return Dgn::DgnDbStatus::NotEnabled;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               05/2019
+---------------+---------------+---------------+---------------+---------------+------*/
PileAspectPtr PileAspect::Create()
    {
    return new PileAspect();
    }

END_QUANTITYTAKEOFFSASPECTS_NAMESPACE
