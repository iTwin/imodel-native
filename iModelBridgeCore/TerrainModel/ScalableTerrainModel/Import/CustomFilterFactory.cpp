/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/CustomFilterFactory.cpp $
|    $RCSfile: CustomFilterFactory.cpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/09/01 14:06:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/CustomFilterFactory.h>
#include <ScalableTerrainModel/Import/Plugin/CustomFilterFactoryV0.h>



BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory CustomFilterFactoryBase::CreateFromBase (CustomFilterFactoryBase* filterP)
    {
    return CustomFilterFactory(filterP);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactoryBase::CustomFilterFactoryBase ()
    :   m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactoryBase::~CustomFilterFactoryBase ()
    {
    }


END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE



BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE


USING_NAMESPACE_BENTLEY_MRDTM_IMPORT_PLUGIN_VERSION(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory::CustomFilterFactory (Base* filterP)
    :   m_basePtr(filterP)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory::~CustomFilterFactory ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory::CustomFilterFactory (const CustomFilterFactory& rhs)
    :   m_basePtr(rhs.m_basePtr)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilterFactory& CustomFilterFactory::operator= (const CustomFilterFactory& rhs)
    {
    m_basePtr = rhs.m_basePtr;
    return *this;
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FilterCreatorCPtr CustomFilterFactory::FindCreatorFor      (const DataType& sourceType,
                                                            Log&            log) const
    {
    return new FilterCreator(const_cast<FilterCreatorBase*>(m_basePtr->_FindCreatorFor(sourceType, log)));
    }


struct CustomFilteringSequence::Impl
    {
    typedef bvector<CustomFilterFactory>    FilterList;
    FilterList                              m_filters;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::CustomFilteringSequence   ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::CustomFilteringSequence (const CustomFilterFactory& filter)
    :   m_implP(new Impl)
    { 
    push_back(filter); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::~CustomFilteringSequence  ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::CustomFilteringSequence (const CustomFilteringSequence& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence& CustomFilteringSequence::operator= (const CustomFilteringSequence& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::const_iterator CustomFilteringSequence::begin () const 
    { 
    return &*m_implP->m_filters.begin(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
CustomFilteringSequence::const_iterator CustomFilteringSequence::end () const 
    { 
    return &*m_implP->m_filters.end(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CustomFilteringSequence::GetCount () const 
    { 
    return m_implP->m_filters.size(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void CustomFilteringSequence::push_back (const CustomFilterFactory& filter)
    { 
    m_implP->m_filters.push_back(filter); 
    }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE