/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/ReprojectionFilterFactory.cpp $
|    $RCSfile: ReprojectionFilterFactory.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2012/02/16 00:36:54 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/GeoCoords/GCS.h>
#include <ScalableTerrainModel/GeoCoords/Reprojection.h>

#include <ScalableTerrainModel/Import/Plugin/FilterV0.h>
#include <ScalableTerrainModel/Import/Plugin/ReprojectionFilterV0.h>
#include <ScalableTerrainModel/Import/Exceptions.h>

#include "PluginRegistryHelper.h"
#include "ReprojectionFilterFactory.h"


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterBase::ReprojectionFilterBase () 
    :   m_implP(0)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterBase::~ReprojectionFilterBase () 
    {
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterCreatorBase::ReprojectionFilterCreatorBase(const DataType& type)
    :   m_type(type)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterCreatorBase::~ReprojectionFilterCreatorBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& ReprojectionFilterCreatorBase::GetType () const
    {
    return m_type;
    }


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterCreatorPlugin
    {
private:
    friend struct                           ReprojectionFilterCreator;

    typedef Plugin::V0::ReprojectionFilterCreatorBase   
                                            Base;

    const Base*                             m_baseP;
public:
    typedef const Base*                     ID;

    explicit                                ReprojectionFilterCreatorPlugin                (const Base&                     base)
        :   m_baseP(&base)
        {
        }

    ID                                      GetID                                          () const { return m_baseP; }

    const DataType&                         GetType                                        () const { return m_baseP->m_type; }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterRegistryImpl : public PluginRegistry<ReprojectionFilterCreatorPlugin>
    {
    const CreatorPlugin*                FindCreatorFor                 (const DataType&            pi_rType) const
        {
        struct Ordering : std::binary_function<CreatorPlugin, CreatorPlugin, bool>
            {
            bool operator () (const CreatorPlugin& lhs, const CreatorPlugin& rhs) const
                { return lhs.GetType() < rhs.GetType(); }
            bool operator () (const CreatorPlugin& lhs, const DataType& rhs) const
                { return lhs.GetType() < rhs; }
            bool operator () (const DataType& lhs, const CreatorPlugin& rhs) const
                { return lhs < rhs.GetType(); }

            };

        return FindFirstCreatorFor(pi_rType, Ordering());
        }
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterRegistry::ReprojectionFilterRegistry ()
    :   m_implP(new ReprojectionFilterRegistryImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterRegistry::~ReprojectionFilterRegistry()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterRegistry::V0ID ReprojectionFilterRegistry::Register (const V0Creator& creator)
    {
    return m_implP->Register(ReprojectionFilterCreatorPlugin(creator));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ReprojectionFilterRegistry::Unregister (V0ID creatorID)
    {
    m_implP->Unregister(creatorID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterRegistry& ReprojectionFilterRegistry::GetInstance ()
    {
    static ReprojectionFilterRegistry SINGLETON;
    return SINGLETON;
    }



END_BENTLEY_MRDTM_IMPORT_PLUGIN_NAMESPACE

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterCreator : public Plugin::V0::FilterCreatorBase
    {
private:
    typedef Plugin::ReprojectionFilterCreatorPlugin
                                    CreatorPlugin;

    const CreatorPlugin&            m_rCreatorPlugin;
    DataType                        m_type;
    Reprojection                    m_reprojection;

    virtual void                    _Bind                              (const PacketGroup&          pi_Src,
                                                                        PacketGroup&                po_Dst) const override
        { m_rCreatorPlugin.m_baseP->Bind(pi_Src, po_Dst); }


    virtual FilterBase*             _Create                            (const PacketGroup&,
                                                                        PacketGroup&,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        log) const override
        {
        return m_rCreatorPlugin.m_baseP->_Create(m_reprojection, config, log);
        }

    virtual const DataType&         _GetSourceType                     () const override
        {
        return m_type;
        }

    virtual const DataType&         _GetTargetType                     () const override
        {
        return m_type;
        }


public:
    explicit                        ReprojectionFilterCreator          (const CreatorPlugin&            creatorPlugin,
                                                                        const DataType&                 type,
                                                                        const Reprojection&             reprojection)
        :   m_rCreatorPlugin(creatorPlugin),
            m_type(type),
            m_reprojection(reprojection)
        {
        }
    };







/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ReprojectionFilterFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const FilterRegistry&                   m_registry;
    const ReprojectionFactory               m_reprojectionFactory;
    Log&                                    m_log;

    explicit                                Impl                           (const FilterRegistry&           registry,
                                                                            Log&                            log)
        :   m_registry(registry),
            m_reprojectionFactory(log),
            m_log(log)
        {
        }

    explicit                                Impl                           (const FilterRegistry&           registry,
                                                                            const ReprojectionFactory&      reprojectionFactory,
                                                                            Log&                            log)
        :   m_registry(registry),
            m_reprojectionFactory(reprojectionFactory),
            m_log(log)
        {
        }


    /*---------------------------------------------------------------------------------**//**
    * @description  
    * @bsimethod                                                  Raymond.Gauthier   10/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    const Plugin::V0::FilterCreatorBase*    FindCreatorFor                 (const DataType&                 type,
                                                                            const Reprojection&             reprojection) const
        {
        const Plugin::ReprojectionFilterCreatorPlugin* creatorP 
            = m_registry.m_implP->FindCreatorFor(type);

        if (0 == creatorP)
            return 0;

        return new ReprojectionFilterCreator(*creatorP, type, reprojection);
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::ReprojectionFilterFactory (Log& log)
    :   m_implP(new Impl(Plugin::ReprojectionFilterRegistry::GetInstance(), log))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::ReprojectionFilterFactory  (const FilterRegistry&            registry,
                                                       Log&                             log)
    :   m_implP(new Impl(registry, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::ReprojectionFilterFactory  (const ReprojectionFactory&       reprojectionFactory,
                                                       Log&                             log)
    :   m_implP(new Impl(Plugin::ReprojectionFilterRegistry::GetInstance(), reprojectionFactory, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::ReprojectionFilterFactory   (const FilterRegistry&           registry,
                                                        const ReprojectionFactory&      reprojectionFactory,
                                                        Log&                            log)
    :   m_implP(new Impl(registry, reprojectionFactory, log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::ReprojectionFilterFactory (const ReprojectionFilterFactory& rhs)
    :   m_implP(rhs.m_implP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionFilterFactory::~ReprojectionFilterFactory  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Reprojection ReprojectionFilterFactory::CreateReprojectionFor  (const GCS&          sourceGCS,
                                                                const GCS&          targetGCS,
                                                                const DRange3d*     sourceExtentP) const
    {
    if (sourceGCS.IsEquivalent(targetGCS))
        return Reprojection::GetNull();

    ReprojectionFactory::Status status;
    Reprojection reprojection(m_implP->m_reprojectionFactory.Create(sourceGCS, targetGCS, sourceExtentP, status));
    if (Reprojection::S_SUCCESS != status) 
        throw ReprojectionException(status);

    return reprojection;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr ReprojectionFilterFactory::FindCreatorFor(const DataType&             type,
                                                            const GCS&                  sourceGCS,
                                                            const GCS&                  targetGCS,
                                                            const DRange3d*             sourceExtentP) const
    {
    return FindCreatorFor(type, 
                          CreateReprojectionFor(sourceGCS, targetGCS, sourceExtentP));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr ReprojectionFilterFactory::FindCreatorFor(const DataType&             type,
                                                            const Reprojection&         reprojection) const
    {
    const Plugin::V0::FilterCreatorBase* creatorBaseP = m_implP->FindCreatorFor(type, reprojection);
    return (0 == creatorBaseP) ? 0 : new FilterCreator(creatorBaseP);
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionCustomFilterFactory::ReprojectionCustomFilterFactory   (const ReprojectionFilterFactory&    factory,
                                                                    const Reprojection&                 reprojection)
    :   m_factory(factory),
        m_reprojection(reprojection)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectionCustomFilterFactory::~ReprojectionCustomFilterFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const ReprojectionCustomFilterFactory::FilterCreatorBase* ReprojectionCustomFilterFactory::_FindCreatorFor (const DataType& sourceType,
                                                                                                            Log&            log) const
    {
    return m_factory.m_implP->FindCreatorFor(sourceType, m_reprojection);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory ReprojectionCustomFilterFactory::CreateFrom    (const ReprojectionFilterFactory&    factory,
                                                                    const Reprojection&                 reprojection)
    {
    return CreateFromBase(new ReprojectionCustomFilterFactory(factory, reprojection));
    }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE