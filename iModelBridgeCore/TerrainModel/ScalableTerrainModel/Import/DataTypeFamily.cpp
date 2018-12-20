/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/DataTypeFamily.cpp $
|    $RCSfile: DataTypeFamily.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/07/12 13:25:18 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/DataTypeFamily.h>
#include <ScalableTerrainModel/Import/Plugin/DataTypeFamilyV0.h>


BEGIN_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamilyBase::DataTypeFamilyBase (ClassID     id,
                                        UInt        roleQty)     
    :   m_id(id), 
        m_roleQty(roleQty),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamilyBase::~DataTypeFamilyBase ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamilyCreatorBase::DataTypeFamilyCreatorBase () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamilyCreatorBase::~DataTypeFamilyCreatorBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamily DataTypeFamilyCreatorBase::CreateFrom (DataTypeFamilyBase* implP) const
    { 
    return DataTypeFamily(implP); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeFamilyCreatorBase::StaticDataTypeFamilyCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeFamilyCreatorBase::~StaticDataTypeFamilyCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& StaticDataTypeFamilyCreatorBase::Create () const
    {
    return _Create();
    }



END_BENTLEY_MRDTM_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamily::DataTypeFamily (DataTypeFamilyBase* pi_pImpl)
    :   m_pImpl(pi_pImpl),  
        m_classID(m_pImpl->m_id)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamily::DataTypeFamily (const DataTypeFamily& rhs)
    :   m_pImpl(rhs.m_pImpl),  
        m_classID(m_pImpl->m_id)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamily& DataTypeFamily::operator= (const DataTypeFamily& rhs)
    {
    m_pImpl = rhs.m_pImpl;
    m_classID = m_pImpl->m_id;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFamily::~DataTypeFamily ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt DataTypeFamily::GetRoleQty () const
    {
    return m_pImpl->m_roleQty;
    }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE