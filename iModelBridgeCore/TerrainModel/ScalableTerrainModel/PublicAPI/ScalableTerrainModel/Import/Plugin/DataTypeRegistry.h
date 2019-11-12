/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Plugin/RegistryTools.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct DataTypeFactory;

END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct StaticDataTypeCreatorBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

struct DataTypeRegistryImpl;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeRegistry : private Uncopyable
    {
private:
    friend struct                           DataTypeFactory;

    typedef V0::StaticDataTypeCreatorBase   V0Creator;
    typedef const V0Creator*                V0ID;

    std::auto_ptr<DataTypeRegistryImpl>     m_implP;
public:
    /*---------------------------------------------------------------------------------**//**
    * @description  RAII register factory template. 
    * @see RAIIRegisterPlugin
    * @bsiclass                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CreatorT>
    struct AutoRegister 
        : public RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, DataTypeRegistry, typename CreatorT::ID> 
        {
        explicit                            AutoRegister                       () {}
        explicit                            AutoRegister                       (registry_type&              registry) 
            :   super_class(registry) {}
        };

    IMPORT_DLLE static DataTypeRegistry&    GetInstance                        ();

    IMPORT_DLLE explicit                    DataTypeRegistry                   ();
    IMPORT_DLLE                             ~DataTypeRegistry                  ();

    IMPORT_DLLE V0ID                        Register                           (const V0Creator&          creator);
    IMPORT_DLLE void                        Unregister                         (V0ID                      creatorID);
    };


END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE