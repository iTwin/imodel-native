/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"
#include <DgnPlatform/DgnDomain.h>

#define TESTDOMAIN_NAME "IModelEvolutionTests"

//=======================================================================================
// @bsiclass
//=======================================================================================    
struct IModelEvolutionTestsDomain final : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(IModelEvolutionTestsDomain,)

    private:
        BeFileName m_relativePath;

        WCharCP _GetSchemaRelativePath() const override { return m_relativePath.GetName(); }

    public:
        IModelEvolutionTestsDomain() : DgnDomain(TESTDOMAIN_NAME, "iModel Evolution Tests Domain", 1) {}
        ~IModelEvolutionTestsDomain() {}

        void ClearHandlers() { m_handlers.clear(); }
        void SetVersion(SchemaVersion const&);
        static BentleyStatus Register(SchemaVersion const& version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly);
    };

