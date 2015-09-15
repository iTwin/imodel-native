/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshLib.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>


USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <ScalableMesh\ScalableMeshLib.h>
#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>

BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

ScalableMeshLib::Host*        t_scalableTerrainModelHost;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshAdmin& ScalableMeshLib::Host::_SupplyScalableMeshAdmin()
    {
    return *new ScalableMeshAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void RegisterPODImportPlugin();

void ScalableMeshLib::Host::Initialize()
    {
    BeAssert (NULL == m_scalableTerrainModelAdmin);   
    m_scalableTerrainModelAdmin = &_SupplyScalableMeshAdmin();  
    Bentley::TerrainModel::Element::DTMElementHandlerManager::InitializeDgnPlatform();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::Host::Terminate(bool onProgramExit)
    {
    if (NULL == t_scalableTerrainModelHost)
        return;
         
    //Terminate host objects
    for(bvector<ObjEntry>::iterator itr=m_hostObj.begin(); itr!=m_hostObj.end(); ++itr)
        {
        IHostObject* pValue(itr->GetValue());
        TERMINATE_HOST_OBJECT(pValue, onProgramExit);
        }

    m_hostObj.clear();
    m_hostVar.clear();
                                
    TERMINATE_HOST_OBJECT(m_scalableTerrainModelAdmin, onProgramExit);    
    t_scalableTerrainModelHost = NULL;
    }

/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu.St-Pierre  05/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ScalableMeshLib::Initialize(ScalableMeshLib::Host& host)
    {    
    BeAssert (NULL == t_scalableTerrainModelHost);  // cannot be called twice on the same thread
    if (NULL != t_scalableTerrainModelHost)
        return;    

    t_scalableTerrainModelHost = &host;
    t_scalableTerrainModelHost->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ScalableMeshLib::IsInitialized ()
    {
    return NULL != t_scalableTerrainModelHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                     11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ScalableMeshLib::Host& ScalableMeshLib::GetHost() 
    {
    return *t_scalableTerrainModelHost;
    }

END_BENTLEY_SCALABLEMESH_NAMESPACE
