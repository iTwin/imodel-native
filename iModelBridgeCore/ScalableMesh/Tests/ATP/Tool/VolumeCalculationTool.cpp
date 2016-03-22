#include "VolumeCalculationTool.h"
#include <ScalableMesh\IScalableMeshATP.h>

StatusInt ComputeVolumeForAgenda(PolyfaceHeaderPtr meshData, IScalableMeshPtr smPtr, double& cut, double& fill, double& volume)
    {
    clock_t timer = clock();
    IScalableMeshATP::StoreInt(L"nTrianglesInCorridor", meshData->GetNumFacet());
    volume = ComputeVolumeCutAndFill(smPtr->GetDTMInterface(), cut, fill, *meshData);
    timer = clock() - timer;
    float secs;
    secs = ((float)timer) / CLOCKS_PER_SEC;
    IScalableMeshATP::StoreDouble(L"volumeTime", secs);
    return SUCCESS;
    }

/*StatusInt ComputeVolumeForAgenda(ElementAgendaR agenda, IScalableMeshPtr smPtr, ElementAgendaR agendaGround, double& cut, double& fill, double& volume, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
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
    }*/

    /*double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeaderPtr& meshGround, PolyfaceHeader& mesh, DRange3d& elemRange, bvector<PolyfaceHeaderPtr>& volumeMeshVector)
        {
        //IScalableMeshPtr mrDTMPtr = (IScalableMesh*)smPtr.get();
        //DTMPtr dtmP = dynamic_cast<BENTLEY_NAMESPACE_NAME::TerrainModel::IDTM*>(&*smPtr->GetDTMInterface());

        IDTMVolumeP volume = smPtr->GetDTMVolume();
        if (volume == NULL) return 0;
        //volume->ComputeVolumeCutAndFill(meshGround, cut, fill, mesh, false, volumeMeshVector);
        volume->ComputeCutFillVolume(cut, fill, volume, mesh, volumeMeshVector);
        return cut - fill;
        }*/

double ComputeVolumeCutAndFill(DTMPtr smPtr, double& cut, double& fill, PolyfaceHeader& mesh)
    {
    double area;

    IDTMVolumeP volume = smPtr->GetDTMVolume();
    if (volume == NULL) return 0;
    volume->ComputeCutFillVolume(&cut, &fill, &area, &mesh);
    return cut - fill;
    }