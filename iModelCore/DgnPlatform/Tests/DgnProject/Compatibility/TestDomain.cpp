/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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

