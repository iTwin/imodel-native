/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "CompatibilityTestFixture.h"
#include <DgnPlatform/DgnDomain.h>

#define TESTDOMAIN_NAME "IModelEvolutionTests"

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle     07/2018
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

