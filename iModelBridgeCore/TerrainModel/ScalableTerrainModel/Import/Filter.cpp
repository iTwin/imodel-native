/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/Filter.cpp $
|    $RCSfile: Filter.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/09/01 14:06:41 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/Warnings.h>

#include <ScalableTerrainModel/Import/Filter.h>
#include <ScalableTerrainModel/Import/Plugin/FilterV0.h>
#include <ScalableTerrainModel/Import/Plugin/CustomFilterFactoryV0.h>
#include <ScalableTerrainModel/Import/CustomFilterFactory.h>

#include <ScalableTerrainModel/Import/FilteringConfig.h>

BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorBase::FilterCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorBase::~FilterCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreator* FilterCreatorBase::CreateFromBase (FilterCreatorBase* baseP)
    {
    return new FilterCreator(baseP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterCreatorBase::_ProducesEffects () const
    {
    // Default is that filter produces some effects
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterBase::FilterBase ()
    :   m_implP(0)
    {

    }
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterBase::~FilterBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketBinder::PacketBinder ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketBinder::~PacketBinder  () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketBinder::Bind    (const Packet&   src,
                            Packet&         dst) const
    { 
    _Bind(src, dst); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroupBinder::PacketGroupBinder ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroupBinder::~PacketGroupBinder ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketGroupBinder::Bind   (const PacketGroup&  src,
                                PacketGroup&        dst) const 
    {
    _Bind(src, dst); 
    }


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


namespace {
const FilteringConfig  DEFAULT_FILTERING_CONFIG;
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreator::FilterCreator (const Base* baseP)
    :   m_baseP(baseP)
    {
    
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreator::~FilterCreator ()
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterPtr FilterCreator::Create   (const PacketGroup&      src,
                                    PacketGroup&            dst,
                                    Log&             log) const
    {
    return Create(src, dst, DEFAULT_FILTERING_CONFIG, log);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterPtr FilterCreator::Create   (PacketGroup&    src,
                                    PacketGroup&    dst,
                                    Log&     log) const
    {
    return Create(src, dst, DEFAULT_FILTERING_CONFIG, log);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterPtr FilterCreator::Create    (const PacketGroup&          src,
                                    PacketGroup&                dst,
                                    const FilteringConfig&      config,
                                    Log&                        log) const
    {
    m_baseP->Bind(src, dst);

    std::auto_ptr<Filter::Base> filterImplP(m_baseP->_Create(src, dst, config, log));
    if (0 == filterImplP.get())
        {
        assert(!"Bad!");
        return 0;
        }

    filterImplP->_Assign(src, dst);

    return new Filter(filterImplP.release());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterPtr FilterCreator::Create    (PacketGroup&                src,
                                    PacketGroup&                dst,
                                    const FilteringConfig&      config,
                                    Log&                        log) const
    {
    m_baseP->Bind(src, dst);

    std::auto_ptr<Filter::Base> filterImplP(m_baseP->_Create(src, dst, config, log));
    if (0 == filterImplP.get())
        {
        assert(!"Bad!");
        return 0;
        }

    filterImplP->_Assign(src, dst);

    return new Filter(filterImplP.release());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& FilterCreator::GetSourceType () const
    {
    return m_baseP->_GetSourceType();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& FilterCreator::GetTargetType () const
    {
    return m_baseP->_GetTargetType();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool FilterCreator::ProducesEffects () const
    {
    return m_baseP->_ProducesEffects();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Filter::Filter (Base*   baseP)
    :   m_baseP(baseP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Filter::~Filter ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void Filter::Run    ()
    {
    m_baseP->_Run();
    }


END_BENTLEY_MRDTM_IMPORT_NAMESPACE