/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <Units/Units.h>

BEGIN_BENTLEY_UNITS_NAMESPACE

struct StandardUnitSystems
{
public:
    static UnitSystemR GetSI();
    static UnitSystemR GetCGS();
    static UnitSystemR GetMetric();
    static UnitSystemR GetImperial();
    static UnitSystemR GetMaritime();
    static UnitSystemR GetUSSurvey();
    static UnitSystemR GetIndustrial();
    static UnitSystemR GetInternational();
    static UnitSystemR GetUSCustom();
    static UnitSystemR GetStatistics();
    static UnitSystemR GetFinance();
    static UnitSystemR GetConstant();
    static UnitSystemR GetDummy();
};

END_BENTLEY_UNITS_NAMESPACE
