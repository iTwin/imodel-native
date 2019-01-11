/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/TestDomain.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "TestDomain.h"

DOMAIN_DEFINE_MEMBERS(IModelEvolutionTestsDomain)

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    09/18
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus IModelEvolutionTestsDomain::Register(SchemaVersion const& version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly)
    {
    IModelEvolutionTestsDomain::GetDomain().SetVersion(version);
    return DgnDomains::RegisterDomain(IModelEvolutionTestsDomain::GetDomain(), isRequired, isReadonly);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                    09/18
//+---------------+---------------+---------------+---------------+---------------+------
void IModelEvolutionTestsDomain::SetVersion(SchemaVersion const& version)
    {
    m_relativePath.SetNameUtf8(Utf8PrintfString("ECSchemas\\Domain\\" TESTDOMAIN_NAME ".%s.ecschema.xml", version.ToString().c_str()));
    }

