/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/TypeConversionFilterFactory.cpp $
|    $RCSfile: TypeConversionFilterFactory.cpp,v $
|   $Revision: 1.7 $
|       $Date: 2012/02/16 00:36:34 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Plugin/TypeConversionFilterV0.h>
#include <ScalableMesh/Import/Plugin/FilterV0.h>

#include <ScalableMesh/Import/DataType.h>

#include <ScalableMesh/Import/DataTypeDescription.h>

#include "TypeConversionFilterFactory.h"
#include "PluginRegistryHelper.h"

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterBase::TypeConversionFilterBase () 
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterBase::~TypeConversionFilterBase () 
    {
    }




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterCreatorBase::TypeConversionFilterCreatorBase   (const DataType&       pi_rSource,
                                                                        const DataType&       pi_rTarget)
    :   m_sourceType(pi_rSource),
        m_targetType(pi_rTarget)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterCreatorBase::~TypeConversionFilterCreatorBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& TypeConversionFilterCreatorBase::GetSourceType () const
    {
    return m_sourceType;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& TypeConversionFilterCreatorBase::GetTargetType () const
    {
    return m_targetType;
    }


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterCreatorPlugin
    {
private:
    friend struct                           TypeConversionFilterCreator;

    typedef Plugin::V0::TypeConversionFilterCreatorBase   
                                            Base;

    const Base*                             m_baseP;
public:
    typedef const Base*                     ID;

    explicit                                TypeConversionFilterCreatorPlugin              (const Base&                 base)
        :   m_baseP(&base)
        {
        }

    ID                                      GetID                                          () const { return m_baseP; }

    const DataType&                         GetSourceType                                  () const { return m_baseP->m_sourceType; }
    const DataType&                         GetTargetType                                  () const { return m_baseP->m_targetType; }
    

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterRegistryImpl : public PluginRegistry<TypeConversionFilterCreatorPlugin>
    {
    const CreatorPlugin*                FindCreatorFor                 (const DataType&             srcType,
                                                                        const DataType&             dstType) const
        {
        typedef std::pair<DataType, DataType> TypePair;

        struct Ordering : std::binary_function<CreatorPlugin, CreatorPlugin, bool>
            {
            bool IsLess (const DataType& lhsSrc, const DataType& rhsSrc, const DataType& lhsDst, const DataType& rhsDst) const
                {
                if (lhsSrc < rhsSrc)
                    return true;
                if (rhsSrc < lhsSrc)
                    return false;
                
                return lhsDst < rhsDst;
                }

            bool operator () (const CreatorPlugin& lhs, const CreatorPlugin& rhs) const
                { return IsLess(lhs.GetSourceType(), rhs.GetSourceType(), lhs.GetTargetType(), rhs.GetTargetType()); }
            bool operator () (const TypePair& lhs, const CreatorPlugin& rhs) const
                { return IsLess(lhs.first, rhs.GetSourceType(), lhs.second, rhs.GetTargetType()); }
            bool operator () (const CreatorPlugin& lhs, const TypePair& rhs) const
                { return IsLess(lhs.GetSourceType(), rhs.first, lhs.GetTargetType(), rhs.second); }
            };

        return FindFirstCreatorFor(std::make_pair(srcType, dstType), Ordering());
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterRegistry::TypeConversionFilterRegistry ()
    :   m_implP(new TypeConversionFilterRegistryImpl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterRegistry::~TypeConversionFilterRegistry()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterRegistry::V0ID  TypeConversionFilterRegistry::Register(const V0Creator& creator)
    {
    return m_implP->Register(TypeConversionFilterCreatorPlugin(creator));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void TypeConversionFilterRegistry::Unregister (V0ID id)
    {
    m_implP->Unregister(id);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterRegistry& TypeConversionFilterRegistry::GetInstance ()
    {
    static TypeConversionFilterRegistry SINGLETON;
    return SINGLETON;
    }

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterCreator : public Plugin::V0::FilterCreatorBase
    {
private:
    typedef Plugin::TypeConversionFilterCreatorPlugin
                                    CreatorPlugin;

    const CreatorPlugin&            m_plugin;
    DataType                        m_sourceType;
    DataType                        m_targetType;


    virtual void                    _Bind                              (const PacketGroup&          src,
                                                                        PacketGroup&                dst) const override
        { m_plugin.m_baseP->Bind(src, dst); }


    virtual FilterBase*             _Create                            (const PacketGroup&,
                                                                        PacketGroup&,
                                                                        const FilteringConfig&      config,
                                                                        Log&                        log) const override
        {
        return m_plugin.m_baseP->_Create(config, log);
        }

    virtual const DataType&         _GetSourceType                     () const override
        {
        return m_sourceType;
        }

    virtual const DataType&         _GetTargetType                     () const override
        {
        return m_targetType;
        }

public:
    explicit                        TypeConversionFilterCreator        (const CreatorPlugin&            pi_rPlugin,
                                                                        const DataType&                 pi_rSourceType,
                                                                        const DataType&                 pi_rTargetType)
        :   m_plugin(pi_rPlugin),
            m_sourceType(pi_rSourceType),
            m_targetType(pi_rTargetType)
            
        {
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeConversionFilterFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const TypeConversionRegistry&       m_registry;
    Log&                                m_warningLog;

    explicit                            Impl     (const TypeConversionRegistry&   registry,
                                                  Log&                            log)
        :   m_registry(registry),
            m_warningLog(log)
        {
        }


    const Plugin::V0::FilterCreatorBase* FindCreatorFor                            (const DataType&                 srcType,
                                                                                    const DataType&                 dstType) const
        {
        const Plugin::TypeConversionFilterCreatorPlugin* pCreator 
                = m_registry.m_implP->FindCreatorFor(srcType, dstType);
        
        if (0 == pCreator)
            return 0;

        return new TypeConversionFilterCreator(*pCreator, srcType, dstType);
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterFactory::TypeConversionFilterFactory (Log&                       log)
    :   m_pImpl(new Impl(Plugin::TypeConversionFilterRegistry::GetInstance(), log))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterFactory::TypeConversionFilterFactory (const Plugin::TypeConversionFilterRegistry&     registry,
                                                          Log&                       log)
    :   m_pImpl(new Impl(registry, log))
    {
    }


TypeConversionFilterFactory::TypeConversionFilterFactory (const TypeConversionFilterFactory& rhs)
    :   m_pImpl(rhs.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionFilterFactory::~TypeConversionFilterFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr TypeConversionFilterFactory::FindCreatorFor  (const DataType&         srcType,
                                                                const DataType&         dstType) const
    {
    const Plugin::V0::FilterCreatorBase* creatorBaseP = m_pImpl->FindCreatorFor(srcType, dstType);
    return (0 == creatorBaseP) ? 0 : new FilterCreator(creatorBaseP);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionCustomFilterFactory::TypeConversionCustomFilterFactory   (const TypeConversionFilterFactory&  factory,
                                                                        const DataType&                     targetType)
    :   m_factory(factory),
        m_targetType(targetType)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
TypeConversionCustomFilterFactory::~TypeConversionCustomFilterFactory  ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const TypeConversionCustomFilterFactory::FilterCreatorBase* TypeConversionCustomFilterFactory::_FindCreatorFor     (const DataType& sourceType,
                                                                                                                    Log&            log) const
    {
    return m_factory.m_pImpl->FindCreatorFor(sourceType, m_targetType);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory TypeConversionCustomFilterFactory::CreateFrom  (const TypeConversionFilterFactory&  factory,
                                                                    const DataType&                     targetType)
    {
    return CreateFromBase(new TypeConversionCustomFilterFactory(factory, targetType));
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
