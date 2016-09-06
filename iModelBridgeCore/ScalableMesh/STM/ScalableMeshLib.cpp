/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshLib.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "ImagePPHeaders.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#include <ScalableMesh\ScalableMeshLib.h>
//#include <TerrainModel/ElementHandler/DTMElementHandlerManager.h>
#include "Plugins\ScalableMeshTypeConversionFilterPlugins.h"
#include "ScalableMeshFileMoniker.h"
#include <ScalableMesh\IScalableMeshProgressiveQuery.h>
#include "SMMemoryPool.h"


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
    SMMemoryPool::GetInstance();
    m_scalableTerrainModelAdmin = &_SupplyScalableMeshAdmin();  
    InitializeProgressiveQueries();
    RegisterPODImportPlugin();
        BeFileName geocoordinateDataPath(L".\\GeoCoordinateData\\");
        GeoCoordinates::BaseGCS::Initialize(geocoordinateDataPath.c_str());
    //BENTLEY_NAMESPACE_NAME::TerrainModel::Element::DTMElementHandlerManager::InitializeDgnPlatform();
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
    TerminateProgressiveQueries();
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

    // Register point converters:
    static const RegisterIDTMPointConverter<DPoint3d, DPoint3d>                        s_ptTypeConv0;

    static const RegisterConverter<IDTMPointConverter< IDTMPointDimConverter<DPoint3d, DPoint3d> >,
        PointType3d64f_R16G16B16_I16Creator,
        PointType3d64fCreator>                                  s_ptTypeConvSp0;


    // Register linear converters
    static const RegisterIDTMLinearConverter<DPoint3d, DPoint3d>                       s_linTypeConv0;

    // Register linear to point converters
    static const RegisterIDTMLinearToPointConverter<DPoint3d, DPoint3d>                s_linToPtTypeConv0;



    // Register mesh converters
    static const RegisterMeshAsIDTMLinearConverter<DPoint3d, DPoint3d>                 s_meshTypeConv0;
    // Register mesh to points converters
    static const RegisterMeshAsIDTMLinearToPointConverter                                  s_meshToPtTypeConv0;
    // Register mesh to linear converters
    static const RegisterMeshAsIDTMLinearToIDTMLinearConverter                             s_meshToLinTypeConv0;


    // Register TIN converters
    static const RegisterTINAsIDTMLinearConverter<DPoint3d, DPoint3d>                  s_tinTypeConv0;
    // Register TIN to points converters
    static const RegisterTINAsIDTMLinearToPointConverter                                   s_tinToPtTypeConv0;
    // Register TIN to linear converters
    static const RegisterTINAsIDTMLinearToIDTMLinearConverter                              s_tinToLinTypeConv0;

    static const RegisterMeshConverter<DPoint3d, DPoint3d>                        s_ptMeshConv0;

    
    // Register Moniker

    

    t_scalableTerrainModelHost = &host;
    t_scalableTerrainModelHost->Initialize();
    BeFileName tempDir;
    BeFileNameStatus beStatus = BeFileName::BeGetTempPath(tempDir);
    assert(BeFileNameStatus::Success == beStatus);
    BeSQLiteLib::Initialize(tempDir.GetNameUtf8().c_str());
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
