/*-------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATP/TiledTriangulation/MrDTMUtil.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <TerrainModel/TerrainModel.h>
#include <DgnPlatform/DgnPlatform.h>
//#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>
#include <ImagePP/h/ImageppAPI.h>
#include <GeoCoord/BaseGeoCoord.h>
//#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>
//#include <ScalableMesh/GeoCoords/Definitions.h>
#include <ScalableMesh/GeoCoords/GCS.h>
//#include <DgnPlatform/ElementHandle.h>
//#include <TerrainModel/ElementHandler/DTMDataRef.h>
//#include <TerrainModel/ElementHandler/IMrDTMDataRef.h>
#include <ScalableMesh/IScalableMesh.h>
#include <DgnPlatform/DgnPlatform.r.h>
#undef static_assert
#include <DgnPlatform/DgnModel.h>

#include    <DgnPlatform/DgnCoreAPI.h>

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh\ScalableMeshDefs.h>
//#include <ScalableMesh\IScalableMeshStream.h>
#include <ScalableMesh\IScalableMeshURL.h>
#include <ScalableMesh\Import\ScalableMeshData.h>
#include <ScalableMesh\IScalableMeshSources.h>
#include <ScalableMesh\IScalableMeshSourceImportConfig.h>
#include <ScalableMesh\IScalableMeshSourceCollection.h>

//#include    <DgnGeoCoord\DgnGeoCoordApi.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_SCALABLEMESH_GEOCOORDINATES

using namespace std;

typedef enum
    {
    MrDTMTakeGCSFrom_CurrentModel    =(0),
    MrDTMTakeGCSFrom_NameSpecified   =(1)
    } MrDTMGCSInputSource;

typedef enum
    {
    Extract_SUCCESS           = 0,
    Extract_ERROR             = 1,
    Extract_NBPTSEXCEEDMAX    = 2,
    Extract_NOPOINTS          = 3,
    Extract_ABORT             = 4,
    } ExtractStatus;

typedef enum
    {
    DTMUnit_RootGlobalCS    =(0),
    DTMUnit_ActiveMeter     =(1),
    } DTMUnit;



void AugmentExtentByFactor(DRange2d& extent, const double& factor);

uint64_t getExtractedTMMaxPointCount ();

//bool GeoCoord_IsAvailable();
//bool IsElemHandleReadOnly (ElementHandle eh);
void EnsureFolderSeparatorCharacterAtEnd (WStringR wFolderName);
//StatusInt LoadStringFromGEODTM(WChar* pString, UInt32 listId, UInt32 stringNum);
/*StatusInt ExportSTMToDTM(EditElementHandleR eeh, DgnModelRefP modelRef);
StatusInt ExportSTMToDTM(EditElementHandleR dtmElm, EditElementHandleCR stmElm, DgnModelRefP modelRef);*/
//BentleyApi::Dgn::StandardUnit FindUnitNumber(double ratioToMeter);
//GCS GetModelGCS (DgnModelRefP modelRef);
//WString GetFileNameFor (const Bentley::TerrainModel::Element::IMrDTMDataRef& dataRef);
//bool IsProcessTerminatedByUser ();


/*int ConvertMrDTMtoFullResDTM(RefCountedPtr<BcDTM>&            singleResolutionDtm,
                                                  const std::vector<DPoint3d>&      regionPoints,
                                                  ElementAgendaCR                   agenda,
                                                  ElementHandleCR                      selectedElement,
                                                  bool                              applyClip,
                                                  DTMUnit                           DTMOutputUnit,
                                                  bool                              useFullResolution);

int ConvertMrDTMtoFullResDTM(RefCountedPtr<BcDTM>&            singleResolutionDtm,
                                                  const std::vector<DPoint3d>&      regionPoints,
                                                  ElementAgendaCR                   agenda,
                                                  ElementHandleCR                      selectedElement,
                                                  bool                              applyClip,
                                                  DTMUnit                           DTMOutputUnit);*/


//Transform transformFromStorageToActiveUOR(ElementHandleCR tmElementHandle);

struct ClipInfo
    {
    std::vector<DPoint3d> m_region;
    bool             m_isMask;
    };

/*StatusInt updateDTMPtrWithSTMDirect(DTMPtr& dtmPtr, const DPoint3d* regionPts, size_t nbPts, IScalableMeshPtr mrDTMPtr,
                                                         bool applyTriangulation, size_t& remainingPointCount, vector<ClipInfo> clips,
                                                         bool applyClip, int edgeMethod, DRange3d drange);

StatusInt updateDTMPtrWithSTMDirect(DTMPtr& dtmPtr, const DPoint3d* regionPts, size_t nbPts, IScalableMeshPtr mrDTMPtr,
                                                         bool applyTriangulation, size_t& remainingPointCount, vector<ClipInfo> clips,
                                                         bool applyClip, int edgeMethod, double edgeMethodLength, DRange3d drange, DgnGCSP destinationGCS);*/

//Transform transformFromStorageToActiveMeter(ElementHandleCR tmElementHandle);

//Transform transformFromModelRefToActiveMeter(DgnModelRefP modelRef);

/*StatusInt transformDTMFromModelGlobalToElementStorage     (BcDTM&         ibcdtm,
                                                                                ElementHandleCR    tmElementHandle);

/*StatusInt transformDTMFromStorageToActiveMeter(BcDTM&      ibcdtm,
                                                                    ElementHandleCR tmElementHandle);

/*StatusInt GetApproximationNbPtsNeedToExtract(ElementHandleCR                 eeh,
                                                                  const std::vector<DPoint3d>& regionPoints,
                                                                  unsigned int*                nbPoints);

StatusInt GetApproximationNbPtsNeedToExtract(ElementHandleCR                 eeh,
                                                                  const std::vector<DPoint3d>& regionPoints,
                                                                  unsigned int*                nbPointsForPointFeatures,
                                                                  unsigned int*                nbPointsForLinearFeatures);

StatusInt GetApproximationNbPtsNeedToExtract(IScalableMeshPtr                    mrDTMPtr,
                                                                  DgnGCSP                     destinationGCS,
                                                                  DRange3d                     drange,
                                                                  const std::vector<DPoint3d>& regionPointsInStorageCS,
                                                                  vector<ClipInfo>             clips,
                                                                  unsigned int*                nbPoints);

StatusInt GetApproximationNbPtsNeedToExtract(IScalableMeshPtr                    mrDTMPtr,
                                                                  DgnGCSP                     destinationGCS,
                                                                  DRange3d                     drange,
                                                                  const std::vector<DPoint3d>& regionPointsInStorageCS,
                                                                  vector<ClipInfo>             clips,
                                                                  unsigned int*                nbPointsForPointFeatures,
                                                                  unsigned int*                nbPointsForLinearFeatures);*/

//void GetFromModelRefToActiveTransform(TransformR fromModelRefToActiveTransform, DgnModelP modelRef);

//StatusInt transformDTMFromActiveMeterToUOR(BcDTM& ibcdtm, ElementHandleCR tmElementHandle);

//StatusInt transformDTMFromStorageToUOR(BcDTM& ibcdtm, ElementHandleCR tmElementHandle);

/*class NonUndoableTxn : DgnCacheTxn
    {
    public:
        NonUndoableTxn ()
            {
            m_oldTxn =  &ITxnManager::GetManager().SetCurrentTxn (*this);
            }

        ~NonUndoableTxn ()
            {
            ITxnManager::GetManager().SetCurrentTxn (*m_oldTxn);
            }

    private:
        ITxn*           m_oldTxn;
        bool            m_oldUndoRedoState;
    };*/

//void GetTransformForPoints(Transform& uorToMeter, Transform& meterToUor);

void AugmentRangeByFactor(DRange2d& range, const double& factor);