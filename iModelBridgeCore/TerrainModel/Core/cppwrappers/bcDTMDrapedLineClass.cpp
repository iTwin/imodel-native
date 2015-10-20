/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMDrapedLineClass.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <math.h>

/*----------------------------------------------------------------------+
| Include BCivil general header files                                   |
+----------------------------------------------------------------------*/
#include <bcMacros.h>
#include <bcGmcNorm.h>

/*------------------------------------------------------------------+
| Include of the current class header                               |
+------------------------------------------------------------------*/
#include "bcDTMImpl.h"
#include "bcDTM.h"
#include "bcMem.h"
#include "bcUtil.h"

/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/
#include <bcDTMBaseDef.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL

/*----------------------------------------------+
| Import class definitions                      |
+----------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   Return code of a draped point                                       |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMDrapedLineCode BcDTMDrapedLinePoint::_GetCode () const
{
    return _code;
};

/*----------------------------------------------------------------------+
|                                                                       |
|   Return code of a draped point                                       |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMUserTag BcDTMDrapedLinePoint::GetBcDTMUserTag () const
{
    DTMUserTag   returnValue;

    returnValue = 0;
    for (int iTag = 0; iTag < (int)_features.size (); iTag++)
    {
        // TEMPO CBE
    if (_features[iTag].dtmUserTag != 0 && _features[iTag].dtmUserTag != DTM_NULL_USER_TAG)
        {
        returnValue = _features[iTag].dtmUserTag;
            break;
        }
    }

    return returnValue;
};

/*----------------------------------------------------------------------+
|                                                                       |
|   Return code of a draped point                                       |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
#if (_MSC_VER < 1300)
#else
DTMFeatureId BcDTMDrapedLinePoint::GetBcDTMFeatureId () const
{
    DTMFeatureId   returnValue  = DTM_NULL_FEATURE_ID;

    for (int iTag = 0; iTag < (int)_features.size (); iTag++)
    {
        if (_features[iTag].dtmFeatureId != DTM_NULL_FEATURE_ID )
        {
        returnValue = _features[iTag].dtmFeatureId;
            break;
        }
    }

    return returnValue;
};
#endif

#ifdef ToDo //Vancovuer
/*----------------------------------------------------------------------+
|                                                                       |
|   Add Feature to a draped point                                       |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMDrapedLinePoint::AddFeature
(
    DTMFeatureType  feature,
    DTMUserTag      userTag,
    DTMFeatureId    featureId
)
{
    PtFeature   singleFeature;

    // Assign Feature and Element Id
    singleFeature.feature = feature   ;
    singleFeature.guId    = featureId ;
    singleFeature.userTag = userTag   ;

    // Add the feature to the feature array
    _features.push_back (singleFeature);

    // Set breakLine member according to the feature

    if( _breakLine == false && ( feature == DTMFeatureType::Breakline || feature == DTMFeatureType::SoftBreakline ))
    {
        _breakLine = true ;
    }
};
#endif
/*----------------------------------------------------------------------+
|                                                                       |
|   Add Feature to a draped point                                       |
|                                                                       |
|   rsc.29jul201   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
void BcDTMDrapedLinePoint::AddFeature
(
    int             dtmFeatureIndex,
    DTMFeatureType  dtmFeatureType,
    DTMUserTag    dtmFeatureUserTag,
    DTMFeatureId  dtmFeatureId,
    int             priorFeaturePoint,
    int             nextFeaturePoint
)
{
    PtFeature   singleFeature;

    // Assign Feature and Element Id

    //ToDo Vancouver is this needed    singleFeature.feature = dtmFeatureIndex;
    //ToDo Vancouver is this needed    singleFeature.guId = dtmFeatureId;
    //ToDo Vancouver is this needed    singleFeature.userTag = dtmFeatureUserTag ;


    singleFeature.dtmFeatureIndex    =    dtmFeatureIndex ;
    singleFeature.dtmFeatureType     =    dtmFeatureType ;
    singleFeature.dtmUserTag         =    dtmFeatureUserTag ;
    singleFeature.dtmFeatureId       =    dtmFeatureId ;
    singleFeature.priorFeaturePoint  =    priorFeaturePoint ;
    singleFeature.nextFeaturePoint   =    nextFeaturePoint ;

    // Add the feature to the feature array
    _features.push_back (singleFeature);

    // Set breakLine member according to the feature

    if( _breakLine == false && ( dtmFeatureType == DTMFeatureType::Breakline || dtmFeatureType == DTMFeatureType::SoftBreakline ))
    {
        _breakLine = true ;
    }
};

/*----------------------------------------------------------------------+
|                                                                       |
|   Add Feature to a draped point                                       |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLinePoint::_GetPointCoordinates (DPoint3d& coordP) const
{
    coordP = GetPoint ();

    return DTM_SUCCESS;
};


DTMStatusInt BcDTMDrapedLine::_GetPointByIndex(Bentley::TerrainModel::DTMDrapedLinePointPtr& ret, unsigned int index) const
    {
    // Create the new point
    if(index > (unsigned int)const_cast<BcDTMDrapedLine*>(this)->_GetPointCount())
        return DTM_ERROR;
    ret = _drapedPoints[index];
    return DTM_SUCCESS;
    }

DTMStatusInt BcDTMDrapedLine::_GetPointByIndex (DPoint3dR ptP, double *distanceP, DTMDrapedLineCode *codeP, unsigned int index) const
    {
    if (index > (unsigned int)_drapedPoints.size ())
        return DTM_ERROR;
    ptP = _drapedPoints[index]->GetPoint ();
    if (distanceP) *distanceP = _drapedPoints[index]->GetDistance ();
    if (codeP) *codeP = _drapedPoints[index]->GetCode ();
    return DTM_SUCCESS;
    }

unsigned int BcDTMDrapedLine::_GetPointCount() const
    {
    return (unsigned int)_drapedPoints.size();
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   Add a point in the draped line                                      |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::AddPointInTable
(
    int                  index,
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>>  &selPoint
)
{
   BC_START ();

   // Just push the point in the vector
   selPoint.push_back (_drapedPoints[index]);

   BC_END:;

   BC_END_RETURNSTATUS();
}

/*----------------------------------------------------------------------+
|                                                                       |
|   Add a point in the draped line by z interpolation                   |
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::AddPointInTableByZInterpolation
(
    int                  index,         /* => Index of the point                 */
    double               distance,      /* => Abcissa of the current point      */
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>>  &selPoint
)
{
    DPoint3d    point;
    DTMDrapedLineCode code;

    BC_START ();

    // Do not care about XY
    point.x = dc_zero;
    point.y = dc_zero;

    if ((_drapedPoints[index+1]->GetDistance () - _drapedPoints[index]->GetDistance ()) != dc_zero)
    {
        // If distance between points is not zero, we make the interpolation
        point.z = _drapedPoints[index]->GetPoint ().z +
            ((distance - _drapedPoints[index]->GetDistance ()) *
            (_drapedPoints[index+1]->GetPoint ().z - _drapedPoints[index]->GetPoint ().z) /
            (_drapedPoints[index+1]->GetDistance () - _drapedPoints[index]->GetDistance ()));
    }
    else
    {
        // If distance == zero, we take the elevation of the current point
        point.z = _drapedPoints[index]->GetPoint ().z;
    }

    // Set code to internal DTMFeatureState::Tin
    code = DTMDrapedLineCode::Tin;

    // Create the point
    RefCountedPtr<BcDTMDrapedLinePoint> pt = BcDTMDrapedLinePoint::Create (point, distance, code);

    // Add the point to the vector
    //BC_TRY (_addPointInTable (index, selPoint));
    // Just push the point in the vector
    selPoint.push_back (pt);

    BC_END:;

    BC_END_RETURNSTATUS();
}


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.14jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::_GetProfileSections
(
DPoint3d            **xyzPointsPP,
DPoint3d            **profilePointsPP,
DTMDrapedLineCode   **tabCodePP,
int                 **profileEndIndexPP,
int                 *nSectionP,
DTMDrapedLineFlag   flagPt,        /* => Point selection flag             */
double              distArray[],   /* => Table of distance                */
int                 nbDist         /* => Number of distance               */
)
    {
    int         iPt;
    bool     endSection;
    bool     startSection;
    int         i;
    int         iAbc;
    int         currDistIndex;
    bvector<RefCountedPtr<BcDTMDrapedLinePoint>> selectedDrapedPoints;

    BC_START ();

    // Check arguments
    if (profilePointsPP == NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);
    if (profileEndIndexPP == NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);
    if (nSectionP == NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);

    if (*profilePointsPP != NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);
    if (xyzPointsPP != NULL)
        {
        if (*xyzPointsPP != NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);
        }
    if (tabCodePP != NULL)
        {
        if (*tabCodePP != NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);
        }
    if (*profileEndIndexPP != NULL) BC_RETURN_ERRSTATUS (DTM_ERROR);

    // Initalize results
    *nSectionP = 0;

    currDistIndex = 0;

    // Initialize start ans end section flags
    endSection = FALSE;
    startSection = TRUE;

    // Walk through the list of draped points and generate the sections....
    for (iPt = 0; iPt < (int)_drapedPoints.size (); iPt++)
        {
        DTMDrapedLineCode code = _drapedPoints[iPt]->GetCode ();
        switch (flagPt)
            {
            case DTMDrapedLineFlag::SpecificPoints:

                // In this case we just want the point which are defined by the distance array
                // So, if there is no distance array, there is an error
                if (distArray == NULL || nbDist <= 0)
                    {
                    BC_RETURN_ERRSTATUS (DTM_ERROR);
                    };

                if (
                    code == DTMDrapedLineCode::Tin
                    ||
                    code == DTMDrapedLineCode::Breakline
                    ||
                    code == DTMDrapedLineCode::BetweenBreaklines
                    )
                    {
                    // If the point is on the triangulation it is added to the
                    // line, and we say that we have not reached the end of a
                    // section
                    endSection = FALSE;
                    if (startSection)
                        {
                        // If the point is the first of the section, we add it
                        BC_TRY (
                            AddPointInTable (iPt, selectedDrapedPoints));
                        }
                    if (iPt < (int)_drapedPoints.size () - 1)
                        {
                        // If the current distance is between current point and next
                        // point, we add the point by z interpolation
                        for (iAbc = currDistIndex; iAbc < nbDist; iAbc++)
                            {
                            if (distArray[iAbc] > _drapedPoints[iPt + 1]->GetDistance ()) break;
                            if (distArray[iAbc] > _drapedPoints[iPt]->GetDistance () &&
                                _drapedPoints[iPt + 1]->GetCode () != DTMDrapedLineCode::Void)
                                {
                                BC_TRY (
                                    AddPointInTableByZInterpolation (iPt, distArray[iAbc], selectedDrapedPoints));
                                }
                            }
                        // Update current distance index
                        currDistIndex = iAbc;
                        }
                    startSection = FALSE;
                    }
                if (code == DTMDrapedLineCode::Void && endSection == FALSE && iPt > 0)
                    {
                    // If the point is not on the triangulation and we have not
                    // yet reached the end of a section, we say that we have
                    // reached the end of a section.
                    endSection = TRUE;
                    if (!startSection)
                        {
                        BC_TRY (
                            AddPointInTable (iPt - 1, selectedDrapedPoints));
                        }
                    }
                break;

            case DTMDrapedLineFlag::Tin:

                if (code == DTMDrapedLineCode::Tin ||
                    code == DTMDrapedLineCode::Breakline ||
                    code == DTMDrapedLineCode::BetweenBreaklines ||
                    code == DTMDrapedLineCode::OnPoint ||
                    code == DTMDrapedLineCode::Edge)
                    {
                    // If the point is on the triangulation it is added to the
                    // line, and we say that we have not reached the end of a
                    // section
                    endSection = FALSE;
                    BC_TRY (
                        AddPointInTable (iPt, selectedDrapedPoints));
                    }
                if ((code == DTMDrapedLineCode::External || code == DTMDrapedLineCode::Void) && endSection == FALSE)
                    {
                    // If the point is not on the triangulation and we have not
                    // yet reached the end of a section, we say that we have
                    // reached the end of a section.
                    endSection = TRUE;
                    }
                break;

            case DTMDrapedLineFlag::Breakline:

                if (code == DTMDrapedLineCode::Tin ||
                    code == DTMDrapedLineCode::Breakline ||
                    code == DTMDrapedLineCode::OnPoint ||
                    code == DTMDrapedLineCode::Edge)

                    {
                    // If the point is on the triangulation but not between two
                    // break lines, we add the point and we say that we have not
                    // reached the end of a section.
                    endSection = FALSE;
                    BC_TRY (AddPointInTable (iPt, selectedDrapedPoints));
                    }
                if (code == DTMDrapedLineCode::Void && endSection == FALSE)
                    {
                    // If the point is not on the triangulation and we have not
                    // yet reached the end of a section, we say that we have
                    // reached the end of a section.
                    endSection = TRUE;
                    }
                break;

            case DTMDrapedLineFlag::Void:

                if (code == DTMDrapedLineCode::Void)
                    {
                    endSection = FALSE;
                    if (iPt > 0)
                        {
                        // We test if the previous point was not a void, if it was
                        // not, we the previous point
                        if (_drapedPoints[iPt - 1]->GetCode () != DTMDrapedLineCode::Void)
                            {
                            BC_TRY (
                                AddPointInTable (iPt - 1, selectedDrapedPoints));
                            }
                        }
                    }
                else
                    {
                    // Test if the previous point was a void, in this case add the current point
                    if (iPt > 0)
                        {
                        if (_drapedPoints[iPt - 1]->GetCode () == DTMDrapedLineCode::Void)
                            {
                            // If it was, we add the current point and say that the section is finished
                            BC_TRY (
                                AddPointInTable (iPt, selectedDrapedPoints));
                            endSection = TRUE;
                            }
                        }
                    }
                break;
            }

        if (iPt == (int)_drapedPoints.size () - 1 && !endSection)
            {
            // If this point is the last one, we say that we are at the
            // end of a section.
            endSection = TRUE;

            // If the point was a point in a section and we are in
            // DTMDrapedLineFlag::SpecificPoints case, we add it
            if (flagPt == DTMDrapedLineFlag::SpecificPoints)
                {
                BC_TRY (
                    AddPointInTable (iPt, selectedDrapedPoints));
                }
            }

        if (endSection)
            {
            // If we are at the end of a section we add it in the vertical linear element
            if (selectedDrapedPoints.size () > 1)
                {
                if (*profilePointsPP == NULL)
                    {
                    // If there are not yet any array, we create them...
                    *profilePointsPP =
                        (DPoint3d *)bcMem_calloc (selectedDrapedPoints.size (), sizeof (DPoint3d));
                    if (tabCodePP != NULL)
                        {
                        *tabCodePP =
                            (DTMDrapedLineCode*)bcMem_calloc (selectedDrapedPoints.size (), sizeof (DTMDrapedLineCode));
                        }
                    *profileEndIndexPP =
                        (int*)bcMem_calloc (1, sizeof(int));

                    // Set the end index and the number of sections
                    (*profileEndIndexPP)[0] = (int)selectedDrapedPoints.size () - 1;
                    *nSectionP = 1;

                    // Create the xyzPointPP array if required
                    if (xyzPointsPP != NULL)
                        {
                        *xyzPointsPP = (DPoint3d*)bcMem_calloc (selectedDrapedPoints.size (), sizeof (DPoint3d));
                        }
                    }
                else
                    {
                    // The total number of points is:
                    // - number of points of this section: selectedDrapedPoints.size ()
                    // +
                    // - number of points before this section: ((*profileEndIndexPP)[(*nSectionP)-1] + 1
                    int totalNumberOfPoints =
                        (int)selectedDrapedPoints.size () + (*profileEndIndexPP)[(*nSectionP) - 1] + 1;

                    // We extend the arrays
                    *profilePointsPP =
                        (DPoint3d *)bcMem_realloc (*profilePointsPP, totalNumberOfPoints * sizeof (DPoint3d));
                    if (tabCodePP != NULL)
                        {
                        *tabCodePP =
                            (DTMDrapedLineCode *)bcMem_realloc (*tabCodePP, totalNumberOfPoints * sizeof (DTMDrapedLineCode));
                        }

                    *profileEndIndexPP =
                        (int*)bcMem_realloc (*profileEndIndexPP, ((*nSectionP) + 1) * sizeof(int));

                    // Set the current end index and the number of sections
                    (*profileEndIndexPP)[*nSectionP] = totalNumberOfPoints - 1;
                    *nSectionP += 1;

                    // Extend the xyzPointsPP array if required
                    if (xyzPointsPP != NULL)
                        {
                        *xyzPointsPP = (DPoint3d*)bcMem_realloc (*xyzPointsPP, totalNumberOfPoints * sizeof (DPoint3d));
                        }
                    }

                // Set the start index for this section
                int startIndex = (*profileEndIndexPP)[(*nSectionP) - 1] - (int)selectedDrapedPoints.size () + 1;

                // Populate the arrays
                for (i = 0; i < (int)selectedDrapedPoints.size (); i++)
                    {
                    (*profilePointsPP)[startIndex + i].x = selectedDrapedPoints[i]->GetDistance ();
                    (*profilePointsPP)[startIndex + i].y = selectedDrapedPoints[i]->GetPoint ().z;
                    (*profilePointsPP)[startIndex + i].z = dc_zero;
                    if (tabCodePP != NULL)
                        {
                        (*tabCodePP)[startIndex + i] = selectedDrapedPoints[i]->GetCode ();
                        }
                    if (xyzPointsPP != NULL)
                        {
                        (*xyzPointsPP)[startIndex + i] = selectedDrapedPoints[i]->GetPoint ();
                        }
                    }

                }

            // Clear current section
            selectedDrapedPoints.clear ();
            // Set the section flag
            startSection = TRUE;
            }
        }

BC_END:;

    BC_END_RETURNSTATUS ();
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::_GetBreakLinePoints
(
    bool     selNoFeature,      // => if TRUE gets alla point, otherwise
                                    // gets only points with feature
    DPoint3d     *xyTabP[],         // <= or NULL
    DPoint3d     *szTabP[],         // <= or NULL
    DTMUserTag *userTagTabP[],    // <= Feature point table
    DTMFeatureId *guidTabP[],     // <= Feature point table
    int          *nPtP              // <= number of point                     */
)
{
int    iPt;

    BC_START ();

    *nPtP = 0;

    for (iPt = 0; iPt < (int)_drapedPoints.size (); iPt++)
    {
        if (_drapedPoints[iPt]->GetCode() == DTMDrapedLineCode::Breakline)
        {
            if ((!selNoFeature) && _drapedPoints[iPt]->GetFeatureCount() == 0)
                continue;

            *nPtP += 1;

            if (xyTabP)
            {
                if (*xyTabP == NULL)
                    *xyTabP = (DPoint3d*)bcMem_malloc (sizeof (DPoint3d));
                else
                    *xyTabP = (DPoint3d*)bcMem_realloc (*xyTabP, (*nPtP)*sizeof (DPoint3d));
                (*xyTabP)[(*nPtP)-1] = _drapedPoints[iPt]->GetPoint ();
            }
            if (szTabP)
            {
                if (*szTabP == NULL)
                    *szTabP = (DPoint3d*)bcMem_malloc (sizeof (DPoint3d));
                else
                    *szTabP = (DPoint3d*)bcMem_realloc (*szTabP, (*nPtP)*sizeof (DPoint3d));
                (*szTabP)[(*nPtP)-1].x = _drapedPoints[iPt]->GetDistance ();
                (*szTabP)[(*nPtP)-1].y = _drapedPoints[iPt]->GetPoint ().z;
                (*szTabP)[(*nPtP)-1].z = dc_zero;
            }
            if (userTagTabP)
            {
                if (*userTagTabP == NULL)
                    *userTagTabP = (DTMUserTag*)bcMem_malloc (sizeof (DTMUserTag));
                else
                    *userTagTabP = (DTMUserTag*)bcMem_realloc (*userTagTabP, (*nPtP)*sizeof (DTMUserTag));
                (*userTagTabP)[(*nPtP)-1] = _drapedPoints[iPt]->GetBcDTMUserTag ();
            }
            if (guidTabP)
            {
                if (*guidTabP == NULL)
                    *guidTabP = (DTMFeatureId*)bcMem_malloc (sizeof (DTMFeatureId));
                else
                    *guidTabP = (DTMFeatureId*)bcMem_realloc (*guidTabP, (*nPtP)*sizeof (DTMFeatureId));
#if (_MSC_VER < 1300)
#else
                (*guidTabP)[(*nPtP)-1] = _drapedPoints[iPt]->GetBcDTMFeatureId ();
#endif
            }
        }
    }

    BC_END:;

    BC_END_RETURNSTATUS();
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMDrapedLine::BcDTMDrapedLine
(
bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& drapedPoints
)
{
    _drapedPoints = drapedPoints;
    _linearElP = NULL;
    _containsVoid = -1;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMDrapedLine::BcDTMDrapedLine
(
bvector<RefCountedPtr<BcDTMDrapedLinePoint>>& drapedPoints,
    IRefCounted* linearElP
)
{
    _drapedPoints = drapedPoints;
    _linearElP = linearElP;
    _containsVoid = -1;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMDrapedLine::~BcDTMDrapedLine
(
)
{
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::_GetPointByIndex
(
    BcDTMDrapedLinePointPtr&  drapedPointPP,
    int                     index
)
{
    // Create the new point
    drapedPointPP = _drapedPoints[index];
    if (drapedPointPP.IsNull()) return DTM_ERROR;

    return DTM_SUCCESS;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03jul2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
DTMStatusInt BcDTMDrapedLine::_GetPointByIndex
(
    DPoint3d                *ptP,
    double                  *distanceP,
    DTMDrapedLineCode       *codeP,
    int                     index
)
{
    // Check argument

    if (ptP != NULL)
        *ptP = _drapedPoints[index]->GetPoint ();
    if (codeP != NULL)
        *codeP = _drapedPoints[index]->GetCode ();
    if (distanceP != NULL)
        *distanceP = _drapedPoints[index]->GetDistanceAlong ();

    return DTM_SUCCESS;
}


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.31mar2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.03mar2004   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
bool BcDTMDrapedLine::_IsPartiallyOnDTM
(
)
{
    // TODO: Check with Jay what should be this tolerance here
    double tol = 1e-5;

    if (_containsVoid == -1)
    {
        DPoint3d    *prfPointsP = NULL;
        int         *endIndexP = NULL;
        int         nSection = 0;

        _containsVoid = 1;

        int status = GetProfileSections (NULL, &prfPointsP, NULL, &endIndexP, &nSection, DTMDrapedLineFlag::Void, NULL, 0);
        if (status != DTM_SUCCESS)
            return FALSE;

        if (prfPointsP != NULL)
            bcMem_freeAndClear((void **)&prfPointsP);
        if (endIndexP != NULL)
            bcMem_freeAndClear ((void **)&endIndexP);

        if (nSection > 0)
        {
            _containsVoid = 2;
        }
        else
        {
            DPoint3d    *xyzPointsP = NULL;

            int status = GetProfileSections (&xyzPointsP, &prfPointsP, NULL, &endIndexP, &nSection, DTMDrapedLineFlag::Tin, NULL, 0);
            if (status != DTM_SUCCESS)
                return FALSE;

            if (xyzPointsP != NULL)
            {
                int lastPt = (int)this->_drapedPoints.size() - 1;
                int lastPtSect = endIndexP[nSection-1];
                if (!(
                    bcUtil_equalDouble (xyzPointsP[0].x, this->_drapedPoints[0]->GetPoint().x, tol) &&
                    bcUtil_equalDouble (xyzPointsP[0].y, this->_drapedPoints[0]->GetPoint().y, tol)
                   ))
                    _containsVoid = 2;
                else if (!(
                    bcUtil_equalDouble (xyzPointsP[lastPtSect].x, this->_drapedPoints[lastPt]->GetPoint().x, tol) &&
                    bcUtil_equalDouble (xyzPointsP[lastPtSect].y, this->_drapedPoints[lastPt]->GetPoint().y, tol)
                    ))
                    _containsVoid = 2;
            }
            else
            {
                _containsVoid = 2;
            }

            if (prfPointsP != NULL)
                bcMem_freeAndClear((void **)&prfPointsP);
            if (xyzPointsP != NULL)
                bcMem_freeAndClear((void **)&xyzPointsP);
            if (endIndexP != NULL)
                bcMem_freeAndClear ((void **)&endIndexP);
        }

    }

    return _containsVoid == 2;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLinePoint::GetUserTagCount () const
    {
    return _GetUserTagCount ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Int64 BcDTMDrapedLinePoint::GetUserTagAtIndex (int index) const
    {
    return _GetUserTagAtIndex (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLinePoint::GetFeatureIdCount () const
    {
    return _GetFeatureIdCount ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMFeatureId BcDTMDrapedLinePoint::GetFeatureIdAtIndex (int index) const
    {
    return _GetFeatureIdAtIndex (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Rob.Cormack     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLinePoint::GetFeatureIndexAtIndex (int index) const
    {
    return _GetFeatureIndexAtIndex (index);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Rob.Cormack     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
DTMFeatureType BcDTMDrapedLinePoint::GetFeatureTypeAtIndex (int index) const
    {
    return _GetFeatureTypeAtIndex (index);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Rob.Cormack     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLinePoint::GetFeaturePriorPointAtIndex (int index) const
    {
    return _GetFeaturePriorPointAtIndex (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Rob.Cormack     07/11
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLinePoint::GetFeatureNextPointAtIndex (int index) const
    {
    return _GetFeatureNextPointAtIndex (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTMDrapedLine::GetProfileSections (DPoint3d **xyzPointsP, DPoint3d **profilePointsP, DTMDrapedLineCode **tabCodeP, int **profileEndIndexP, int *nSectionP, DTMDrapedLineFlag flagPt, double abcArray[], int nbAbc)
    {
    return _GetProfileSections(xyzPointsP, profilePointsP, tabCodeP, profileEndIndexP, nSectionP, flagPt, abcArray, nbAbc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTMDrapedLine::GetProfileSections (DPoint3d **profilePointsP, DTMDrapedLineCode **tabCodeP, int **profileEndIndexP, int *nSectionP, DTMDrapedLineFlag flagPt, double abcArray[], int nbAbc)
    {
    return _GetProfileSections(NULL, profilePointsP, tabCodeP, profileEndIndexP, nSectionP, flagPt, abcArray, nbAbc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTMDrapedLine::GetBreakLinePoints (bool selNoFeature, DPoint3d *xyTabP[], DPoint3d *szTabP[], DTMUserTag *userTagTabP[], DTMFeatureId *guidTabP[], int *nPtP)
    {
    return _GetBreakLinePoints(selNoFeature, xyTabP, szTabP, userTagTabP, guidTabP, nPtP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTMDrapedLine::GetPointByIndex (BcDTMDrapedLinePointPtr& drapedPointPP, int index)
    {
    return _GetPointByIndex(drapedPointPP, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
DTMStatusInt BcDTMDrapedLine::GetPointByIndex (DPoint3d *ptP, double *distanceP, DTMDrapedLineCode *codeP, int index)
    {
    return _GetPointByIndex(ptP, distanceP, codeP, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
int BcDTMDrapedLine::GetPointCount()
    {
    return _GetPointCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool BcDTMDrapedLine::IsPartiallyOnDTM ()
    {
    return _IsPartiallyOnDTM ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::TerrainModel::IDTMDrapedLine* BcDTMDrapedLine::GetIDTMDrapedLine()
    {
    return _GetIDTMDrapedLine();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  09/10
+---------------+---------------+---------------+---------------+---------------+------*/
IRefCounted* BcDTMDrapedLine::GetIRefCounted ()
    {
    return _GetIRefCounted ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Daryl.Holmwood  02/11
+---------------+---------------+---------------+---------------+---------------+------*/
void BcDTMDrapedLine::SetIRefCounted (IRefCounted* obj)
    {
    return _SetIRefCounted (obj);
    }
