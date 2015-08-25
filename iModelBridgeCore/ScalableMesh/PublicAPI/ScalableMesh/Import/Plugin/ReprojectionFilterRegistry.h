/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/ReprojectionFilterRegistry.h $
|    $RCSfile: ReprojectionFilterRegistry.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 14:58:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Plugin/RegistryTools.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct ReprojectionFilterCreatorBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct ReprojectionFilterFactory;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE

struct ReprojectionFilterRegistryImpl;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterRegistry : private Uncopyable
    {
private:
    friend struct                               ReprojectionFilterFactory;

    std::auto_ptr<ReprojectionFilterRegistryImpl>   
                                                m_implP;

    typedef V0::ReprojectionFilterCreatorBase   V0Creator;
    typedef const V0Creator*                    V0ID;


public:
    /*---------------------------------------------------------------------------------**//**
    * @description  RAII register factory template. 
    * @see RAIIRegisterPlugin
    * @bsiclass                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CreatorT>
    struct AutoRegister 
        : public RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, ReprojectionFilterRegistry, typename CreatorT::ID> 
        {
        explicit                                AutoRegister                       () {}
        explicit                                AutoRegister                       (registry_type&                      registry) 
            : super_class(registry) {}
        };


    IMPORT_DLLE static ReprojectionFilterRegistry&      
                                                GetInstance                        ();


    IMPORT_DLLE explicit                        ReprojectionFilterRegistry         ();
    IMPORT_DLLE                                 ~ReprojectionFilterRegistry        ();

    IMPORT_DLLE V0ID                            Register                           (const V0Creator&          creator);
    IMPORT_DLLE void                            Unregister                         (V0ID                      creatorID);
    };


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE
