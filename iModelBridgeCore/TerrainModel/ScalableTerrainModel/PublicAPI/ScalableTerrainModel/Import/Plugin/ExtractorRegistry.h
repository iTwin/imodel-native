/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Plugin/ExtractorRegistry.h $
|    $RCSfile: ExtractorRegistry.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/02 14:59:04 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableTerrainModel/Import/Definitions.h>
#include <ScalableTerrainModel/Import/Plugin/RegistryTools.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE
struct InputExtractorCreator;
END_BENTLEY_MRDTM_IMPORT_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct InputExtractorCreatorBase;
END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

struct ExtractorRegistryImpl;

/*---------------------------------------------------------------------------------**//**
* @description  Registry for input extractor plug-ins.
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExtractorRegistry : private Uncopyable
    {
private:
    typedef V0::InputExtractorCreatorBase       V0Creator;
    typedef const V0Creator*                    V0ID;

    std::auto_ptr<ExtractorRegistryImpl>        m_implP;

public:
    typedef const InputExtractorCreator*        CreatorCIter;
    typedef std::pair<CreatorCIter, CreatorCIter> CreatorRange;

    typedef const void*                         SourceClassID;

    /*---------------------------------------------------------------------------------**//**
    * @description  RAII register creator template. 
    * @see RAIIRegisterPlugin
    * @bsiclass                                                  Raymond.Gauthier   7/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename CreatorT>
    struct AutoRegister 
        : public RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, ExtractorRegistry, typename CreatorT::ID> 
        {
        explicit                                AutoRegister                       () {}
        explicit                                AutoRegister                       (registry_type&                      registry) 
            : super_class(registry) {}
        };


    IMPORT_DLLE static ExtractorRegistry&       GetInstance                        ();


    IMPORT_DLLE explicit                        ExtractorRegistry                  ();
                                                ~ExtractorRegistry                 ();

    IMPORT_DLLE CreatorRange                    FindAppropriateCreator             (SourceClassID                       sourceClassID) const;


    IMPORT_DLLE V0ID                            Register                           (const V0Creator&                    creator);
    IMPORT_DLLE void                            Unregister                         (V0ID                                creatorID);

    };


END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE