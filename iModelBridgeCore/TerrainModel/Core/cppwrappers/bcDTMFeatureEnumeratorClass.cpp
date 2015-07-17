/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMFeatureEnumeratorClass.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma warning(disable: 4018)

/*----------------------------------------------------------------------+
| Include standard library header files                                 |
+----------------------------------------------------------------------*/
#include <math.h>

#include <TerrainModel/TerrainModel.h>
#include <bcDTMBaseDef.h>
#include <dtmevars.h>
#include <algorithm>

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <bcMacros.h>
#include <bcGmcNorm.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "bcMem.h"
#include "bcDTMImpl.h"
#include <TerrainModel\Core\TMTransformHelper.h>

/*------------------------------------------------------------------+
| Local defintions                                                  |
+------------------------------------------------------------------*/

/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/
USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMFeatureEnumerator::_Initialize()
    {
    // Call generic BCIVIL object initialization


    // Reset all the attributes
    m_featuresTypes.clear ();
    m_isMinRange = FALSE;
    m_isMaxRange = FALSE;
    m_state = 0; // Creation state

    m_dtmHandleP = NULL;

    // Initialize feature scanning indicators
    m_DtmScanContext = NULL;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureEnumerator::BcDTMFeatureEnumerator (BcDTMP dtmP)
    {
    // Intialize data
    _Initialize();

    // Store the DTM handle and add a reference
    m_dtmP = dtmP;

    // Store the tinHandle (optimization purpose)
    m_dtmHandleP = m_dtmP->GetTinHandle();

    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::Reset ()
    {
    m_state = 1; // Reset state

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*
int BcDTMFeatureEnumerator::_MoveNextFeatureDat
(
)
{
if (m_IsAtEnd)
{
return DTM_ERROR;
}
else
{
long              wasFound = FALSE;
DTM_POINT_ARRAY   **pointsArray = NULL;
long              numPointArray;
DTMUserTag userTag;
long         featureType;
DTMFeatureId     guid;

bool     endLoop = FALSE; // BoolInt
do
{
// Initialize values
pointsArray = NULL;
numPointArray = 0;


// Try to get the next feature for the current feature type
int status = bcdtmLoadNgp_scanForDtmFeatureTypeOccurrenceDataObject ( m_DtmScanContext,
&wasFound, &featureType, &userTag, &guid, &pointsArray, &numPointArray);
if (status != DTM_SUCCESS)
return DTM_ERROR;

if (wasFound)
{
// We can exit the loop, everything is fine
endLoop = TRUE;
}
else
{
// If the feature was not found, we try with the next feature type
m_CurrentFeatureTypeIterator++;
if (m_CurrentFeatureTypeIterator == m_featuresTypes.end ())
{
// If there is no other feature type, it means that the end was reached,
// so we set the "m_IsAtEnd" flag which means that we are at the end
m_IsAtEnd = TRUE;
endLoop = TRUE;
break;
}
else
{
// We reset the position indicators, in order to rescan with this feature type
_resetScanContextForDat ((long)(*m_CurrentFeatureTypeIterator) , FALSE);
}
}

} while (!endLoop);

if (wasFound)
{
// Memorize feature attributes
m_CurrentFeature.featureUserTag = userTag;
Copy (m_CurrentFeature.featureId, guid);
m_CurrentFeature.type = *m_CurrentFeatureTypeIterator;

if (bcDTMGuid_isClear(&m_CurrentFeature.featureId))
{
// Initialize point string
m_CurrentFeature.pointArrays.clear ();
m_CurrentFeature.pointArrays.reserve (numPointArray);
for (int i = 0; i < numPointArray; i++)
{
PointString ptString;
ptString.points.clear();
ptString.points.reserve((*pointsArray)->numPoints);
for (int j = 0; j < (*pointsArray)->numPoints; j++)
{
DPoint3d    pt;
Copy (pt, (*pointsArray)->pointsP[j]);
ptString.points.push_back(pt);
}
m_CurrentFeature.pointArrays.push_back(ptString);
}
}

// Free data
if (numPointArray != 0)
{
// Call MS free (because we are in DTM DLL)
bcdtmMem_freePointerArrayToPointArrayMemory (&pointsArray, numPointArray) ;
}
}
}

return DTM_SUCCESS;
}
*/

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*
bool BcDTMFeatureEnumerator::_MoveNextDat //BoolInt
(
)
{
bool retVal = FALSE; // BoolInt

// Move the cursor to the next value, depending on the current state
switch (m_state)
{
case 0:
{
// If the list is not yet reset, return FALSE
retVal = FALSE;
break;
}
case 1:
{
m_CurrentFeatureTypeIterator = m_featuresTypes.begin ();

_resetScanContextForDat ((long)(*m_CurrentFeatureTypeIterator) , TRUE);
m_IsAtEnd = FALSE;

// Move one position forward
_moveNextFeatureDat ();
if (m_IsAtEnd)
{
m_state = 3;
retVal = FALSE;
}
else
{
m_state = 2;
retVal = TRUE;
}
break;
}
case 3:
{
// If we are already at the end, return FALSE
retVal = FALSE;
break;
}
default:
{
// Move one position forward
_moveNextFeatureDat ();
if (m_IsAtEnd)
{
m_state = 3;
retVal = FALSE;
}
else
{
retVal = TRUE;
}
break;
}
}

return retVal;
}
*/

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::_MoveNextFeatureTin ()
    {
    if (m_IsAtEnd)
        {
        return DTM_ERROR;
        }
    else
        {
        long        wasFound = FALSE;
        DTM_POINT_ARRAY   **pointsArray = NULL;
        long              numPointArray = 0 ;
        DTMUserTag userTag;
        DTMFeatureId     id;
        DTMFeatureType   featureType;

        bool    endLoop = FALSE;
        do
            {
            // Initialize values
            pointsArray = NULL;
            numPointArray = 0;

            // Try to get the next feature for the current feature type
            int status = bcdtmScanContextLoad_scanForDtmFeatureTypeOccurrenceDtmObject (m_DtmScanContext,
                &wasFound, &featureType, &userTag, &id, &pointsArray, &numPointArray);
            if (status != DTM_SUCCESS) return DTM_ERROR;

            if (wasFound)
                {
                // We can exit the loop, everything is fine
                endLoop = TRUE;
                }
            else
                {
                // If the feature was not found, we try with the next feature type
                m_CurrentFeatureTypeIterator++;
                if (m_CurrentFeatureTypeIterator == m_featuresTypes.end ())
                    {
                    // If there is no other feature type, it means that the end was reached,
                    // so we set the "m_IsAtEnd" flag which means that we are at the end
                    m_IsAtEnd = TRUE;
                    endLoop = TRUE;
                    break;
                    }
                else
                    {
                    // We reset the position indicators, in order to rescan with this feature type
                    _ResetScanContextForTin (*m_CurrentFeatureTypeIterator , FALSE);
                    }
                }

            } while (!endLoop);

            if (wasFound)
                {
                // Memorize feature attributes
                m_CurrentFeature.featureUserTag= userTag;
                m_CurrentFeature.featureId = id ;
                m_CurrentFeature.type = *m_CurrentFeatureTypeIterator;

                // Initialize point string
                m_CurrentFeature.pointArrays.clear ();
                m_CurrentFeature.pointArrays.reserve (numPointArray);
                for (int i = 0; i < numPointArray; i++)
                    {
                    DtmString ptString;

                    if (m_dtmP->GetTransformHelper ())
                        m_dtmP->GetTransformHelper ()->convertPointsFromDTM ((*pointsArray)->pointsP, (*pointsArray)->numPoints);

                    ptString.reserve((*pointsArray)->numPoints);
                    for (int j = 0; j < (*pointsArray)->numPoints; j++)
                        {
                        ptString.push_back ((*pointsArray)->pointsP[j]);
                        }
                    m_CurrentFeature.pointArrays.push_back (ptString);
                    }

                // Free data
                if (numPointArray != 0)
                    {
                    // Call MS free (because we are in DTM DLL)
                    bcdtmMem_freePointerArrayToPointArrayMemory (&pointsArray, numPointArray) ;
                    }
                }
        }

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
bool BcDTMFeatureEnumerator::_MoveNextTin ()
    {
    bool retVal = FALSE;

    // Move the cursor to the next value, depending on the current state
    switch (m_state)
        {
        case 0:
            {
            // If the list is not yet reset, return FALSE
            retVal = FALSE;
            break;
            }
        case 1:
            {
            m_CurrentFeatureTypeIterator = m_featuresTypes.begin ();

            _ResetScanContextForTin (*m_CurrentFeatureTypeIterator , TRUE);

            m_IsAtEnd = FALSE;

            // Move one position forward
            _MoveNextFeatureTin ();
            if (m_IsAtEnd)
                {
                m_state = 3;
                retVal = FALSE;
                }
            else
                {
                m_state = 2;
                retVal = TRUE;
                }
            break;
            }
        case 3:
            {
            // If we are already at the end, return FALSE
            retVal = FALSE;
            break;
            }
        default:
            {
            // Move one position forward
            _MoveNextFeatureTin ();
            if (m_IsAtEnd)
                {
                m_state = 3;
                retVal = FALSE;
                }
            else
                {
                retVal = TRUE;
                }
            break;
            }
        }

    return retVal;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
bool BcDTMFeatureEnumerator::MoveNext ()
    {
    //if (m_dtmP != NULL)
    //{
    return _MoveNextTin ();
    //}
    //
    //else
    //{
    //    return _MoveNextDat ();
    //}
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeaturePtr BcDTMFeatureEnumerator::_CurrentFeature ()
    {
    BcDTMFeaturePtr         dtmFeatureP = NULL;

    if (m_state != 2)
        return NULL;

    int status = DTM_ERROR;
    if (m_CurrentFeature.featureId == DTM_NULL_FEATURE_ID)
        {
        //    RobC - A Group Spot Feature Is A Linear Feature
        if (m_CurrentFeature.type == DTMFeatureType::GroupSpots || m_CurrentFeature.type == DTMFeatureType::RandomSpots)

            //      if ( m_CurrentFeature.type == -99999 )
            {
            dtmFeatureP = BcDTMSpot::Create (m_CurrentFeature.featureUserTag, m_CurrentFeature.featureId, m_CurrentFeature.type, m_CurrentFeature.pointArrays[0].data(),
                (int)m_CurrentFeature.pointArrays[0].size());
            }
        else
            {
            dtmFeatureP = BcDTMLinearFeature::Create (m_CurrentFeature.type, m_CurrentFeature.featureUserTag,
                                                      m_CurrentFeature.featureId, m_CurrentFeature.pointArrays[0].data (), (int)m_CurrentFeature.pointArrays[0].size ());
            }

        status = DTM_SUCCESS;
        }

    else
        {
        status = m_dtmP->GetFeatureById (dtmFeatureP, m_CurrentFeature.featureId);
        }

    if (status == DTM_SUCCESS)
        return dtmFeatureP;
    else
        return NULL;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeature *BcDTMFeatureEnumerator::Current()
    {
    return _CurrentFeature ().get();
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureEnumerator::~BcDTMFeatureEnumerator()
    {

    if (m_DtmScanContext != NULL)
        bcdtmScanContextLoad_deleteScanContext (&m_DtmScanContext);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::ExcludeAllFeatureTypes()
    {
    m_featuresTypes.clear ();

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::IncludeFeatureType (DTMFeatureType	featureType)
    {
    // Put the new feature at the end
    m_featuresTypes.push_back (featureType);

    // Make features unique
    std::sort (m_featuresTypes.begin (), m_featuresTypes.end ());
    std::unique (m_featuresTypes.begin (), m_featuresTypes.end ());

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::SetRange (DPoint3dP minRangeP, DPoint3dP maxRangeP)
    {
    // Check argument
    if (minRangeP != NULL)
        {
        m_isMinRange = TRUE;
        m_minRange = *minRangeP;
        }

    // Check argument
    if (maxRangeP != NULL)
        {
        m_isMaxRange = TRUE;
        m_maxRange = *maxRangeP;
        }

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::RemoveRange ()
    {
    // Check argument
    m_isMinRange = FALSE;
    m_isMaxRange = FALSE;

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::SetFeatureTypes  (DTMFeatureType* featureTypesP, int nFeature)
    {
    for (int iType = 0; iType < nFeature; iType++)
        IncludeFeatureType (featureTypesP[iType]);

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.01apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::_GetFenceFromRange (DPoint3dP& fenceP, int& nPointP)
    {
    fenceP = NULL;
    nPointP = 0;

    if (! (m_isMinRange && m_isMaxRange))
        return DTM_SUCCESS;

    nPointP = 5;

    fenceP = (DPoint3dP)bcMem_malloc (nPointP *sizeof (DPoint3d));
    fenceP[0].x = m_minRange.x;
    fenceP[0].y = m_minRange.y;
    fenceP[0].z = 0.0;
    fenceP[1]= fenceP[0];
    fenceP[1].y = m_maxRange.y;
    fenceP[2]= fenceP[1];
    fenceP[2].x = m_maxRange.x;
    fenceP[3]= fenceP[2];
    fenceP[3].y = fenceP[0].y;
    fenceP[4]  = fenceP[0];

    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.01apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
/*
int BcDTMFeatureEnumerator::_ResetScanContextForDat
(
long  newfeatureType,
bool  newScan
)
{
if (newScan && m_DtmScanContext != NULL) bcdtmScanContextLoad_deleteScanContext (&m_DtmScanContext);
if (m_DtmScanContext != NULL)
{
// reset the scan context structure
m_DtmScanContext->dtmFeatureType = newfeatureType;
m_DtmScanContext->scanOffset1 = 0L;
m_DtmScanContext->scanOffset2 = 0L;
m_DtmScanContext->scanOffset3 = 0L;
}
else
{
DPoint3d    *fencePt = NULL;
int         nFencePt = 0;

_GetFenceFromRange (&fencePt, &nFencePt);

// Set the maxSpot argument equal to 1 in case of spot features
// this is to be sure they are return back one point for each call
bcdtmLoadNgp_createDtmScanContextForDataObject (m_dtmHandleP, newfeatureType,
((newfeatureType == DTMFeatureType::RandomSpots || newfeatureType == DTMFeatureType::GroupSpots)? 1 :0), (nFencePt > 0),
1, (DPoint3d*)fencePt, nFencePt, &m_DtmScanContext);

if (fencePt != NULL)
bcMem_free (fencePt);
}
return DTM_SUCCESS;
}
*/

/*----------------------------------------------------------------------+
|                                                                       |
|   cbe.01apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMFeatureEnumerator::_ResetScanContextForTin (DTMFeatureType newfeatureType, bool newScan)
    {
    if (newScan && m_DtmScanContext != NULL) bcdtmScanContextLoad_deleteScanContext (&m_DtmScanContext);
    if (m_DtmScanContext != NULL)
        {
        // reset the scan context structure
        m_DtmScanContext->dtmFeatureType = newfeatureType;
        m_DtmScanContext->scanOffset1 = 0L;
        m_DtmScanContext->scanOffset2 = 0L;
        m_DtmScanContext->scanOffset3 = 0L;
        }
    else
        {
        DPoint3dP   fencePt = NULL;
        DTMFenceType fenceType = DTMFenceType::Block ;
        long        maxSpots = 1024 ;
        int         nFencePt = 0;

        _GetFenceFromRange (fencePt, nFencePt);

        bcdtmScanContextLoad_createScanContextForDtmObject((BC_DTM_OBJ *)m_dtmHandleP, newfeatureType,
            //RobC 04sep2008            ((newfeatureType == DTMFeatureType::RandomSpots || newfeatureType == DTMFeatureType::GroupSpots)? maxSpots : 0 ), (nFencePt > 0),
            ( newfeatureType == DTMFeatureType::RandomSpots ? maxSpots : 0 ), (nFencePt > 0),
            fenceType, DTMFenceOption::Inside, (DPoint3d*)fencePt, nFencePt, &m_DtmScanContext);
        if (fencePt != NULL)
            bcMem_free (fencePt);
        }
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureEnumeratorPtr BcDTMFeatureEnumerator::Clone (void)
    {
    return BcDTMFeatureEnumerator::Create (*this);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMFeatureEnumerator::BcDTMFeatureEnumerator (BcDTMFeatureEnumerator& rhs)
    {
    // Intialize data
    _Initialize();

    // Copy all the atributes
    m_state = rhs.m_state;
    m_it = rhs.m_it;
    m_isMinRange = rhs.m_isMinRange;
    m_isMaxRange = rhs.m_isMaxRange;
    m_minRange = rhs.m_minRange;
    m_maxRange = rhs.m_maxRange;
    m_featuresTypes = rhs.m_featuresTypes;
    // Store the  DTM handle and add a reference
    m_dtmP = rhs.m_dtmP;
    // Store the tinHandle (optimization purpose)
    m_dtmHandleP = rhs.m_dtmHandleP;



    // Initialize feature scanning indicators
    m_Position1 = rhs.m_Position1;
    m_Position2 = rhs.m_Position2;
    m_Position3 = rhs.m_Position3;
    m_CurrentFeatureTypeIterator = rhs.m_CurrentFeatureTypeIterator;
    m_IsAtEnd = rhs.m_IsAtEnd;
    m_CurrentFeature = rhs.m_CurrentFeature;
    }
