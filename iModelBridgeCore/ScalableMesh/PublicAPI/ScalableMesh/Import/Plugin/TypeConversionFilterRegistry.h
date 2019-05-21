/*--------------------------------------------------------------------------------------+
|    $RCSfile: TypeConversionFilterRegistry.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/02 14:58:43 $
|     $Author: Raymond.Gauthier $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Plugin/RegistryTools.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct TypeConversionFilterCreatorBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct TypeConversionFilterFactory;
END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE

struct TypeConversionFilterRegistryImpl;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterRegistry : private Uncopyable
    {
private:
    friend struct                           BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::TypeConversionFilterFactory;

    typedef V0::TypeConversionFilterCreatorBase       
                                            V0Creator;
    typedef const V0Creator*                V0ID;

    std::auto_ptr<TypeConversionFilterRegistryImpl> 
                                            m_implP;

public:
    /*---------------------------------------------------------------------------------**//**
    * @description  RAII register factory template. 
    * @see RAIIRegisterPlugin
    * @bsiclass                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CreatorT>
    struct AutoRegister 
        : public RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, TypeConversionFilterRegistry, typename CreatorT::ID> 
        {
typedef  RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, TypeConversionFilterRegistry, typename CreatorT::ID>  super_class;
        explicit                                AutoRegister                       () {}
        explicit                                AutoRegister                       (typename super_class::registry_type&                      registry) 
            : super_class(registry) {}
        };

    IMPORT_DLLE static TypeConversionFilterRegistry&      
                                            GetInstance                        ();

    IMPORT_DLLE explicit                    TypeConversionFilterRegistry       ();
    IMPORT_DLLE                             ~TypeConversionFilterRegistry      ();

    IMPORT_DLLE V0ID                        Register                           (const V0Creator&          creator);
    IMPORT_DLLE void                        Unregister                         (V0ID                      creatorID);
    };


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE
