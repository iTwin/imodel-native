/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//#include "..\GeoDTMCoreInit.h"

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier    01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename RegistryT, typename PluginT>
struct RegisterPluginInit : public GeoDTMCoreInit
    {
private:
    PluginT                     m_plugin;
    RegistryT&                  m_registry;

    virtual const WChar*         _GetDescription                        () const
        {
        return WString(typeid(PluginT).name()).c_str();
        }

    virtual bool                _OnInit                                ()
        {
        return 0 != m_registry.Register(m_plugin);
        }
    virtual bool                _OnDispose                             ()
        {
        m_registry.Unregister(&m_plugin);
        return true;
        }

public:
    explicit                    RegisterPluginInit                 ()
        :   m_registry(RegistryT::GetInstance())
        {
        }
    explicit                    RegisterPluginInit                 (RegistryT& registry)
        :   m_registry(registry)
        {
        }
    };

WString                     CreateAStrFromWCStr                 (const WChar*          cstr);


END_BENTLEY_SCALABLEMESH_NAMESPACE
