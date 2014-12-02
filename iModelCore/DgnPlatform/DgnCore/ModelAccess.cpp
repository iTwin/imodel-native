/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ModelAccess.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

BEGIN_BENTLEY_API_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  04/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::GetMillimetersPerMaster() const
    {
    UnitDefinitionCR units = GetModelInfo().GetMasterUnit();
    return units.IsLinear() ? units.ToMillimeters() : 1000.;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/01
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::GetSubPerMaster ()const
    {
    double  subPerMast;
    GetModelInfo().GetSubUnit().ConvertDistanceFrom (subPerMast, 1.0, GetModelInfo().GetMasterUnit());
    return subPerMast;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam Wilson      02/09
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnModel::GetBRepScaleToDestination (DgnModelR dst)
    {
#if defined (NEEDS_WORK_DGNITEM)
    if (this == &dst)
        return 1.0;

    double srcExtent = GetDgnProject().Units().GetSolidExtent();
    double dstExtent = dst.GetDgnProject().Units().GetSolidExtent();

    return srcExtent / dstExtent;
#endif
    return 1.0;
    }

END_BENTLEY_API_NAMESPACE
