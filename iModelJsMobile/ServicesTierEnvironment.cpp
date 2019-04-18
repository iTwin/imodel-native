/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "iModelJsInternal.h"

BEGIN_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Environment::InstallCoreExtensions()
    {
    Extension::Install ([]() { return new Utilities; });
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Environment::Environment()
    {
    BeAssert (s_instance == nullptr);

    s_instance = this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Environment::~Environment()
    {
    BeAssert (s_instance == this);
    BeAssert (m_extensions.empty());

    s_instance = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
EnvironmentR Environment::GetInstance()
    {
    BeAssert (s_instance != nullptr);

    return *s_instance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Environment::Install (ExtensionPtr extension)
    {
    m_extensions [extension->SupplyName()] = extension;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
Napi::Value Environment::DeliverExtension(Js::RuntimeR runtime, Utf8StringCR identifier)
    {
    auto it = m_extensions.find(identifier);
    if (it == m_extensions.end())
        return runtime.Env().Undefined();

    return it->second->ExportJsModule(runtime);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Steve.Wilson                    7/2017
//---------------------------------------------------------------------------------------
void Environment::Shutdown()
    {
    m_extensions.clear();
    }

EnvironmentP Environment::s_instance = nullptr;

END_BENTLEY_IMODELJS_SERVICES_TIER_NAMESPACE
