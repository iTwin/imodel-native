/*----------------------------------------------------------------------+
|
|   $Source: Tests/ATP/TiledTriangulation/MrDTMUtil.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
//#include "DcStmCorePCH.h" //always first
//#include "ScalableMeshATPPch.h"
#include "MrDTMUtil.h"
//#include <thread>
//#include <DgnPlatform/Tools/fileutil.h>
#include <DgnPlatform/DesktopTools/fileutil.h>
//#include <DgnGeoCoord/DgnGeoCoord.h>
//#include <DcInternal\DcStmCore\DGNModelGeoref.h>
//#include <DcInternal\DcStmCore\GeoDTMCore.h>
#undef static_assert

USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_BENTLEY
//USING_NAMESPACE_GEODTMAPP

//using BENTLEY_NAMESPACE_NAME::GeoCoordinates::DgnGCS;
//using BENTLEY_NAMESPACE_NAME::GeoCoordinates::DgnGCSPtr;

#define GeoCoord_MstnGCS_DLLNAME        "Bentley.MicroStation.GeoCoord.dll"
#define GeoCoord_basegeocoord_DLLNAME   "basegeocoord.dll"
#define GeoCoord_GCSDialog_DLLNAME      "GCSDialog.dll"

#define DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION        -1
#define NB_LINEAR_FEATURE_POINTS_REQUESTED_FULL_RESOLUTION UINT_MAX


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
WString Wrapper_GetDllPath(WChar const* dllName)
    {
    BeFileName expandedName;
    //From dynaload.c; we don't use it directly because it output an error message if not found
    if (!((util_findFile (NULL, &expandedName, dllName, NULL, NULL, UF_NO_CUR_DIR) != 0) &&
        (util_findFile (NULL, &expandedName, dllName, L"MS_MDL", NULL, UF_NO_CUR_DIR) != 0) &&
        (util_findFileInPath (&expandedName, const_cast<WChar *>(dllName), NULL) != 0) &&
        (util_findFile (NULL, &expandedName, dllName, NULL, NULL, UF_CUR_DIR_SWAP) != 0)) )
        {
        return expandedName;
        }
    return WString();
    }

static unsigned int s_maxNumberOfPointForLinearFeatures;
static double       s_pointFeatureDecimationFactor;

void AugmentExtentByFactor(DRange2d& extent, const double& factor)
    {
    const double distance = sqrt(pow(extent.low.x - extent.high.x,2)+pow(extent.low.y-extent.high.y,2));
    //const double distanceStep = 1.0;
    //int step = 1;

    const double scale = factor/100.0;
    extent.low.x = extent.low.x - distance*scale;
    extent.low.y = extent.low.y - distance*scale;
    extent.high.x = extent.high.x + distance*scale;
    extent.high.y = extent.high.y + distance*scale;
    }


//BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t getExtractedTMMaxPointCountFromConfig ()
    {
    static const uint64_t DEFAULT = 5000000;

    return DEFAULT;

    // NEEDS_WORK_SM : because return default is in the begining, I comment all other stuff, do we still need it ?
    /*WString cfgVarStr;
    if (SUCCESS != mdlSystem_getCfgVar (&cfgVarStr, L"STM_EXTRACTED_TM_MAX_POINT_COUNT"))
        return DEFAULT;

    std::wstringstream cfgVarStream(cfgVarStr.c_str());

    UInt64 value = DEFAULT;
    cfgVarStream >> value;

    // Maybe user should be notified for all these via a warning?
    if (cfgVarStream.fail())
        {
        assert(!"Error reading configuration variable!");
        return DEFAULT;
        }

    assert(cfgVarStream.eof() && !!"Configuration variable is expected to contain no more than a single integer!");

    static const UInt64 SYSTEMS_MAX_IN_MEMORY_POINT_COUNT = (numeric_limits<size_t>::max)() / (sizeof DPoint3d);

    if (SYSTEMS_MAX_IN_MEMORY_POINT_COUNT < value)
        {
        assert(!"Specified configuration variable exceed allowed system limit!");
        return SYSTEMS_MAX_IN_MEMORY_POINT_COUNT;
        }

    return value;*/
    }

//END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
//Copied from PointCloudQueryService function
/*void GetFromModelRefToActiveTransform(TransformR fromModelRefToActiveTransform, DgnModelP modelRef)
    {
    bsiTransform_initIdentity(&fromModelRefToActiveTransform);

    if (modelRef != ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef())
        {
        Transform fromRootToActive;

        //DgnAttachment* refP = mdlRefFile_getInfo(ACTIVEMODEL);
        DgnAttachment* refP = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->AsDgnAttachmentP();
        mdlRefFile_getTransformFromParent(&fromRootToActive, refP, 0);

        Transform fromElemModelRefToRoot;

        //refP = mdlRefFile_getInfo(modelRef);
        refP = modelRef->AsDgnAttachmentP();
        mdlRefFile_getTransformToParent(&fromElemModelRefToRoot, refP, 0);

        bsiTransform_multiplyTransformTransform(&fromModelRefToActiveTransform, &fromRootToActive, &fromElemModelRefToRoot);
        }
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t getExtractedTMMaxPointCount ()
    {
    // Lazily initialize our value and preserve it afterward.
    static const uint64_t VALUE = getExtractedTMMaxPointCountFromConfig();
    return VALUE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  10/2007
+---------------+---------------+---------------+---------------+---------------+------*/
/*bool GeoCoord_IsAvailable()
    {
    return GeoCoordinationManager::GetServices() != NULL;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
/*bool IsElemHandleReadOnly (ElementHandle eh)
    {
    // Must come from active model.
    if (!mdlModelRef_isActiveModel (eh.GetModelRef ()))
        return true;

    if (mdlModelRef_isReadOnly (eh.GetModelRef ()))
        return true;

    if (eh.GetElementCP()->ehdr.locked)
        return true;

    UInt32 level = elementRef_getElemLevel(eh.GetElementRef());
    if (mdlLevel_isElementLocked (eh.GetModelRef (), level))
        return true;

    return false;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  12/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void EnsureFolderSeparatorCharacterAtEnd (WStringR wFolderName)
    {
    if (wFolderName.empty ())
        {
        wFolderName.push_back (WCSDIR_SEPARATOR_CHAR);
        return;
        }

    WChar lastChar  = *wFolderName.rbegin ();
    if (WCSDIR_SEPARATOR_CHAR != lastChar && WCSALT_DIR_SEPARATOR_CHAR != lastChar)
        wFolderName.push_back (WCSDIR_SEPARATOR_CHAR);
    }

typedef StatusInt (*DcStmAppMsgLoaderFP)(WCharP pString, uint32_t listId, uint32_t stringNum);



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt LoadStringFromGEODTM(WCharP pString, UInt32 listId, UInt32 stringNum)
    {
    StatusInt status = ERROR;
    
    assert(GeoDTMCore::GetInstance().GetDcStmAppMsgLoaderCallback() != 0);

    if (GeoDTMCore::GetInstance().GetDcStmAppMsgLoaderCallback() != 0)
        {            
        status  = (*GeoDTMCore::GetInstance().GetDcStmAppMsgLoaderCallback())(pString, listId, stringNum);
        }

    return status; 
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt ExportSTMToDTM(EditElementHandleR dtmElm, EditElementHandleCR stmElm, DgnModelRefP modelRef)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    if (DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, stmElm) != SUCCESS)
        return ERROR;

    RefCountedPtr<IMrDTMDataRef> mrRef = (IMrDTMDataRef*)DTMDataRef.get();

    if (mrRef->GetMrDTMState() == SCM_STATE_EMPTY)
        return ERROR;

    IScalableMeshPtr iMrDtmPtr(dynamic_cast<IScalableMesh*>(DTMDataRef->GetDTMStorage(GetStatistics)));
    if (iMrDtmPtr == 0)
        return ERROR;

    const Bentley::WString fileStr(GetFileNameFor(*mrRef));

    UInt32  prefSizeInMB = 5;
    WString defaultSizeInMB;    

    if ((mdlSystem_getCfgVar (&defaultSizeInMB, L"STM_DOWNSAMPLING_PREFSIZE") == SUCCESS) && (_wtoi(defaultSizeInMB.c_str()) > 0))
        prefSizeInMB = _wtoi(defaultSizeInMB.c_str());

    UInt32 prefSizeInBytes  = prefSizeInMB * 1024 * 1024;
    UInt64 nbPoints = iMrDtmPtr->GetPointCount();

    struct __stat64 st;
    _wstat64(fileStr.c_str(), &st);
    long outPoints = (long)(nbPoints * prefSizeInBytes / st.st_size);

    if (outPoints > (long)nbPoints)
        outPoints = (long)nbPoints;

    DTMDataRef = 0;
    mrRef = 0;

    // convert it in single Res DTM    
    return DTMElementHandlerManager::ConvertMrDTMtoSingleResDTM(dtmElm, stmElm, outPoints, modelRef);
    }

StatusInt ExportSTMToDTM(EditElementHandleR stmElm, DgnModelRefP modelRef)
    {
    EditElementHandle dtmElm;

    StatusInt status = ExportSTMToDTM(dtmElm, stmElm, modelRef);

    if (status == SUCCESS)
        {
        dtmElm.AddToModel();  
        dtmElm.ReplaceInModel(dtmElm.GetElementRef());  
        stmElm.DeleteFromModel ();
        }        
    
    return status;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*BentleyApi::Dgn::StandardUnit FindUnitNumber(double ratioToMeter)
    {    
    BentleyApi::Dgn::StandardUnit unitNumber = BentleyApi::Dgn::StandardUnit::None;

    UnitIteratorOptionsPtr pUnitIterOptions(UnitIteratorOptions::Create());

    //  First get everything in the English
    pUnitIterOptions->SetAllowSingleBase(UnitBase::Meter);
    pUnitIterOptions->SetAllowSingleSystem(UnitSystem::English);

    UserUnitCollection englishUnits(*pUnitIterOptions);
    
    for (UserUnitCollection::Entry const& unit : englishUnits)
        {
        //! Get the denominator of this Unit.  The numerator divided by the denominator is the scale
        //! factor between this unit and its base.            
        if (LegacyMath::DEqual(ratioToMeter, unit.GetUnitDef().GetDenominator () / unit.GetUnitDef().GetNumerator ()))
            {            
            return unit.GetNumber ();
            }            
        }
    
    //  Now get everything in the Metric
    pUnitIterOptions->SetAllowSingleBase(UnitBase::Meter);
    pUnitIterOptions->SetAllowSingleSystem(UnitSystem::Metric);

    UserUnitCollection metricUnits(*pUnitIterOptions);

    for (UserUnitCollection::Entry const& unit : metricUnits)
        {
        //! Get the denominator of this Unit.  The numerator divided by the denominator is the scale
        //! factor between this unit and its base.            
        if (LegacyMath::DEqual(ratioToMeter, unit.GetUnitDef().GetDenominator () / unit.GetUnitDef().GetNumerator ()))
            {            
            return unit.GetNumber ();
            }            
        }

    //  Now get everything in the US Survey Foot
    pUnitIterOptions->SetAllowSingleBase(UnitBase::Meter);
    pUnitIterOptions->SetAllowSingleSystem(UnitSystem::USSurvey);

    UserUnitCollection usSurveyUnits(*pUnitIterOptions);

    for (UserUnitCollection::Entry const& unit : usSurveyUnits)
        {
        //! Get the denominator of this Unit.  The numerator divided by the denominator is the scale
        //! factor between this unit and its base.            
        if (LegacyMath::DEqual(ratioToMeter, unit.GetUnitDef().GetDenominator () / unit.GetUnitDef().GetNumerator ()))
            {            
            return unit.GetNumber ();
            }            
        }

    return unitNumber;
    }*/

inline const TransfoMatrix& FromBSI (const Transform& transform)
    {
    return reinterpret_cast<const TransfoMatrix&>(transform);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Raymond.Gauthier    09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*LocalTransform GetRootMasterToRefUorTransform (DgnModelRef* modelRef)
    {
    // Get [ROOT uor -> ROOT master] scale
    Transform rootUorToRootMasterTransform;
    const double masterUnitPerUor = 1.0/mdlModelRef_getUorPerMaster(modelRef->GetRoot());
    mdlTMatrix_getIdentity(&rootUorToRootMasterTransform);
    mdlTMatrix_scale(&rootUorToRootMasterTransform, &rootUorToRootMasterTransform, masterUnitPerUor, masterUnitPerUor, masterUnitPerUor);

    // Get [Reference uor -> ROOT uor] transform
    Transform refUorToRootMasterTransform;
    
    mdlRefFile_getTransformToParent(&refUorToRootMasterTransform, modelRef->AsDgnAttachmentCP(), NULL);
    
    // Produce [Reference uor -> ROOT master] transform by including [ROOT uor -> ROOT master] scaling
    // to [Reference uor -> ROOT uor] transform
    bsiTransform_multiplyTransformTransform(&refUorToRootMasterTransform, &rootUorToRootMasterTransform, &refUorToRootMasterTransform);

    return LocalTransform::CreateFromToGlobal(TransfoModel::CreateAffineFrom(FromBSI(refUorToRootMasterTransform)));
    }*/

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                Jean-Francois.Cote   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/*GCS GetModelGCS (DgnModelRefP modelRef)
    {
    UnitInfo masterUnit;

    DgnModelRefP rootModelRef = (modelRef->IsDgnAttachment()) ? modelRef->GetRoot() : modelRef;

    const DgnGCSPtr modelGCSPtr = DgnGCS::FromModel(rootModelRef, true);
    if(0 != modelGCSPtr.get())
        {
        return GetGCSFactory().Create(modelGCSPtr.get(),
            GetRootMasterToRefUorTransform(modelRef));
        }

    if(0 == mdlModelRef_getMasterUnit(rootModelRef, &masterUnit))
        {
        DgnPlatform::UnitBase unitBase = mdlUnits_getUnitBase(&masterUnit);
        if(UnitBase::Meter == unitBase)
            return GetGCSFactory().Create(Unit::CreateLinearFrom(masterUnit.label, (masterUnit.denominator / masterUnit.numerator)),
            GetRootMasterToRefUorTransform(modelRef));

        if(UnitBase::Degree == unitBase)
            return GetGCSFactory().Create(Unit::CreateFromDegreeBased(masterUnit.label, (masterUnit.denominator / masterUnit.numerator)),
            GetRootMasterToRefUorTransform(modelRef));
        }

    assert(!"Unexpected");
    return GCS::GetNull();
    }*/







int transformDTMCallback(DPoint3d* pts, size_t numPoints, void* userP)
    {
    assert(0 != userP);

    const TransfoModel& transform = *(const TransfoModel*)userP;

    if (TransfoModel::S_SUCCESS != transform.Transform(pts, numPoints, pts))
        return BSIERROR;

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt transformDTMFromModelGlobalCSToRootGlobalCS (IDTM& dtm, ElementHandleCR stmElement)
    {
    USING_NAMESPACE_GEODTMAPP

    BcDTM& ibcdtm = *dtm.GetBcDTM();

    Transform stmStorageToUOR;
    DTMElementHandlerManager::GetStorageToUORMatrix(stmStorageToUOR, stmElement);

    TransfoModel stmStorageToRootGlobal(TransfoModel::CreateFrom(FromBSITransform(stmStorageToUOR)));
    if (TransfoModel::S_SUCCESS != stmStorageToRootGlobal.Append(GetModelDesignToGlobalTransfoModel(stmElement.GetModelRef())) ||
        TransfoModel::S_SUCCESS != stmStorageToRootGlobal.Append(GetModelGlobalToRootGlobalTransfoModelWReprojSupport(stmElement.GetModelRef())))
        return BSIERROR;

    ibcdtm.SetMemoryAccess (DTMAccessMode::Write);
    return ibcdtm.TransformUsingCallback (&transformDTMCallback, const_cast<TransfoModel*>(&stmStorageToRootGlobal));
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt appendDTMToExisting (IDTM& existingDTM, const IDTM& dtmToAppend, bool tmsTriangulated)
    {
    BcDTM& existingIBcDTM = *existingDTM.GetBcDTM();
    BcDTM& ibcdtmToAppend = *const_cast<IDTM&>(dtmToAppend).GetBcDTM();

    if (tmsTriangulated)
        {
        assert(DTMState::Tin == existingIBcDTM.GetDTMState() && DTMState::TinError == existingIBcDTM.GetDTMState() &&
               DTMState::Tin == ibcdtmToAppend.GetDTMState() && DTMState::TinError == ibcdtmToAppend.GetDTMState());

        return existingIBcDTM.Merge(ibcdtmToAppend);
        }

    assert(DTMState::Tin != existingIBcDTM.GetDTMState() && DTMState::TinError != existingIBcDTM.GetDTMState() &&
           DTMState::Tin != ibcdtmToAppend.GetDTMState() && DTMState::TinError != ibcdtmToAppend.GetDTMState());

    return existingIBcDTM.Append(ibcdtmToAppend);
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt transformDTMFromStorageToActiveMeter(BcDTM& ibcdtm, ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform  fromStorageToActiveInMeters = transformFromStorageToActiveMeter(tmElementHandle);

    TransfoModel fromStorageToActiveInMetersTransfo(TransfoModel::CreateFrom(FromBSITransform(fromStorageToActiveInMeters)));

    ibcdtm.SetMemoryAccess (DTMAccessMode::Write);
    return ibcdtm.TransformUsingCallback (&transformDTMCallback, const_cast<TransfoModel*>(&fromStorageToActiveInMetersTransfo));
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Chantal.Poulin   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt transformDTMFromActiveMeterToUOR(BcDTM& ibcdtm, ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    const double invUorPerMeter = 1.0/mdlModelRef_getUorPerMeter(ACTIVEMODEL);

    Transform uorToMeter;
    bsiTransform_initFromRowValues(&uorToMeter, invUorPerMeter, 0.0, 0.0, 0.0, 0.0, invUorPerMeter, 0.0, 0.0, 0.0, 0.0, invUorPerMeter, 0.0);

    Transform meterToUor;
    bsiTransform_invertTransform(&meterToUor, &uorToMeter);

    DPoint3d translation;
    StatusInt status = mdlModelRef_getGlobalOrigin(ACTIVEMODEL, &translation);
    assert(status == SUCCESS);

    Transform fromActiveModelToActiveModelGlobalOrigin;
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, translation.x, 0.0, 1.0, 0.0, translation.y, 0.0, 0.0, 1.0, translation.z);

    bsiTransform_multiplyTransformTransform(&meterToUor, &fromActiveModelToActiveModelGlobalOrigin, &meterToUor);

    Transform fromModelRefToActiveTransform;
    Transform invertFromModelRefToActiveTransform;

    GetFromModelRefToActiveTransform(fromModelRefToActiveTransform, tmElementHandle.GetModelRef());

    bool invertSuccess = bsiTransform_invertTransform(&invertFromModelRefToActiveTransform, &fromModelRefToActiveTransform);
    assert(invertSuccess != 0);

    bsiTransform_multiplyTransformTransform(&meterToUor, &invertFromModelRefToActiveTransform, &meterToUor);

    mdlModelRef_getGlobalOrigin(tmElementHandle.GetModelRef(), &translation);
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, -translation.x, 0.0, 1.0, 0.0, -translation.y, 0.0, 0.0, 1.0, translation.z);
    bsiTransform_multiplyTransformTransform(&meterToUor, &fromActiveModelToActiveModelGlobalOrigin, &meterToUor);

    TransfoModel fromActiveMeterToActiveUORTransfo(TransfoModel::CreateFrom(FromBSITransform(meterToUor)));

    ibcdtm.SetMemoryAccess (DTMAccessMode::Write);
    return ibcdtm.TransformUsingCallback (&transformDTMCallback, const_cast<TransfoModel*>(&fromActiveMeterToActiveUORTransfo));
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Chantal.Poulin   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt transformDTMFromStorageToUOR(BcDTM& ibcdtm, ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform fromStorageToUOR;
    DTMElementHandlerManager::GetStorageToUORMatrix(fromStorageToUOR, tmElementHandle);

    DPoint3d translation;
    mdlModelRef_getGlobalOrigin(tmElementHandle.GetModelRef(), &translation);

    Transform fromActiveModelToActiveModelGlobalOrigin;
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, -translation.x, 0.0, 1.0, 0.0, -translation.y, 0.0, 0.0, 1.0, translation.z);

    bsiTransform_multiplyTransformTransform(&fromStorageToUOR, &fromActiveModelToActiveModelGlobalOrigin, &fromStorageToUOR);

    TransfoModel fromStorageToUORTransfo(TransfoModel::CreateFrom(FromBSITransform(fromStorageToUOR)));

    ibcdtm.SetMemoryAccess (DTMAccessMode::Write);
    return ibcdtm.TransformUsingCallback (&transformDTMCallback, const_cast<TransfoModel*>(&fromStorageToUORTransfo));
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt updateDTMPtrWithSTM  (DTMPtr& dtmPtr, const DPoint3d* regionPts, size_t nbPts, ElementHandleCR eeh,
                                size_t& remainingPointCount, bool applyClip, DTMUnit DTMOutputUnit,
                                double decimationFactorForPointFeatures, unsigned int maximumNbLinearFeaturePoints)
    {

    //#error badRemaningPointCount

    static const bool tmTriangulated = false;

    DTMPtr tmpDtmPtr;

    const StatusInt status = updateDTMPtrWithSTMDirect(tmpDtmPtr, regionPts, nbPts, eeh,
                                                       tmTriangulated, remainingPointCount, applyClip,
                                                       decimationFactorForPointFeatures, maximumNbLinearFeaturePoints);

    if (BSISUCCESS != status || NULL == &*tmpDtmPtr)
        return status;

    switch(DTMOutputUnit)
        {
        case DTMUnit_RootGlobalCS:
            {
            if (BSISUCCESS != transformDTMFromModelGlobalCSToRootGlobalCS(*tmpDtmPtr, eeh))
                return BSIERROR;
            }
            break;

        case DTMUnit_ActiveMeter:
            {
            if (BSISUCCESS != transformDTMFromStorageToActiveMeter(*tmpDtmPtr->GetBcDTM(), eeh))
                return BSIERROR;
            }
            break;
        }

    if (NULL == &*dtmPtr)
        {
        swap(dtmPtr, tmpDtmPtr);
        return BSISUCCESS;
        }

    if (BSISUCCESS != appendDTMToExisting(*dtmPtr, *tmpDtmPtr, tmTriangulated))
        return BSIERROR;

    return BSISUCCESS;
    }*/



enum TriangulationEdgeOption
    {
    TRIANGULATION_EDGE_OPTION_USE_DEFAULT,
    TRIANGULATION_EDGE_OPTION_DO_NOT_REMOVE_ANY_TIN_HULL_TRIANGLE,
    TRIANGULATION_EDGE_OPTION_REMOVE_SLIVER_TRIANGLE_ON_TIN_HULL,
    TRIANGULATION_EDGE_OPTION_REMOVE_TRIANGLE_ON_TIN_HULL_WITH_SIDE_LENGTH_GREATHER_THEN_MAX_SIDE,
    };

/*---------------------------------------------------------------------------------**//**
* NTERAY:   Copy of DTM\bclib\sources\unmanaged\dll\DTM\MrDTM\MrDTMQuery.cpp
*           TriangulateDTM function.
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt triangulateDTM   (DTMPtr&                             dtmPtr,
                            TriangulationEdgeOption             edgeOption,
                            double                              maxTriangleSideLength,
                            checkTriangulationStopCallbackFP    checkTriangulationStopCallbackFnP)
    {
    assert((dtmPtr != 0) && (dtmPtr->GetBcDTM() != 0) && (dtmPtr->GetBcDTM()->GetTinHandle()));

    BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());

    // We should not try to triangulate DTMs with less than 3 points
    // as the bc triangulation function will return an error.
    if (dtmObjP->numPoints < 3)
        return BSISUCCESS;

    // See \DTM\bclib\sources\unmanaged\dll\DTM\MrDTM\MrDTMQuery.cpp IMrDTMQueryParameters::m_p2pTolerance.
    static const double TRIANGULATION_TOLERANCE = 0.0000001;

    bcdtmObject_setTriangulationParametersDtmObject(dtmObjP,
                                                    TRIANGULATION_TOLERANCE,
                                                    TRIANGULATION_TOLERANCE,
                                                    edgeOption,
                                                    maxTriangleSideLength);

    if (NULL != checkTriangulationStopCallbackFnP)
        {
        // NTERAY: Shouldn't this state be restored as it was after completion?
        const StatusInt status = bcdtmTin_setTriangulationCheckStopCallBackFunction(dtmObjP, checkTriangulationStopCallbackFnP);
        assert(BSISUCCESS == status);
        }
    const StatusInt status = bcdtmObject_triangulateDtmObject(dtmObjP);
    
    if (status == 0)
        {
        RefCountedPtr<BcDTM> bcDtmObjPtr;

        bcDtmObjPtr = BcDTM::CreateFromDtmHandle(dtmObjP);        
        
        dtmPtr = bcDtmObjPtr.get();
        }

    return status;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt getTriangulationParameters   (ElementHandleCR                eh,
                                        TriangulationEdgeOption&    edgeOption,
                                        double&                     maxTriangleSideLength)
    {
    // Will trigger the use of default values
    edgeOption = TRIANGULATION_EDGE_OPTION_USE_DEFAULT;
    maxTriangleSideLength = -1.0;

    RefCountedPtr<DTMDataRef> dtmRef;
    if (BSISUCCESS != DTMElementHandlerManager::GetDTMDataRef (dtmRef, eh))
        return BSIERROR;

    IMrDTMDataRef& mrdtmRef = static_cast<IMrDTMDataRef&>(*dtmRef);
    if (SCM_STATE_EMPTY == mrdtmRef.GetMrDTMState())
        return BSIERROR;

    IScalableMeshPtr mrdtmPtr(dynamic_cast<IScalableMesh*>(dtmRef->GetDTMStorage(GetMrDtm)));
    if (NULL == &*mrdtmPtr)
        return BSIERROR;

    int edgeMethod;
    mrdtmRef.GetEdgeMethod(edgeMethod);

    assert(0 <= edgeMethod && 3 >= edgeMethod);

    edgeOption = static_cast<TriangulationEdgeOption>(edgeMethod);
    maxTriangleSideLength = mrdtmRef.GetEdgeMethodLengthInStorage();

    return BSISUCCESS;
    }*/



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt addClipToRootGlobalClipContainer(IScalableMeshClipContainerPtr& clips,
                                           ElementHandleCR         eh,
                                           DTMUnit                 dtmUnit)
    {
    assert(clips != 0);

    USING_NAMESPACE_GEODTMAPP

    RefCountedPtr<DTMDataRef> dtmRefPtr;
    if (BSISUCCESS != DTMElementHandlerManager::GetDTMDataRef (dtmRefPtr, eh))
        return BSIERROR;

    IMrDTMDataRef& stmRef = static_cast<IMrDTMDataRef&>(*dtmRefPtr);

    if (!stmRef.GetClipActivation() || 0 == stmRef.GetNbClips())
        return BSISUCCESS;

    TransfoModel transfoToDTMUnit(TransfoModel::GetIdentity());

    switch (dtmUnit)
        {
        case DTMUnit_RootGlobalCS:
            {
            transfoToDTMUnit = GetModelDesignToGlobalTransfoModel(eh.GetModelRef());
            if (TransfoModel::S_SUCCESS != transfoToDTMUnit.Append(GetModelGlobalToRootGlobalTransfoModelWReprojSupport(eh.GetModelRef())))
                return BSIERROR;
            }
            break;

        case DTMUnit_ActiveMeter:
            {
            Transform fromModelRefToActive;
            fromModelRefToActive = transformFromModelRefToActiveMeter(eh.GetModelRef());
            TransfoMatrix transfoMatrix(fromModelRefToActive.form3d);
            transfoToDTMUnit = TransfoModel::CreateFrom(transfoMatrix);
            }
            break;

        default:
            assert(!"Unknown unit");
            break;
        };

    IScalableMeshClipContainerPtr clipsInModelDesignCS(IScalableMeshClipContainer::Create());
    for (int clipIdx = 0; clipIdx < stmRef.GetNbClips(); ++clipIdx)
        {
        DPoint3d*   clipPointsP = 0;
        int         numberOfClipPoints = 0;
        bool        isClipMask = false;

        StatusInt getClipStatus = stmRef.GetClip(clipPointsP, numberOfClipPoints, isClipMask, clipIdx);
        if (BSISUCCESS != getClipStatus || 0 == clipPointsP)
            {
            delete clipPointsP;
            assert(!"We may have skipped a clip item.");
            continue;
            }

        IScalableMeshClipInfoPtr clipInfoPtr(IScalableMeshClipInfo::Create(clipPointsP, numberOfClipPoints, isClipMask));

        clipsInModelDesignCS->AddClip(clipInfoPtr);        

        delete clipPointsP;
        }
    
    for (size_t clipInd = 0; clipInd < clipsInModelDesignCS->GetNbClips(); clipInd++)
        {
        IScalableMeshClipInfoPtr clipInfoPtr;
        
        clipsInModelDesignCS->GetClip(clipInfoPtr, clipInd);

        // TDORAY:  This is not the way we should transform the clips. Clips should be intersected with GeoCS domains
        //          and points should be added for transform that don't preserve lines. Could not perform as would
        //          be appropriate because domain description could not be accessed from Descartes. See what is done
        //          in the re-projection query's add clip.

        
        if (TransfoModel::S_SUCCESS != transfoToDTMUnit.Transform(clipInfoPtr->GetClipPoints(), clipInfoPtr->GetNbClipPoints(), clipInfoPtr->GetClipPoints()))
            return BSIERROR;

        clips->AddClip(clipInfoPtr);
        }

    return BSISUCCESS;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt addClipsToRootGlobalClipContainer(IScalableMeshClipContainerPtr& clips,
                                                ElementAgendaCR         agenda,
                                                DTMUnit                 dtmUnit)
    {
    for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
        {
        if (BSISUCCESS != addClipToRootGlobalClipContainer(clips, *elemIter, dtmUnit))
            return BSIERROR;
        }

    return BSISUCCESS;

    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt intersectClipWithAreaRegion(IScalableMeshClipContainerPtr&      clips,
                                        const HFCPtr<HVE2DShape>&    regionShape,
                                        const HFCPtr<HGF2DCoordSys>& coordSys)
    {
    assert(regionShape->IsCompatibleWith(HVE2DPolygonOfSegments::CLASS_ID) == true);
    assert(clips != 0);

    IScalableMeshClipContainerPtr intersectedClips(IScalableMeshClipContainer::Create());
    
    HFCPtr<HVE2DSimpleShape> regionExtentShape(new HVE2DRectangle(regionShape->GetExtent()));
    IScalableMeshClipInfoPtr clipInfoPtr;

    for (size_t clipInd = 0; clipInd < clips->GetNbClips(); clipInd++)
        {
        clips->GetClip(clipInfoPtr, clipInd);
        
        if (clipInfoPtr->IsClipMask() == true)
            {
            intersectedClips->AddClip(clipInfoPtr);
            }
        else
            {
            size_t nVertices =  clipInfoPtr->GetNbClipPoints() * 2;

            HArrayAutoPtr<double> tempBuffer(new double[nVertices]);

            for (size_t ptIndex = 0; ptIndex < clipInfoPtr->GetNbClipPoints(); ptIndex++)
                {
                tempBuffer[ptIndex * 2]     = clipInfoPtr->GetClipPoints()[ptIndex].x;
                tempBuffer[ptIndex * 2 + 1] = clipInfoPtr->GetClipPoints()[ptIndex].y;
                }

            HFCPtr<HVE2DShape> clipShape;

            clipShape = new HVE2DPolygonOfSegments(nVertices, tempBuffer, coordSys);
            clipShape = clipShape->IntersectShape(*regionShape);

            assert(clipShape->HasHoles() == false);

            if (clipShape->IsComplex() == true)
                {
                HVE2DShape::ShapeList::const_iterator shapeIter(clipShape->GetShapeList().begin());
                HVE2DShape::ShapeList::const_iterator shapeIterEnd(clipShape->GetShapeList().end());

                double                  tolerance = 0;
                HGF2DLocationCollection shapePoints;

                while (shapeIter != shapeIterEnd)
                    {
                    (*shapeIter)->Drop(&shapePoints, tolerance);
                    vector<DPoint3d> shapePoints3D(shapePoints.size());

                    for (size_t pointInd = 0; pointInd < shapePoints.size(); pointInd++)
                        {
                        shapePoints3D[pointInd].x = shapePoints[pointInd].GetX();
                        shapePoints3D[pointInd].y = shapePoints[pointInd].GetY();
                        shapePoints3D[pointInd].z = 0;
                        }

                    shapePoints.clear();

                    IScalableMeshClipInfoPtr clipInfoPtr(IScalableMeshClipInfo::Create(const_cast<DPoint3d*>(&shapePoints3D[0]), shapePoints3D.size(), false));

                    intersectedClips->AddClip(clipInfoPtr);
                    shapeIter++;
                    }
                }
            else
            if (clipShape->IsEmpty() == false)
                {
                double                  tolerance = 0;
                HGF2DLocationCollection shapePoints;

                clipShape->Drop(&shapePoints, tolerance);

                vector<DPoint3d> shapePoints3D(shapePoints.size());

                for (size_t pointInd = 0; pointInd < shapePoints.size(); pointInd++)
                    {
                    shapePoints3D[pointInd].x = shapePoints[pointInd].GetX();
                    shapePoints3D[pointInd].y = shapePoints[pointInd].GetY();
                    shapePoints3D[pointInd].z = 0;
                    }

                shapePoints.clear();

                IScalableMeshClipInfoPtr clipInfoPtr(IScalableMeshClipInfo::Create(const_cast<DPoint3d*>(&shapePoints3D[0]), shapePoints3D.size(), false));

                intersectedClips->AddClip(clipInfoPtr);
                }
            }        
        }
    
    if (intersectedClips->GetNbClips() > 0)
        {
        clips = intersectedClips;
        }
    else
        {
        clips = IScalableMeshClipContainer::Create();
        }

    return SUCCESS;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*    StatusInt addClipsToClipContainer(IScalableMeshClipContainerPtr&         clips,
                                        ElementAgendaCR                 agenda,
                                        const vector<DPoint3d>&         regionPointsInRootUOR,
                                        bool                            applyClip,
                                        DTMUnit                         dtmUnit)
    {
    assert(clips != 0);
    
    ElementHandleCR lastElementHandle = agenda.back();
    DgnModelRefP rootModelRef = lastElementHandle.GetModelRef()->GetRoot();

    if (BSISUCCESS != addClipsToRootGlobalClipContainer(clips, agenda, dtmUnit))
        return BSIERROR;

    if (applyClip)
        {
        vector<DPoint3d> regionPointsInRootGlobal(regionPointsInRootUOR.size());

        TransfoModel transfoToDtmUnit(TransfoModel::GetIdentity());

        switch (dtmUnit)
            {
            case DTMUnit_RootGlobalCS:
                {
                transfoToDtmUnit = GetModelDesignToGlobalTransfoModel(rootModelRef);
                }
                break;

            case DTMUnit_ActiveMeter:
                {
                DPoint3d translation;
                StatusInt status = mdlModelRef_getGlobalOrigin(ACTIVEMODEL, &translation);
                assert(status == SUCCESS);

                double meterPerUorActiveModel = 1 / mdlModelRef_getUorPerMeter(ACTIVEMODEL);

                Transform transfo;
                bsiTransform_initFromRowValues(&transfo, meterPerUorActiveModel, 0.0, 0.0, -translation.x * meterPerUorActiveModel,
                                                         0.0, meterPerUorActiveModel, 0.0, -translation.y * meterPerUorActiveModel,
                                                         0.0, 0.0, meterPerUorActiveModel, -translation.z * meterPerUorActiveModel);

                TransfoMatrix transfoMatrix(transfo.form3d);
                transfoToDtmUnit = TransfoModel::CreateFrom(transfoMatrix);
                }
                break;

            default:
                assert(!"Unknown unit");
                break;
            };


        transfoToDtmUnit.Transform(&regionPointsInRootUOR[0], regionPointsInRootUOR.size(), &regionPointsInRootGlobal[0]);

        size_t nVertices = regionPointsInRootGlobal.size() * 2;

        HFCPtr<HGF2DCoordSys> coordSys(new HGF2DCoordSys());

        HArrayAutoPtr<double> tempBuffer(new double[nVertices]);

        for (size_t ptIndex = 0 ; ptIndex < regionPointsInRootGlobal.size(); ptIndex++)
            {
            tempBuffer[ptIndex * 2]     = regionPointsInRootGlobal[ptIndex].x;
            tempBuffer[ptIndex * 2 + 1] = regionPointsInRootGlobal[ptIndex].y;
            }

        HFCPtr<HVE2DShape> regionShape(new HVE2DPolygonOfSegments(nVertices, tempBuffer, coordSys));

        if (clips->GetNbClips() > 0)
            {
            intersectClipWithAreaRegion(clips, regionShape, coordSys);

            //Region doesn't intersect clip on STM.            
            if (clips->GetNbClips() == 0)
                {
                return BSIERROR;
                }
            }
        else
            {   
            IScalableMeshClipInfoPtr clipInfoPtr(IScalableMeshClipInfo::Create(const_cast<DPoint3d*>(&regionPointsInRootGlobal[0]), regionPointsInRootGlobal.size(), false));

            clips->AddClip(clipInfoPtr);
            }
        }

    return BSISUCCESS;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
#if 0 //TR 352836 - WIP
static bool s_newClip = true;
#endif

/*StatusInt triangulateDTMAndApplyClips  (DTMPtr&                       dtmPtr,
                                        ElementHandleCR               eh,
                                        const IScalableMeshClipContainerPtr& clips)
    {
    assert(clips != 0);

    TriangulationEdgeOption edgeOption;
    double maxTriangleSideLength;

    if (BSISUCCESS != getTriangulationParameters(eh, edgeOption, maxTriangleSideLength))
        return BSIERROR;

    // Trying to replicate original behavior found in DTM\bclib\sources\unmanaged\dll\DTM\MrDTM\MrDTMQuery.cpp reprojection query.
    TriangulationEdgeOption postApplyClipEdgeOption = static_cast<TriangulationEdgeOption>(static_cast<UInt>(edgeOption) + 1);
    assert(postApplyClipEdgeOption <= 3);

    /*
    TRIANGULATION_EDGE_OPTION_USE_DEFAULT,
    TRIANGULATION_EDGE_OPTION_DO_NOT_REMOVE_ANY_TIN_HULL_TRIANGLE,
    TRIANGULATION_EDGE_OPTION_REMOVE_SLIVER_TRIANGLE_ON_TIN_HULL,
    TRIANGULATION_EDGE_OPTION_REMOVE_TRIANGLE_ON_TIN_HULL_WITH_SIDE_LENGTH_GREATHER_THEN_MAX_SIDE,
    */
/*    edgeOption = TRIANGULATION_EDGE_OPTION_DO_NOT_REMOVE_ANY_TIN_HULL_TRIANGLE;
    postApplyClipEdgeOption = TRIANGULATION_EDGE_OPTION_DO_NOT_REMOVE_ANY_TIN_HULL_TRIANGLE;

#if 0 //TR 352836 - WIP
    if (s_newClip)
        {
        if (BSISUCCESS != triangulateDTM(dtmPtr, edgeOption, maxTriangleSideLength, CheckTriangulationStopCallback))
            return BSIERROR;

        if (!clips.empty())
            {
            MrDTMClipContainer::const_iterator     clipIter(clips.begin());
            MrDTMClipContainer::const_iterator     clipIterEnd(clips.end());
            vector<HFCPtr<HVE2DPolygonOfSegments>> polygons;

            HFCPtr<HGF2DCoordSys> coordSys(new HGF2DCoordSys());

            while (clipIter != clipIterEnd)
                {
                DPoint3d* regionPts = new DPoint3d[clipIter->m_points.size()];

                size_t nVertices =  clipIter->m_points.size() * 2;
                HArrayAutoPtr<double> tempBuffer(new double[nVertices]);

                for (size_t pointInd = 0; pointInd < clipIter->m_points.size(); pointInd++)
                    {
                    regionPts[pointInd].X = clipIter->m_points[pointInd].x;
                    regionPts[pointInd].Y = clipIter->m_points[pointInd].y;
                    regionPts[pointInd].Z = clipIter->m_points[pointInd].z;

                    tempBuffer[pointInd * 2]     = clipIter->m_points[pointInd].x;
                    tempBuffer[pointInd * 2 + 1] = clipIter->m_points[pointInd].y;
                    }

                polygons.push_back(new HVE2DPolygonOfSegments(nVertices,
                                                              tempBuffer,
                                                              coordSys));

                int          status;
                DTMUserTag userTag = 0;
                DTMFeatureId* textureRegionIdsP = 0;
                long            numRegionTextureIds;

                status = bcdtmInsert_internalDtmFeatureMrDtmObject(dtmPtr->GetBcDTM()->GetTinHandle(),
                                                                   DTMFeatureType::Region   ,
                                                                   1,
                                                                   2,
                                                                   userTag,
                                                                   &textureRegionIdsP,
                                                                   &numRegionTextureIds,
                                                                   regionPts,
                                                                   clipIter->m_points.size());

                if ((numRegionTextureIds == 0) || (status != SUCCESS))
                    {
                    assert(0);
                    break;
                    }

                clipIter++;
                }

            if (clipIter == clipIterEnd)
                {
                long firstCall = true;

                long maxMeshSize = ULONG_MAX;

                IBcDTMMesh* pMesh = dtmPtr->GetBcDTM()->getMesh(firstCall, maxMeshSize, 0, 0);

                assert(pMesh != 0);

                const int numFaces = pMesh->GetFaceCount();

                IBcDTMMeshFacePtr pFace;

                vector<DPoint3d> triangles;

                    for (int faceInd = 0; faceInd < numFaces; faceInd++)
                    {
                    pFace = GetMeshFacePtr(pMesh->GetFace(faceInd));

                    DPoint3d pointToCheck;

                    pointToCheck.sumOf(&pFace->GetCoordinates(0), &pFace->GetCoordinates(1));
                    pointToCheck.sumOf(&pointToCheck, &pFace->GetCoordinates(2));
                    pointToCheck.scale(1.0/3.0);

                    HGF2DLocation locationToCheck(pointToCheck.x, pointToCheck.y, coordSys);

                    vector<HFCPtr<HVE2DPolygonOfSegments>>::const_iterator polygonIter(polygons.begin());
                    vector<HFCPtr<HVE2DPolygonOfSegments>>::const_iterator polygonIterEnd(polygons.end());

                    while (polygonIter != polygonIterEnd)
                        {
                        if ((*polygonIter)->IsPointIn(locationToCheck))
                            {
                            triangles.push_back(pFace->GetCoordinates(0));
                            triangles.push_back(pFace->GetCoordinates(1));
                            triangles.push_back(pFace->GetCoordinates(2));
                            break;
                            }

                        polygonIter++;
                        }
                    }

                // First clear the DTM
                CreateBcDTM(dtmPtr);

                //MST TBD - What should we do for triangles of 0 size.
                if (triangles.size() > 0)
                    {
                    int status = InsertTrianglesAsFeatureInDTM(dtmPtr, triangles, DTMF_GRAPHIC_BREAK);

                    if (status == SUCCESS)
                        {
                        BC_DTM_OBJ* dtmObjP(dtmPtr->GetBcDTM()->GetTinHandle());
                        status = bcdtmObject_triangulateStmTrianglesDtmObject(dtmObjP);
                        }

                    //MST TBD - What should we do if failure.
                    assert(status == SUCCESS);
                    }
                }
            }
        }
    else
#endif
        {

        if (clips->GetNbClips() > 0)
            {
            DRange3d dtmRange;
            if (BSISUCCESS != dtmPtr->GetRange(dtmRange))
                return BSIERROR;

            if (BSISUCCESS != triangulateDTM(dtmPtr, edgeOption, maxTriangleSideLength, CheckTriangulationStopCallback))
                    return BSIERROR;

            if (BSISUCCESS != SetClipsToDTM(dtmPtr, dtmRange, clips))
                return BSIERROR;
            }

        if (BSISUCCESS != triangulateDTM(dtmPtr, postApplyClipEdgeOption, maxTriangleSideLength, CheckTriangulationStopCallback))
            return BSIERROR;
        }



    return BSISUCCESS;
    }*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_outputTINinMemDTMbeforeTri = false;
static __int64 s_outputTINinMemDTMindexBeforeTri = 0;


/*int ConvertMrDTMtoFullResDTM(RefCountedPtr<BcDTM>&        singleResolutionDtm,
                                                  const std::vector<DPoint3d>&  regionPoints,
                                                  ElementAgendaCR               agenda,
                                                  ElementHandleCR                  selectedElement,
                                                  bool                          applyClip,
                                                  DTMUnit                       DTMOutputUnit)
    {
    return ConvertMrDTMtoFullResDTM(singleResolutionDtm,
                                    regionPoints,
                                    agenda,
                                    selectedElement,
                                    applyClip,
                                    DTMOutputUnit,
                                    true);
    }*/

/*int ConvertMrDTMtoFullResDTM(RefCountedPtr<BcDTM>&        singleResolutionDtm,
                                                  const std::vector<DPoint3d>&  regionPoints,
                                                  ElementAgendaCR               agenda,
                                                  ElementHandleCR                  selectedElement,
                                                  bool                          applyClip,
                                                  DTMUnit                       DTMOutputUnit,
                                                  bool                          useFullResolution)
    {
    DTMPtr        dtmPtr = 0;

    const UInt64 MAX_POINT_COUNT = getExtractedTMMaxPointCount();
    assert(MAX_POINT_COUNT <= (numeric_limits<size_t>::max)());

    size_t  remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);

    double decimationFactorRequested;

    vector<__int64> approximateNbPointsForLinearFeaturesList;

    if (useFullResolution == false)
        {
        __int64 approximateTotalNbPoints = 0;

        for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
            {
            unsigned int approximateNbPointsForPointFeatures;
            unsigned int approximateNbPointsForLinearFeatures;

            //MST TBD - Check what occurs with this param : applyClip
            StatusInt status = GetApproximationNbPtsNeedToExtract(*elemIter, regionPoints, &approximateNbPointsForPointFeatures, &approximateNbPointsForLinearFeatures);

            if (status != SUCCESS)
                {
                //MST TBD - What should we do?
                approximateNbPointsForLinearFeaturesList.push_back(0);
                }
            else
                {
                approximateTotalNbPoints += approximateNbPointsForPointFeatures + approximateNbPointsForLinearFeatures;
                approximateNbPointsForLinearFeaturesList.push_back(approximateNbPointsForLinearFeatures);
                }
            }


        if ((__int64)remainingPointCount < approximateTotalNbPoints)
            {
            decimationFactorRequested = (double)remainingPointCount / approximateTotalNbPoints;
            }
        else
            {
            decimationFactorRequested = 1.0;
            }
        }
    else
        {
        decimationFactorRequested = DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION;
        }

    StatusInt updateStatus;


    int nbPointsForLinearFeatureInd = 0;
    int maxNbTries;

    if (useFullResolution == false)
        {
        maxNbTries = 20;
        }
    else
        {
        maxNbTries = 1;
        }

    int tryInd = 0;

    double       decimationFactorForPointFeatures = decimationFactorRequested;
    unsigned int maximumNbLinearFeaturePoints;

    for (; tryInd < maxNbTries; tryInd++)
        {
        updateStatus = IScalableMeshQuery::S_SUCCESS;
        nbPointsForLinearFeatureInd = 0;

    for (ElementAgenda::const_iterator elemIter = agenda.begin(); elemIter != agenda.end(); ++elemIter)
        {
        if (decimationFactorRequested == DECIMATION_FACTOR_REQUESTED_FULL_RESOLUTION)
            {
            maximumNbLinearFeaturePoints = NB_LINEAR_FEATURE_POINTS_REQUESTED_FULL_RESOLUTION;
            }
        else
            {
            maximumNbLinearFeaturePoints = (unsigned int)(approximateNbPointsForLinearFeaturesList[nbPointsForLinearFeatureInd] * decimationFactorRequested);
            }

        if (IsProcessTerminatedByUser())
            return Extract_ABORT;

        ElementHandleCP eh = &*elemIter;

        Transform fromModelRefToActiveTransform;
        Transform invertFromModelRefToActiveTransform;

        GetFromModelRefToActiveTransform(fromModelRefToActiveTransform, eh->GetModelRef());

        bool invertSuccess = bsiTransform_invertTransform(&invertFromModelRefToActiveTransform, &fromModelRefToActiveTransform);
        assert(invertSuccess != 0);

        Transform trsf;
        Transform invertTrsf;
        DTMElementHandlerManager::GetStorageToUORMatrix(trsf, *eh);

        invertSuccess = bsiTransform_invertTransform(&invertTrsf, &trsf);
        assert(invertSuccess != 0);

        Transform activeToStorageTrsf;
        bsiTransform_multiplyTransformTransform(&activeToStorageTrsf, &invertTrsf, &invertFromModelRefToActiveTransform);

        std::vector<DPoint3d> regionPointsInStorageCS(regionPoints);
        bsiTransform_multiplyDPoint3dArrayInPlace(&activeToStorageTrsf, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size());

        s_queryProcessTerminatedByUser = false;

        updateStatus = updateDTMPtrWithSTM(dtmPtr,
                                           &regionPointsInStorageCS[0], regionPointsInStorageCS.size(),
                                           *eh,
                                           remainingPointCount,
                                           applyClip,
                                           DTMOutputUnit,
                                           decimationFactorForPointFeatures,
                                           maximumNbLinearFeaturePoints);

        if (useFullResolution == false)
            {
            nbPointsForLinearFeatureInd++;
            }

        if ((BSISUCCESS != updateStatus) || (s_queryProcessTerminatedByUser == true))
            {
            if (IScalableMeshQuery::S_NBPTSEXCEEDMAX == updateStatus)
                {
                break;
                }
            else if (s_queryProcessTerminatedByUser)
                {
                break;
                }
            else
                {
                break;
                }
            }
        }

        if (BSISUCCESS != updateStatus)
            {
            if (IScalableMeshQuery::S_NBPTSEXCEEDMAX == updateStatus)
                {
                if (useFullResolution == false)
                    {
                    //MST TBD - When there is only with STM containing only points dividing by 4 is more optimal.
                    decimationFactorRequested /= 2;
                    remainingPointCount = static_cast<size_t>(MAX_POINT_COUNT);
                    }
                else
                    {
                    return Extract_NBPTSEXCEEDMAX;
                    }
                }
            else if (s_queryProcessTerminatedByUser)
                {
                return Extract_ABORT;
                }
            else
                {
                return Extract_ERROR;
                }
            }
        else
            {
            break;
            }
        }

    if (mdlSystem_extendedAbortRequested() || ESC_KEY == mdlSystem_getChar())
        {
        return Extract_ABORT;
        }

    if (tryInd == maxNbTries)
        {
        return Extract_ERROR;
        }

    if (NULL == &*dtmPtr)
        return Extract_SUCCESS; // NTERAY: Should it really be a success??

    if (dtmPtr->GetPointCount() <= 0)
        return Extract_NOPOINTS;

    IScalableMeshClipContainerPtr clips(IScalableMeshClipContainer::Create());

    if (BSISUCCESS != addClipsToClipContainer(clips, agenda, regionPoints, applyClip, DTMOutputUnit))
        return Extract_ERROR;

    s_queryProcessTerminatedByUser = false;

    if (BSISUCCESS != triangulateDTMAndApplyClips(dtmPtr, selectedElement, clips))
        {
        if (s_queryProcessTerminatedByUser == true)
            {
            return Extract_ABORT;
            }
        else
            {
            return Extract_ERROR;
            }
        }

    singleResolutionDtm = dtmPtr->GetBcDTM();
    return Extract_SUCCESS;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Chantal.Poulin    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*Transform transformFromStorageToActiveUOR(ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform fromModelRefToActive;
    GetFromModelRefToActiveTransform(fromModelRefToActive, tmElementHandle.GetModelRef());

 /*   DPoint3d translation;
    StatusInt status = mdlModelRef_getGlobalOrigin(ACTIVEMODEL, &translation);
    assert(status == SUCCESS);

    Transform fromActiveModelToActiveModelGlobalOrigin;
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, -translation.x, 0.0, 1.0, 0.0, -translation.y, 0.0, 0.0, 1.0, -translation.z);
    bsiTransform_multiplyTransformTransform(&fromModelRefToActive, &fromActiveModelToActiveModelGlobalOrigin, &fromModelRefToActive);
    */

/*    Transform fromStorageToUOR;
    DTMElementHandlerManager::GetStorageToUORMatrix(fromStorageToUOR, tmElementHandle);

    Transform fromStorageToActiveUOR;

    bsiTransform_multiplyTransformTransform(&fromStorageToActiveUOR, &fromModelRefToActive, &fromStorageToUOR);

    return fromStorageToActiveUOR;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*Transform transformFromStorageToActiveMeter(ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform fromModelRefToActive;
    GetFromModelRefToActiveTransform(fromModelRefToActive, tmElementHandle.GetModelRef());

    Transform trsf;
    DTMElementHandlerManager::GetStorageToUORMatrix(trsf, tmElementHandle);

    DPoint3d translation;
    StatusInt status = mdlModelRef_getGlobalOrigin(ACTIVEMODEL, &translation);
    assert(status == SUCCESS);

    Transform fromActiveModelToActiveModelGlobalOrigin;
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, -translation.x, 0.0, 1.0, 0.0, -translation.y, 0.0, 0.0, 1.0, -translation.z);
    bsiTransform_multiplyTransformTransform(&fromModelRefToActive, &fromActiveModelToActiveModelGlobalOrigin, &fromModelRefToActive);

    Transform fromStorageToActiveInMeters;

    bsiTransform_multiplyTransformTransform(&fromStorageToActiveInMeters, &fromModelRefToActive, &trsf);

    double meterPerUorActiveModel = 1 / mdlModelRef_getUorPerMeter(ACTIVEMODEL);

    Transform fromUORtoMeterActive;

    bsiTransform_initFromRowValues(&fromUORtoMeterActive, meterPerUorActiveModel, 0.0, 0.0, 0.0, 0.0, meterPerUorActiveModel, 0.0, 0.0, 0.0, 0.0, meterPerUorActiveModel, 0.0);

    bsiTransform_multiplyTransformTransform(&fromStorageToActiveInMeters, &fromUORtoMeterActive, &fromStorageToActiveInMeters);

    return fromStorageToActiveInMeters;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Mathieu.St-Pierre    09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*Transform transformFromModelRefToActiveMeter(DgnModelRefP modelRef)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform fromModelRefToActive;
    GetFromModelRefToActiveTransform(fromModelRefToActive, modelRef);

    DPoint3d translation;
    StatusInt status = mdlModelRef_getGlobalOrigin(ACTIVEMODEL, &translation);
    assert(status == SUCCESS);

    Transform fromActiveModelToActiveModelGlobalOrigin;
    bsiTransform_initFromRowValues(&fromActiveModelToActiveModelGlobalOrigin, 1.0, 0.0, 0.0, -translation.x, 0.0, 1.0, 0.0, -translation.y, 0.0, 0.0, 1.0, -translation.z);
    bsiTransform_multiplyTransformTransform(&fromModelRefToActive, &fromActiveModelToActiveModelGlobalOrigin, &fromModelRefToActive);

    double meterPerUorActiveModel = 1 / mdlModelRef_getUorPerMeter(ACTIVEMODEL);

    Transform fromModelRefToActiveInMeters;

    bsiTransform_initFromRowValues(&fromModelRefToActiveInMeters, meterPerUorActiveModel, 0.0, 0.0, 0.0, 0.0, meterPerUorActiveModel, 0.0, 0.0, 0.0, 0.0, meterPerUorActiveModel, 0.0);

    bsiTransform_multiplyTransformTransform(&fromModelRefToActiveInMeters, &fromModelRefToActiveInMeters, &fromModelRefToActive);

    return fromModelRefToActiveInMeters;
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt transformDTMFromModelGlobalToElementStorage (BcDTM& ibcdtm, ElementHandleCR tmElementHandle)
    {
    USING_NAMESPACE_GEODTMAPP

    Transform elementStorageToUOR;
    DTMElementHandlerManager::GetStorageToUORMatrix(elementStorageToUOR, tmElementHandle);
    const TransfoModel uorToElementStorage(TransfoModel::CreateFrom(InverseOf(FromBSITransform(elementStorageToUOR))));

    TransfoModel modelGlobalToElementStorage(Combine(GetModelGlobalToDesignTransfoModel(tmElementHandle.GetModelRef()), uorToElementStorage));

    ibcdtm.SetMemoryAccess (DTMAccessMode::Write);
    return ibcdtm.TransformUsingCallback (&transformDTMCallback, const_cast<TransfoModel*>(&modelGlobalToElementStorage));    
    }*/


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.St-Pierre  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt GetApproximationNbPtsNeedToExtract(IScalableMeshPtr                    mrDTMPtr,
                                                                  DgnGCSP                     destinationGCS,
                                                                  DRange3d                     drange,
                                                                  const std::vector<DPoint3d>& regionPointsInStorageCS,
                                                                  vector<ClipInfo>             clips,
                                                                  unsigned int*                nbPoints)
    {
    assert(nbPoints != 0);

    unsigned int nbPointsForPointFeatures;
    unsigned int nbPointsForLinearFeatures;

    StatusInt status = GetApproximationNbPtsNeedToExtract(mrDTMPtr,
                                                          destinationGCS,
                                                          drange,
                                                          regionPointsInStorageCS,
                                                          clips,
                                                          &nbPointsForPointFeatures,
                                                          &nbPointsForLinearFeatures);

    *nbPoints = nbPointsForPointFeatures + nbPointsForLinearFeatures;

    return status;
    }

StatusInt GetApproximationNbPtsNeedToExtract(IScalableMeshPtr                    mrDTMPtr,
                                                                  DgnGCSP                     destinationGCS,
                                                                  DRange3d                     drange,
                                                                  const std::vector<DPoint3d>& regionPointsInStorageCS,
                                                                  vector<ClipInfo>             clips,
                                                                  unsigned int*                nbPointsForPointFeatures,
                                                                  unsigned int*                nbPointsForLinearFeatures)
    {
    DTMPtr         dtmPtr = 0;
    IScalableMeshQueryPtr fullResLinearQueryPtr;

    //Query the linears
    if ((mrDTMPtr->GetBaseGCS() == 0)|| (destinationGCS == 0))
        {
        //Get the query interfaces
        fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_LINEAR);
        }
    else
        {
        fullResLinearQueryPtr = mrDTMPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_LINEAR,
                                                            (Bentley::GeoCoordinates::BaseGCSPtr&)destinationGCS, drange);
        }

    if (fullResLinearQueryPtr != 0)
        {
        IScalableMeshFullResolutionLinearQueryParamsPtr mrDtmFullResLinearParametersPtr(IScalableMeshFullResolutionLinearQueryParams::CreateParams());
        mrDtmFullResLinearParametersPtr->SetTriangulationState(false);
        mrDtmFullResLinearParametersPtr->SetUseDecimation(false);
        mrDtmFullResLinearParametersPtr->SetCutLinears(false);

        if (AddClipOnQuery(fullResLinearQueryPtr, clips) != SUCCESS)
            return ERROR;

        if (fullResLinearQueryPtr->Query(dtmPtr, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size(), IScalableMeshQueryParametersPtr(mrDtmFullResLinearParametersPtr)) != IScalableMeshQuery::S_SUCCESS)
            return ERROR;

        *nbPointsForLinearFeatures = (unsigned int)dtmPtr->GetPointCount();
        }

    dtmPtr = 0;

    IScalableMeshQueryPtr fixResPointQueryPtr = mrDTMPtr->GetQueryInterface(DTM_QUERY_FIX_RESOLUTION_VIEW, DTM_QUERY_DATA_POINT);

    assert(fixResPointQueryPtr != 0 || mrDTMPtr->GetPointCount() == 0);

    if (fixResPointQueryPtr != 0)
        {
        // Find the last res that have the less than 200000 points
        IScalableMeshFixResolutionIndexQueryParamsPtr mrDtmFixResqueryParamsPtr(IScalableMeshFixResolutionIndexQueryParams::CreateParams());

        int  res = 0;
        bool found = false;
        DTMPtr singleResolutionViewDtmPtr = 0;

        for (res=0; res<mrDTMPtr->GetNbResolutions(DTM_QUERY_DATA_POINT); res++)
            {
            mrDtmFixResqueryParamsPtr->SetResolutionIndex(res);
            mrDtmFixResqueryParamsPtr->SetTriangulationState(false);

            singleResolutionViewDtmPtr = 0;
            if (fixResPointQueryPtr->Query(singleResolutionViewDtmPtr, 0, 0, IScalableMeshQueryParametersPtr(mrDtmFixResqueryParamsPtr)) != IScalableMeshQuery::S_SUCCESS)
                return ERROR;

            if (singleResolutionViewDtmPtr->GetPointCount() < 200000)
                {
                found = true;
                break;
                }
            }

        if (!found)
            return ERROR;

        IScalableMeshPtr      singleResMrDTMViewPtr = IScalableMeshPtr((IScalableMesh*)singleResolutionViewDtmPtr.get());
        IScalableMeshQueryPtr fullResQueryPtr;

        if ((mrDTMPtr->GetBaseGCS() == 0) || (destinationGCS == 0))
            {
            fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_POINT);
            }
        else
            {
            fullResQueryPtr = singleResMrDTMViewPtr->GetQueryInterface(DTM_QUERY_FULL_RESOLUTION, DTM_QUERY_DATA_POINT,
                                                                       (Bentley::GeoCoordinates::BaseGCSPtr&)destinationGCS, drange);
            }

        assert(fullResQueryPtr != 0);

        IScalableMeshFullResolutionQueryParamsPtr mrDtmFullResQueryParam(IScalableMeshFullResolutionQueryParams::CreateParams());
        mrDtmFullResQueryParam->SetTriangulationState(false);
        mrDtmFullResQueryParam->SetReturnAllPtsForLowestLevel(false);

        if (AddClipOnQuery(fullResQueryPtr, clips) != SUCCESS)
            return ERROR;

        if (fullResQueryPtr->Query(dtmPtr, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size(), IScalableMeshQueryParametersPtr(mrDtmFullResQueryParam)) != IScalableMeshQuery::S_SUCCESS)
            return ERROR;

        *nbPointsForPointFeatures = (unsigned int)dtmPtr->GetPointCount() * (int)pow(4.0, mrDTMPtr->GetNbResolutions(DTM_QUERY_DATA_POINT) - res - 1);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Chantal.Poulin  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt GetApproximationNbPtsNeedToExtract(ElementHandleCR                  eeh,
                                                                  const std::vector<DPoint3d>&  regionPoints,
                                                                  unsigned int*                 nbPoints)
    {
    assert(nbPoints != 0);

    unsigned int nbPointsForPointFeatures;
    unsigned int nbPointsForLinearFeatures;

    StatusInt status = GetApproximationNbPtsNeedToExtract(eeh, regionPoints, &nbPointsForPointFeatures, &nbPointsForLinearFeatures);

    *nbPoints = nbPointsForPointFeatures + nbPointsForLinearFeatures;

    return status;
    }

StatusInt GetApproximationNbPtsNeedToExtract(ElementHandleCR                  eeh,
                                                                  const std::vector<DPoint3d>&  regionPoints,
                                                                  unsigned int*                 nbPointsForPointFeatures,
                                                                  unsigned int*                 nbPointsForLinearFeatures)
    {
    RefCountedPtr<DTMDataRef> DTMDataRef;
    if (DTMElementHandlerManager::GetDTMDataRef (DTMDataRef, eeh) != SUCCESS)
        return ERROR;

    RefCountedPtr<IMrDTMDataRef> mrRef = (IMrDTMDataRef*)DTMDataRef.get();

    if (mrRef->GetMrDTMState() == SCM_STATE_EMPTY)
        return ERROR;

    IScalableMeshPtr mrDTMPtr(dynamic_cast<IScalableMesh*>(DTMDataRef->GetDTMStorage(GetMrDtm)));

    if (mrDTMPtr == 0)
        return ERROR;

    *nbPointsForPointFeatures = 0;
    *nbPointsForLinearFeatures = 0;

    Transform fromModelRefToActiveTransform;
    GetFromModelRefToActiveTransform(fromModelRefToActiveTransform, eeh.GetModelRef());

    Transform fromActiveToModelRefTransform;
    bsiTransform_invertTransform(&fromActiveToModelRefTransform, &fromModelRefToActiveTransform);

    Transform trsf;
    Transform invertTrsf;
    DTMElementHandlerManager::GetStorageToUORMatrix(trsf, eeh);

    bool invert = bsiTransform_invertTransform(&invertTrsf, &trsf);
    assert(invert != 0);

    bsiTransform_multiplyTransformTransform(&invertTrsf, &invertTrsf, &fromActiveToModelRefTransform);

    vector<DPoint3d> regionPointsInStorageCS (regionPoints);
    bsiTransform_multiplyDPoint3dArrayInPlace(&invertTrsf, &regionPointsInStorageCS[0], (int)regionPointsInStorageCS.size());

    DRange3d drange;
    mrRef->GetExtents(drange);

    vector<ClipInfo> clips;

    int status = GetClipForQuery(clips, eeh);

    assert(status == SUCCESS);

    return GetApproximationNbPtsNeedToExtract(mrDTMPtr,
                                              mrRef->GetDestinationGCS(),
                                              drange,
                                              regionPointsInStorageCS,
                                              clips,
                                              nbPointsForPointFeatures,
                                              nbPointsForLinearFeatures);
    }*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                   06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/*void GetTransformForPoints(Transform& uorToMeter, Transform& meterToUor)
    {
    const double invUorPerMeter = 1.0/ ModelInfo::GetUorPerMeter(ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP());

    bsiTransform_initFromRowValues(&uorToMeter, invUorPerMeter, 0.0, 0.0, 0.0, 0.0, invUorPerMeter, 0.0, 0.0, 0.0, 0.0, invUorPerMeter, 0.0);

    bsiTransform_invertTransform(&meterToUor, &uorToMeter);
    }*/

void AugmentRangeByFactor(DRange2d& range, const double& factor)
    {
    const double distance = sqrt(pow(range.low.x - range.high.x,2)+pow(range.low.y-range.high.y,2));
    //const double distanceStep = 1.0;
    //int step = 1;

    //MST TBD - While if distance * scale greater than x or y dimension?
    const double scale = factor/100.0;
    range.low.x  = range.low.x  - distance*scale;
    range.low.y  = range.low.y  - distance*scale;
    range.high.x = range.high.x + distance*scale;
    range.high.y = range.high.y + distance*scale;
    }
