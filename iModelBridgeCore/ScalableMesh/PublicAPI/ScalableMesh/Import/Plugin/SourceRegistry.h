/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/SourceRegistry.h $
|    $RCSfile: SourceRegistry.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/09/01 14:07:37 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/Plugin/RegistryTools.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct SourceRef;
struct LocalFileSourceRef;
struct DGNElementSourceRef;

struct SourceCreator;
struct LocalFileSourceCreator;
struct DGNElementSourceCreator;

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)
struct SourceCreatorBase;
struct LocalFileSourceCreatorBase;
struct DGNElementSourceCreatorBase;
END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE

struct SourceRegistryImpl;

/*---------------------------------------------------------------------------------**//**
* @description  Registry for source plug-ins
*               TDORAY: Should we also register store and sink here?
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct SourceRegistry : private Uncopyable
    {
private:
    typedef V0::SourceCreatorBase               V0Creator;
    typedef const V0Creator*                    V0ID;

    typedef V0::LocalFileSourceCreatorBase      V0LocalFileCreator;
    typedef const V0LocalFileCreator*           V0LocalFileID;

    typedef V0::DGNElementSourceCreatorBase     V0DGNElementCreator;
    typedef const V0DGNElementCreator*          V0DGNElementID;

    std::auto_ptr<SourceRegistryImpl>           m_implP;

public:
    template <typename CreatorT>
    struct AutoRegister 
        : public RAIIAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, SourceRegistry, typename CreatorT::ID> 
        {
        explicit                                AutoRegister                       () {}
        explicit                                AutoRegister                       (registry_type&                      registry) 
            : super_class(registry) {}
        };

    template <typename CreatorT>
    struct PriorizedAutoRegister 
        : public RAIIPriorizedAutoRegisterMixin<AutoRegister<CreatorT>, CreatorT, SourceRegistry, typename CreatorT::ID, int> 
        {
        explicit                                PriorizedAutoRegister              (int                                 priority) 
            : super_class(priority) {}
        explicit                                PriorizedAutoRegister              (registry_type&                      registry,
                                                                                    int                                 priority) 
            : super_class(registry, priority) {}
        };



    IMPORT_DLLE static SourceRegistry&          GetInstance                        ();


    IMPORT_DLLE explicit                        SourceRegistry                     ();
    IMPORT_DLLE                                 ~SourceRegistry                    ();


    IMPORT_DLLE const SourceCreator*            FindAppropriateCreator             (const SourceRef&                    sourceRef) const;
    IMPORT_DLLE const LocalFileSourceCreator*   FindAppropriateCreator             (const LocalFileSourceRef&           sourceRef) const;
    IMPORT_DLLE const DGNElementSourceCreator*  FindAppropriateCreator             (const DGNElementSourceRef&          sourceRef) const;


    IMPORT_DLLE V0ID                            Register                           (const V0Creator&                    creator);
    IMPORT_DLLE void                            Unregister                         (V0ID                                creatorID);

    IMPORT_DLLE V0LocalFileID                   Register                           (const V0LocalFileCreator&           creator);
    IMPORT_DLLE V0LocalFileID                   Register                           (const V0LocalFileCreator&           creator,
                                                                                    int                                 priority);
    IMPORT_DLLE void                            Unregister                         (V0LocalFileID                       creatorID);

    IMPORT_DLLE V0DGNElementID                  Register                           (const V0DGNElementCreator&          creator);
    IMPORT_DLLE void                            Unregister                         (V0DGNElementID                      creatorID);

    };

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE
