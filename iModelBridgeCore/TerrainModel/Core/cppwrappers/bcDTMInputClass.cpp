/*--------------------------------------------------------------------------------------+
|
//    $Source: Core/cppwrappers/bcDTMInputClass.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma warning(disable: 4018) 

//----------------------------------------------------------------------+
//Include standard library header files                                 |
//-----------------------------------------------------------------------
#include <math.h>

#include <TerrainModel/TerrainModel.h>
#include <bcDTMBaseDef.h>

//----------------------------------------------------------------------+
//Include BCivil general header files                                   |
//-----------------------------------------------------------------------
#include <bcMacros.h>
#include <bcGmcNorm.h>   

/*------------------------------------------------------------------+
//Include of the current class header                               |
+------------------------------------------------------------------*/
#include "bcMem.h"
#include "bcDTMImpl.h"

#include <TerrainModel\Core\TMTransformHelper.h>

//-------------------------------------------------------------------
//Include COGO definitions                                          |
//-------------------------------------------------------------------

USING_NAMESPACE_BENTLEY_TERRAINMODEL

//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20Feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPoint (DPoint3dCR point)
    {
    return AddPoints (&point, 1);
    }

//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20Feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPoints (DPoint3dCP ptsP, int numPts)
    {
    // Check arguments
    if ( ptsP == NULL) return DTM_ERROR ;

    // Store As Random Spots

    DTMFeatureId dtmFeatureId = DTM_NULL_FEATURE_ID ; 
    if (SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    if (_dtmTransformHelper.IsValid())
        return (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject (_dtmHandleP, DTMFeatureType::RandomSpots,DTM_NULL_USER_TAG,1,&dtmFeatureId, _dtmTransformHelper->copyPointsToDTM (ptsP, numPts), numPts);

    return (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject (_dtmHandleP, DTMFeatureType::RandomSpots, DTM_NULL_USER_TAG, 1, &dtmFeatureId, ptsP, numPts);
    }
//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20Feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCR point, DTMFeatureId* featureIdP)
    {
    return AddPointFeature (&point, 1, DTM_NULL_USER_TAG, featureIdP);
    }

//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20dec2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCR point, DTMUserTag userTag, DTMFeatureId* featureIdP)
    {
    return AddPointFeature (&point, 1, userTag, featureIdP);
    }
//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20Feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCP ptsP, int numPts, DTMFeatureId* featureIdP)
    {
    return AddPointFeature (ptsP, numPts, DTM_NULL_USER_TAG, featureIdP);
    }

//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20Feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCP ptsP, int numPts, DTMUserTag userTag, DTMFeatureId* featureIdP)
    {
    // This calls the same underlining code as AddLinearFeature with Group Spot as its feature type.
    return AddLinearFeature (DTMFeatureType::GroupSpots, ptsP, numPts, userTag, featureIdP);
    }

//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddLinearFeature(DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMFeatureId* featureIdP)
    {
    return AddLinearFeature (dtmFeatureType, ptsP, numPts, DTM_NULL_USER_TAG, featureIdP);
    }
//----------------------------------------------------------------------+
//                                                                      |
//  rsc.20feb2008   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::AddLinearFeature (DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMUserTag userTag, DTMFeatureId*  featureIdP)
    {

    // Check arguments
    if ( ptsP == NULL) return DTM_ERROR ;

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    // Store the feature in the DTM
    if (_dtmTransformHelper.IsValid())
        return (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject (_dtmHandleP, dtmFeatureType, userTag, 3, featureIdP, _dtmTransformHelper->copyPointsToDTM (ptsP, numPts), numPts);
    return (DTMStatusInt)bcdtmObject_storeDtmFeatureInDtmObject (_dtmHandleP, dtmFeatureType, userTag, 3, featureIdP, (DPoint3d*)ptsP, numPts);
    }

//----------------------------------------------------------------------+
//                                                                      |
//  cbe.17jan2005   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::DeleteFeaturesByUserTag (DTMUserTag  userTag)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_deleteAllOccurrencesOfDtmFeaturesWithUserTagDtmObject (_dtmHandleP, userTag);
    return status;
    }

//----------------------------------------------------------------------+
//                                                                      |
//  cbe.17jan2005   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::DeleteFeatureById (DTMFeatureId featureID)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_deleteAllOccurrencesOfDtmFeaturesWithFeatureIdDtmObject (_dtmHandleP, featureID);
    return status;
    }

//----------------------------------------------------------------------+
//                                                                      |
//  cbe.17jan2005   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::DeleteFeaturesByType (DTMFeatureType featureType)
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject (_dtmHandleP, featureType);
    return status ;
    }

//----------------------------------------------------------------------+
//                                                                      |
//  cbe.17jan2005   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::RemoveHull()
    {
    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    DTMStatusInt status = (DTMStatusInt)bcdtmData_deleteAllOccurrencesOfDtmFeatureTypeDtmObject (_dtmHandleP, DTMFeatureType::Hull);
    return status;
    }

//----------------------------------------------------------------------+
//                                                                      |
//  cbe.17jan2005   -  Created.                                         |
//                                                                      |
//-----------------------------------------------------------------------
DTMStatusInt BcDTM::JoinFeatures (DTMFeatureType dtmFeat, int* nFeatures, int* nJoinedFeatures, double tol)
    {
    DTMStatusInt status;	
    long	numFeat = 0, numJoined = 0 , numJoinUserTags = 0 ;
    DTMFeatureType dtmFeatureType = DTMFeatureType::None, joinedDtmFeatureType = DTMFeatureType::None;
    DTM_JOIN_USER_TAGS *joinUserTagsP=NULL ;

    if (dtmFeat == DTMFeatureType::Hole)
        {
        dtmFeatureType = DTMFeatureType::HoleLine ;
        joinedDtmFeatureType = DTMFeatureType::Hole ;
        }
    else if (dtmFeat == DTMFeatureType::Void)
        {
        dtmFeatureType = DTMFeatureType::VoidLine ;
        joinedDtmFeatureType = DTMFeatureType::Void ;
        }
    else
        {
        return DTM_ERROR;
        }

    if (SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }
    status = (DTMStatusInt)bcdtmJoin_dtmFeatureTypeWithRollbackDtmObject (
        _dtmHandleP,
        tol, 
        dtmFeatureType,
        joinedDtmFeatureType,
        &numFeat,
        &numJoined,
        &joinUserTagsP,
        &numJoinUserTags, true);

    if (nFeatures) *nFeatures = numFeat;
    if (nJoinedFeatures) *nJoinedFeatures = numJoined;
    if( joinUserTagsP != NULL ) free(&joinUserTagsP) ;

    return status;
    }

#ifdef NOTDEF
/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPoint(DPoint3dCR point)
    {
    return _AddPoint (point);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPoints (DPoint3dCP pointsP, int nPoint)
    {
    return _AddPoints (pointsP,nPoint);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCR point, DTMFeatureId *featureIdP)
    {
    return _AddPointFeature (point,featureIdP);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCR point, DTMUserTag userTag, DTMFeatureId *idP)
    {
    return _AddPointFeature (point,userTag,idP);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCP ptsP, int numPts,DTMFeatureId *featureIdP)
    {
    return _AddPointFeature (ptsP,numPts,featureIdP);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddPointFeature (DPoint3dCP ptsP, int numPts,DTMUserTag userTag,DTMFeatureId *idP)
    {
    return _AddPointFeature (ptsP,numPts,userTag,idP);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddLinearFeature (DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMFeatureId *featureIdP )
    {
    return _AddLinearFeature (dtmFeatureType, ptsP, numPts, featureIdP);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::AddLinearFeature (DTMFeatureType dtmFeatureType, DPoint3dCP ptsP, int numPts, DTMUserTag userTag,DTMFeatureId *featureIdP )
    {
    return _AddLinearFeature (dtmFeatureType, ptsP, numPts, userTag,featureIdP );
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::DeleteFeatureById (DTMFeatureId guID)
    {
    return _DeleteFeatureById (guID);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::DeleteFeaturesByUserTag (DTMUserTag userTag)
    {
    return _DeleteFeaturesByUserTag (userTag);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::DeleteFeaturesByType (DTMFeatureType dtmfeat)
    {
    return _DeleteFeaturesByType(dtmfeat);
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::JoinFeatures(DTMFeatureType dtmFeatureType,int *nFeatures,int *nJoinedFeatures,double tol)
    {
    return _JoinFeatures(dtmFeatureType,nFeatures,nJoinedFeatures,tol);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTM::RemoveHull()
    {
    return _RemoveHull();
    }
#endif