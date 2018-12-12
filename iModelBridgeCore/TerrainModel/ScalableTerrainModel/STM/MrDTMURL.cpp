/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/STM/MrDTMURL.cpp $
|    $RCSfile: MrDTMURL.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/07/20 20:22:27 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>

#include <ScalableTerrainModel/IMrDTMURL.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct LocalFileURL::Impl
    {
    WString                m_localPath;

    explicit                    Impl                       (const WChar*              localPath)
        :   m_localPath(localPath)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileURL::LocalFileURL (const WChar* localFilePath)
    :   m_implP(new Impl(localFilePath))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileURL::~LocalFileURL ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileURL::LocalFileURL (const LocalFileURL& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
LocalFileURL& LocalFileURL::operator= (const LocalFileURL& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& LocalFileURL::GetPath () const
    {
    return m_implP->m_localPath;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                 Raymond.Gauthier    07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* LocalFileURL::GetPathCStr () const
    {
    return m_implP->m_localPath.c_str();
    }



END_BENTLEY_MRDTM_NAMESPACE