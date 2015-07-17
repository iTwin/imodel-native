/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/bcDTMFeatureClass.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <math.h>

#include <TerrainModel/TerrainModel.h>

#include <bcDTMBaseDef.h> 

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

/*------------------------------------------------------------------+
| Include COGO definitions                                          |
+------------------------------------------------------------------*/
#include <bcDTMBaseDef.h> 
#include <TerrainModel\Core\TMTransformHelper.h>

USING_NAMESPACE_BENTLEY_TERRAINMODEL
/*==================================================================*/
/*                                                                  */
/*          INTERNAL FUNCTIONS                                      */
/*                                                                  */
/*==================================================================*/

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMComplexLinearFeaturePtr BcDTMComplexLinearFeature::Create
(
DTMFeatureType  featureType,
DTMUserTag    userTagP,
DTMFeatureId  featureId,
DPoint3dCP    point,
int           nbPt
)
    {
    // Conctruct a feature and return it
    return new BcDTMComplexLinearFeature (featureType, userTagP, featureId, point, nbPt);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMComplexLinearFeaturePtr BcDTMComplexLinearFeature::Create
(
DTMFeatureType featureType,
DTMUserTag     userTagP,
DTMFeatureId   featureId,
const DtmString* pLinearElement,
int			   nbLinearElement
)
    {
    // Conctruct a feature and return it
    return new BcDTMComplexLinearFeature (featureType, userTagP, featureId, pLinearElement, nbLinearElement);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMComplexLinearFeature::BcDTMComplexLinearFeature
(
DTMFeatureType featureType,
DTMUserTag    userTag,
DTMFeatureId  featureId,
DPoint3dCP    point,
int           nbPt
) : BcDTMFeature (featureType, userTag, featureId)
    {
    AppendElement (DtmString (point, nbPt));
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMComplexLinearFeature::BcDTMComplexLinearFeature
(
DTMFeatureType featureType,
DTMUserTag       userTag,
DTMFeatureId     featureId,
const DtmString* pLinearElement,
int              nbLinearElement
) : BcDTMFeature (featureType, userTag, featureId)
    {
    for (int iElem = 0; iElem < nbLinearElement; iElem++)
        AppendElement (pLinearElement[iElem]);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|	Append element to a complex element                                 |
|																		|
|   cbe.04apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMComplexLinearFeature::AppendElement
(
    const DtmString&    elmt
)
{
//int         index;

    // Initialize value
    //index = 0;
    //if (!m_elmList.empty ())
    //{
    //    // Get index value
    //    index = (*(_elmList.end()-1)).getIndex () + 1;
    //}
    // Add the elements in the tables
    //DtmStringBcDtmLinearElDescr elDescr (elmt, index);

    //////if (m_transformHelper.IsValid ())
    //////    m_transformHelper->convertPointsFromDTM (_elDescr.getElementReference ().pointsP, _elDescr.getElementReference ().numPoints);
    // Add the element in the element vector
    m_elmList.push_back (elmt);

    return DTM_SUCCESS;
}
/*----------------------------------------------------------------------+
|                                                                       |
|	Gets a Linear Feature Component count                               |
|																		|
|   cbe.04apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMComplexLinearFeature::GetComponentCount () const
{
    return (int)m_elmList.size();
}
/*----------------------------------------------------------------------+
|                                                                       |
|	Gets a Linear Feature Component by Index                            |
|																		|
|   cbe.04apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMComplexLinearFeature::GetComponentByIndex
(
    DtmString&              elemPP,
    int                     index
) const
{
    // Check argument
    if (index < 0 || index >= (int)m_elmList.size()) return DTM_ERROR;

    elemPP = m_elmList[index];

    return DTM_SUCCESS;
}

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMComplexLinearFeature::GetDefinitionPoints
(
 DPoint3dP&	pointsPP, 
 int&		nbPtP,
 int index
 )
    {
    DtmString ElP;
    int status = GetComponentByIndex (ElP, index);
    if(status == DTM_SUCCESS)
        {
        nbPtP = (int)ElP.size();
        pointsPP = (DPoint3dP)bcMem_malloc (sizeof(DPoint3d) * nbPtP);
        memcpy (pointsPP, ElP.data (), sizeof(DPoint3d) * nbPtP);
        }

    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMLinearFeaturePtr BcDTMLinearFeature::Create
(
 DTMFeatureType      feature,
 DTMUserTag      userTagP, 
 DTMFeatureId    featureId,
 DPoint3dCP      point, 
 int             nbPt
 )
    {
    // Conctruct a feature and return it
    return new BcDTMLinearFeature (feature, userTagP, featureId, point, nbPt);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMLinearFeature::BcDTMLinearFeature 
(
 DTMFeatureType    featureType,
 DTMUserTag   userTag, 
 DTMFeatureId featureId,
 DPoint3dCP   point, 
 int          nbPt
 ) : BcDTMFeature (featureType, userTag, featureId)
    {
    m_linearElP = DtmString (point, nbPt);
    }
/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMLinearFeature::GetDefinitionPoints
(
 DPoint3dP&	pointsPP, 
 int&		nbPtP
 )
    {
    nbPtP = (int)m_linearElP.size();
    pointsPP = (DPoint3d*)bcMem_malloc (sizeof(DPoint3d)* nbPtP);
    memcpy (pointsPP, m_linearElP.data(), sizeof(DPoint3d)* nbPtP);

    return DTM_SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMSpotPtr BcDTMSpot::Create
(
DTMUserTag         userTagP,
DTMFeatureId       identP,
DTMFeatureType     featureTypeP,
DPoint3dCP 	       points,
int                nPt
)
    {
    // Create a spot
    return new BcDTMSpot (userTagP, identP, featureTypeP, points, nPt);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
BcDTMSpot::BcDTMSpot
(
DTMUserTag      userTag,
DTMFeatureId    featureId,
DTMFeatureType  featureType,
DPoint3dCP      points,
int             nPt
) : BcDTMFeature (featureType, userTag, featureId)
    {
    m_points = DtmString (points, nPt);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   spu.12dec2002   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMSpot::GetPoints
(
DPoint3dP&	pointsPP,
int&		nPtP
)
    {
    nPtP = (int)m_points.size ();
    pointsPP = (DPoint3d*)bcMem_malloc (sizeof(DPoint3d)* nPtP);
    memcpy (pointsPP, m_points.data(), sizeof(DPoint3d)* nPtP);
    return DTM_SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|	Append element to a complex element                                 |
|																		|
|   cbe.04apr2005   -  Created.                                         |
|                                                                       |
+----------------------------------------------------------------------*/
int BcDTMSpot::AppendPoints (DPoint3dCP pointsP, int nPt)
    {
    int lastPt = (int)m_points.size ();
    m_points.resize (lastPt + nPt);
    memcpy (&m_points[lastPt], pointsP, nPt * sizeof (DPoint3d));

    return DTM_SUCCESS;
    }



