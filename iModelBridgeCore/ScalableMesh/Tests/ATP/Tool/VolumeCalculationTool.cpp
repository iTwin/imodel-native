#include "VolumeCalculationTool.h"
#include "..\TiledTriangulation\MrDTMUtil.h"
#include <ScalableMesh\IScalableMeshATP.h>

StatusInt ComputeVolumeForAgenda(ElementAgendaR agenda, IScalableMeshPtr smPtr, double& cut, double& fill, double& volume, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    EditElementHandleP meshElement = agenda.GetFirstP();

    Handler&  elmHandler = meshElement->GetHandler();
    IMeshQuery*  meshQuery = dynamic_cast <IMeshQuery*> (&elmHandler);
    clock_t timer = clock();
    DRange3d elemRange;
    ScanRangeCP scanRangeP = elemHandle_checkIndexRange(*(EditElementHandleP)agenda.GetFirstP());
    DataConvert::ScanRangeToDRange3d(elemRange, *scanRangeP);
    if (NULL != meshQuery)
        {
        PolyfaceHeaderPtr meshData;

        if (SUCCESS == meshQuery->GetMeshData(*meshElement, meshData))
            {
            Transform uorToMeter, meterToUor;
            GetTransformForPoints(uorToMeter, meterToUor);
            meshData->Transform(uorToMeter);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.high, 1);
            Transform refToActiveTrf;
            GetFromModelRefToActiveTransform(refToActiveTrf, meshElement->GetModelRef());
            meshData->Transform(refToActiveTrf);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.high, 1);
            IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", meshData->GetNumFacet());
            
            //PolyfaceHeaderPtr dataMesh = PolyfaceHeader::Create
            PolyfaceHeaderPtr meshDataTest;
            PolyfaceQuery* poly = new PolyfaceQueryCarrier(3, false/*twoSided*/, meshData->GetFaceIndexCount(), meshData->GetPointCount(), meshData->GetPointCP(), meshData->GetFaceIndexCP());
            //PolyfaceHeaderPtr meshData;
            IFacetOptionsPtr options = IFacetOptions::Create();
            options->SetMaxPerFace(3);
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*options);
            builder->AddPolyface(*poly);
            meshDataTest = builder->GetClientMeshPtr();

            double area;
            IDTMVolumeP volumeDTM = smPtr->GetDTMVolume();
            if (volumeDTM == NULL) return 0;
            volumeDTM->ComputeVolumeCutAndFill(cut, fill, area, *meshDataTest, elemRange, volumeMeshVector);
            volume = cut - fill;

            //volume = ComputeVolumeCutAndFill(smPtr->GetDTMInterface(), cut, fill, *meshData, elemRange, volumeMeshVector);
            timer = clock() - timer;
            float secs;
            secs = ((float)timer) / CLOCKS_PER_SEC;
            IScalableMeshATP::StoreDouble(L"volumeTime", secs);
            return SUCCESS;
            }
        }
    return ERROR;

    }

StatusInt ComputeVolumeForAgenda(ElementAgendaR agenda, IScalableMeshPtr smPtr, ElementAgendaR agendaGround, double& cut, double& fill, double& volume, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    EditElementHandleP meshElement = agenda.GetFirstP();

    Handler&  elmHandler = meshElement->GetHandler();
    IMeshQuery*  meshQuery = dynamic_cast <IMeshQuery*> (&elmHandler);
    clock_t timer = clock();
    DRange3d elemRange;
    ScanRangeCP scanRangeP = elemHandle_checkIndexRange(*(EditElementHandleP)agenda.GetFirstP());
    DataConvert::ScanRangeToDRange3d(elemRange, *scanRangeP);

    EditElementHandleP meshElementGround = agendaGround.GetFirstP();

    Handler&  elmHandlerGround = meshElementGround->GetHandler();
    IMeshQuery*  meshQueryGround = dynamic_cast <IMeshQuery*> (&elmHandlerGround);
    timer = clock();
    DRange3d elemRangeGround;
    ScanRangeCP scanRangeGroundP = elemHandle_checkIndexRange(*(EditElementHandleP)agendaGround.GetFirstP());
    DataConvert::ScanRangeToDRange3d(elemRangeGround, *scanRangeGroundP);
    if (NULL != meshQuery && NULL != meshQueryGround)
        {
        PolyfaceHeaderPtr meshData;
        PolyfaceHeaderPtr meshDataGround;

        if (SUCCESS == meshQuery->GetMeshData(*meshElement, meshData) && SUCCESS == meshQueryGround->GetMeshData(*meshElementGround, meshDataGround))
            {
            Transform uorToMeter, meterToUor;
            GetTransformForPoints(uorToMeter, meterToUor);
            meshData->Transform(uorToMeter);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.high, 1);

            meshDataGround->Transform(uorToMeter);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRangeGround.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRangeGround.high, 1);

            Transform refToActiveTrf, refToActiveTrfGround;
            GetFromModelRefToActiveTransform(refToActiveTrf, meshElement->GetModelRef());
            GetFromModelRefToActiveTransform(refToActiveTrfGround, meshElementGround->GetModelRef());
            meshData->Transform(refToActiveTrf);
            meshDataGround->Transform(refToActiveTrfGround);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.high, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrfGround, &elemRangeGround.low, 1);
            bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrfGround, &elemRangeGround.high, 1);
            IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", meshData->GetNumFacet());
            IScalableMeshATP::StoreInt(L"nTrianglesInCorridorGround", meshDataGround->GetNumFacet());
            //volume = ComputeVolumeCutAndFill(smPtr, cut, fill, *meshData, elemRange, volumeMeshVector);

            //            double area;
            //            ScalableMeshVolume volumeComputation();
            //            volumeComputation.ComputeVolumeCutAndFillForTile(meshDataGround, cut, fill, meshData, false, volumeMeshVector);

            volume = ComputeVolumeCutAndFill(smPtr->GetDTMInterface(), cut, fill, meshDataGround, *meshData, elemRange, volumeMeshVector);
            timer = clock() - timer;
            float secs;
            secs = ((float)timer) / CLOCKS_PER_SEC;
            IScalableMeshATP::StoreDouble(L"volumeTime", secs);
            return SUCCESS;
            }
        }
    return ERROR;
    }

StatusInt ComputeVolumeForAgenda(BENTLEY_NAMESPACE_NAME::DRange3d& elemRange, PolyfaceHeaderPtr meshData, IScalableMeshPtr smPtr, double& cut, double& fill, double& volume, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    clock_t timer = clock();


    //Transform uorToMeter, meterToUor;
    //GetTransformForPoints(uorToMeter, meterToUor);
    //meshData->Transform(uorToMeter);
    //bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.low, 1);
    //bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, &elemRange.high, 1);
    //Transform refToActiveTrf;
    //GetFromModelRefToActiveTransform(refToActiveTrf, meshElement->GetModelRef());
    //meshData->Transform(refToActiveTrf);
    //bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.low, 1);
    //bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &elemRange.high, 1);
    IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", meshData->GetNumFacet());
    //volume = ComputeVolumeCutAndFill(smPtr->GetDTMInterface(), cut, fill, *meshData, elemRange, volumeMeshVector);

    double area;
    IDTMVolumeP volumeDTM = smPtr->GetDTMVolume();
    if (volumeDTM == NULL) return 0;
    volumeDTM->ComputeVolumeCutAndFill(cut, fill, area, *meshData, elemRange, volumeMeshVector);
    volume = cut - fill;

    timer = clock() - timer;
    float secs;
    secs = ((float)timer) / CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(L"volumeTime", secs);
    return SUCCESS;
    }

/*double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeader& mesh/*, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector*//*)
    {
    double area;

    //IScalableMeshPtr mrDTMPtr = (IScalableMesh*)smPtr.get();
    //DTMPtr dtmP = dynamic_cast<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*>(&*smPtr->GetDTMInterface());

    //IDTMVolumeP volume = smPtr->GetDTMVolume();
    IScalableMeshPtr mrDTMPtr = (IScalableMesh*)smPtr.get();
    IDTMVolumeP volume = mrDTMPtr->GetDTMVolume();

    if (volume == NULL) return 0;
    //volume->ComputeCutFillVolume(cut, fill, area, mesh, elemRange, volumeMeshVector);
    volume->ComputeCutFillVolume(&cut, &fill, &area, &mesh/*, elemRange, volumeMeshVector*//*);
    return cut - fill;
    }*/

double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeaderPtr& meshGround, PolyfaceHeader& mesh, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    IScalableMeshPtr mrDTMPtr = (IScalableMesh*)smPtr.get();
    IDTMVolumeP volume = mrDTMPtr->GetDTMVolume();
    if (volume == NULL) return 0;
    volume->ComputeVolumeCutAndFill(meshGround, cut, fill, mesh, false, volumeMeshVector);
    return cut - fill;
    }

double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeader& mesh, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
    {
    double area;

    IScalableMeshPtr mrDTMPtr = (IScalableMesh*)smPtr.get();
    IDTMVolumeP volume = mrDTMPtr->GetDTMVolume();
    if (volume == NULL) return 0;
    volume->ComputeVolumeCutAndFill(cut, fill, area, mesh, elemRange, volumeMeshVector);
    return cut - fill;
    }