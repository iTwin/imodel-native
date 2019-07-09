/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include <TerrainModel/AutomaticGroundDetection/GroundDetectionManager.h>
#include "GroundDetectionManagerDc.h"

BEGIN_GROUND_DETECTION_NAMESPACE
  
static IGroundDetectionServices* s_pGroundDetectionManagerImpl = GroundDetectionManagerDc::GetServices();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetectionManager::Register(IGroundDetectionServices* pGroundDetectionServicesImpl)
    {
    if (pGroundDetectionServicesImpl == NULL)
        return;

    s_pGroundDetectionManagerImpl = pGroundDetectionServicesImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void GroundDetectionManager::UnRegister(IGroundDetectionServices* pGroundDetectionServicesImpl)
    {
    if (pGroundDetectionServicesImpl == s_pGroundDetectionManagerImpl)
        {
        s_pGroundDetectionManagerImpl = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IGroundDetectionServices*   GroundDetectionManager::GetServices()
    {
    return s_pGroundDetectionManagerImpl;
    }

END_GROUND_DETECTION_NAMESPACE
