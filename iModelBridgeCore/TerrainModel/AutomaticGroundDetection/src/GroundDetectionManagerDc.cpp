/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/GroundDetectionManagerDc.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include "GroundDetectionMacros.h"
#include "GroundDetectionManagerDc.h"
#include "PCGroundTIN.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_POINTCLOUD
USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_DCSTMCORE

BEGIN_GROUND_DETECTION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
IGroundDetectionServices*   GroundDetectionManagerDc::GetServices()
    {
    static  GroundDetectionManagerDc GDManager;
    return &GDManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetectionManagerDc::_DoGroundDetection(GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener)
    {
    StatusInt status(SUCCESS);
    
    ProgressReportPtr pReport(ProgressReport::Create(pProgressListener));
    pReport->SetTotalNumberOfPhases(3);

    //Select Initial seed points
    GroundDetectionParametersPtr pParameters = &params;

    if (m_PCGroundTIN == NULL)
    {
        if (pParameters->GetUseMultiThread())
            m_PCGroundTIN = PCGroundTINMT::Create(*pParameters, *pReport);
        else
            m_PCGroundTIN = PCGroundTIN::Create(*pParameters, *pReport);

        if (GroundDetectionParameters::USE_EXISTING_DTM != pParameters->GetCreateDtmFile())
            {
            //First we will find initial seed point and create our TIN
            status = m_PCGroundTIN->CreateInitialTIN();
            }
    }


    //Execute several iterations to densify our TIN
    if (pParameters->GetDensifyTin() && (status==SUCCESS))
        {
        status = m_PCGroundTIN->DensifyTIN();
        }


    //Classify all point cloud points from resulting TIN
    if (pParameters->GetClassifyPointCloud() && (status == SUCCESS) && (NULL!=params.GetElementHandleToClassifyCP()))
        {
        // Create channel handler for saving ground channel
        EditElementHandle eeh(*params.GetElementHandleToClassifyCP(), true);
        ClassificationChannelHandler* pData = ClassificationChannelHandler::GetChannelHandler(eeh);
        IPointCloudChannelPtr channel = pData->GetChannel();
        ChannelUndoRedoManager::GetInstance().SaveUndo(&eeh, pData, ClassificationChannelManager::Get()._GetExtension());

        status = m_PCGroundTIN->Classify();

        // Save classification channel
        SisterFileManager::GetInstance().SaveChannelToFile(eeh, channel, ClassificationChannelManager::Get()._GetExtension());

        // For undo/redo classification
        ChannelUndoRedoManager::GetInstance().RecordUndoRedo(true, ClassificationChannelManager::Get()._GetExtension());
        }
    
    //Attach result in STM Manager
    if (params.GetCreateStmAttachment())
        {
        MrDTMNewSettings stmSettings;
        BeFileName stmFilename(params.GetStmFilename());
        BeFileName tinFilename(params.GetDtmFilename());

        StmManager::CreateNewStmAttachmentWithElevationFromFile(stmFilename.c_str(), stmSettings, tinFilename.c_str(), BENTLEY_NAMESPACE_NAME::MrDTM::DTMSourceDataType::DTM_SOURCE_DATA_DTM);
        }

    //Destroy PCGroundTIN - we want to recreate it on next ground detection
    m_PCGroundTIN = nullptr;    
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetectionManagerDc::_GetSeedPointsFromTIN(bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener)
    {
    StatusInt status(SUCCESS);
        
    ProgressReportPtr pReport(ProgressReport::Create(pProgressListener));
    pReport->SetTotalNumberOfPhases(3);

    //Select Initial seed points
    GroundDetectionParametersPtr pParameters = &params;
    
    if (pParameters->GetUseMultiThread())
        m_PCGroundTIN = PCGroundTINMT::Create(*pParameters, *pReport);
    else
        m_PCGroundTIN = PCGroundTIN::Create(*pParameters, *pReport);


    if (GroundDetectionParameters::USE_EXISTING_DTM != pParameters->GetCreateDtmFile())
        {
        //First we will find initial seed point and create our TIN
        status = m_PCGroundTIN->CreateInitialTIN();
        }

    //Get the points
    m_PCGroundTIN->GetDTMPoints(seedpoints);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GroundDetectionManagerDc::_UpdateTINFileFromPoints(const bvector<DPoint3d>& seedpoints, GroundDetectionParameters& params, IGroundDetectionProgressListener* pProgressListener)
    {
    StatusInt status(SUCCESS);

    m_PCGroundTIN->SetNewSeedPoints(seedpoints);

    return status;
    }

END_GROUND_DETECTION_NAMESPACE
