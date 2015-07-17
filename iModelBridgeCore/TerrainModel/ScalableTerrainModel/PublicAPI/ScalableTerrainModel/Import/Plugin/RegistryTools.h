/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/RegistryTools.h $
|    $RCSfile: RegistryTools.h,v $
|   $Revision: 1.7 $
|       $Date: 2011/08/04 19:28:38 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  RAII register creator template. Used to register a creator when 
*               constructed and unregister it when destructed. This can be used at
*               any scope (function/class/module) but is particularly useful 
*               when used at module scope for registering plug-ins importers. When 
*               used at module scope, importer creator is registered at DLL load and
*               cleanly unregistered at DLL unload.
*
*               E.g. (module scope): 
*               MyCpp.cpp :
*
*               template <typename PluginType>
*               struct MyRAIIRegister : RAIIRegisterPlugin<MyRAIIRegister<PluginType>, PluginType, MyPluginRegistry> {};
*
*               static const MyRAIIRegister<MyPluginCreatorType> 
*                   s_RegisterImporter;
*
*               E.g. (function scope): 
*               MyFunction {
*               const MyRAIIRegister<MyImporterCreatorType> RegisterImporter;
*               ... (use MasterImporter)
*               } // NOTE: registered creator is automatically unregistered here
*
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename RAIIRegisterT, typename PluginCreatorT, typename RegistryT, typename PluginIDT>
struct RAIIAutoRegisterMixin
    {
private:
    RegistryT&                                  m_registry;
    PluginIDT                                   m_pluginID;

protected:
    typedef RegistryT                           registry_type;

    // May be hidden by RAIIRegisterT in order to customize creation phase
    static typename const PluginCreatorT&       GetCreator                             () 
        { 
        static const PluginCreatorT SINGLETON;
        return SINGLETON;
        }

    explicit                                    RAIIAutoRegisterMixin                  ()   
        :   m_registry(RegistryT::GetInstance()),
            m_pluginID (m_registry.Register(typename RAIIRegisterT::GetCreator())) 
        {}

    explicit                                    RAIIAutoRegisterMixin                  (RegistryT&      registry)   
        :   m_registry(registry),
            m_pluginID (m_registry.Register(typename RAIIRegisterT::GetCreator())) 
        {}


    explicit                                    RAIIAutoRegisterMixin                  (const PluginCreatorT&   creator)   
        :   m_registry(RegistryT::GetInstance()),
            m_pluginID (m_registry.Register(creator)) 
        {}

    explicit                                    RAIIAutoRegisterMixin                  (RegistryT&              registry,
                                                                                        const PluginCreatorT&   creator)   
        :   m_registry(registry),
            m_pluginID (m_registry.Register(creator)) 
        {}

    // Not meant to be used polymorphically
                                                ~RAIIAutoRegisterMixin                 ()   
        {m_registry.Unregister(m_pluginID);}

public:
    typedef RAIIAutoRegisterMixin<RAIIRegisterT, PluginCreatorT, RegistryT, PluginIDT>
                                                super_class;

    };


/*---------------------------------------------------------------------------------**//**
* @description  Same as preceding
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename RAIIRegisterT, typename PluginCreatorT, typename RegistryT, typename PluginIDT, typename PriorityT>
struct RAIIPriorizedAutoRegisterMixin
    {
private:
    RegistryT&                                  m_registry;
    PluginIDT                                   m_pluginID;

protected:
    typedef RegistryT                           registry_type;

    // May be hidden by RAIIRegisterT in order to customize creation phase
    static typename const PluginCreatorT&       GetCreator                             () 
        { 
        static const PluginCreatorT SINGLETON;
        return SINGLETON;
        }

    explicit                                    RAIIPriorizedAutoRegisterMixin         (PriorityT               priority)   
        :   m_registry(RegistryT::GetInstance()),
            m_pluginID (m_registry.Register(typename RAIIRegisterT::GetCreator(), priority)) 
        {}

    explicit                                    RAIIPriorizedAutoRegisterMixin         (RegistryT&              registry,
                                                                                        PriorityT               priority)   
        :   m_registry(registry),
            m_pluginID (m_registry.Register(typename RAIIRegisterT::GetCreator(), priority)) 
        {}


    explicit                                    RAIIPriorizedAutoRegisterMixin         (const PluginCreatorT&   creator,
                                                                                        PriorityT               priority)   
        :   m_registry(RegistryT::GetInstance()),
            m_pluginID (m_registry.Register(creator, priority)) 
        {}

    explicit                                    RAIIPriorizedAutoRegisterMixin         (RegistryT&              registry,
                                                                                        const PluginCreatorT&   creator,
                                                                                        PriorityT               priority)   
        :   m_registry(registry),
            m_pluginID (m_registry.Register(creator, priority)) 
        {}

    // Not meant to be used polymorphically
                                                ~RAIIPriorizedAutoRegisterMixin        ()   
        {m_registry.Unregister(m_pluginID);}

public:
    typedef RAIIPriorizedAutoRegisterMixin<RAIIRegisterT, PluginCreatorT, RegistryT, PluginIDT, PriorityT>
                                                super_class;

    };

END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE



