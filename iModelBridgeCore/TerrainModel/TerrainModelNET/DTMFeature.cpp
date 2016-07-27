/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMFeature.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "StdAfx.h"
#include < vcclr.h >
#include ".\dtmfeature.h"
#include ".\dtmexception.h"
#using <mscorlib.dll>
#include "DTMHelpers.h"

using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

#pragma warning (disable:4458)

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
DTMFeature::DTMFeature(BcDTMFeature* dtmFeature)
    {
    m_dtmFeature = dtmFeature;
    m_dtmFeature->AddRef();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
DTMFeature::~DTMFeature()
    {
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
DTMFeature::!DTMFeature()
    {
    if(m_dtmFeature)
        {
        m_dtmFeature->Release();
        m_dtmFeature = nullptr;
        }
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureId DTMFeature::Id::get ()
    {
    const ::DTMFeatureId& Id = m_dtmFeature->GetIdent();
    return DTMFeatureId (Id);
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
::DTMUserTag  DTMFeature::UserTag::get ()
    {
    return m_dtmFeature->GetUserTag ();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMFeatureType DTMFeature::FeatureType::get ()
    {
    return (DTMFeatureType)m_dtmFeature->GetFeatureType();
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int DTMFeature::ElementsCount::get ()
    {
    return 1;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMSpot::DTMSpot (BcDTMSpot* dtmSpot) : DTMFeature(dtmSpot)
    {
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
array<BGEO::DPoint3d>^   DTMSpot::GetPoints ()
    {
    BcDTMSpot* _dtmSpot = m_dtmFeature->AsSpot();
    DPoint3d   *tPoint = NULL;
    int        nPoint; 
    array<BGEO::DPoint3d>^ resPoint = nullptr; 
    int status = _dtmSpot->GetPoints (tPoint, nPoint);

    DTMException::CheckForErrorStatus(status);
    if (status == SUCCESS)
        {
        resPoint = gcnew array<BGEO::DPoint3d>(nPoint);
        for (int iPoint = 0; iPoint < nPoint; iPoint ++)
            DTMHelpers::Copy (resPoint[iPoint], tPoint[iPoint]);
        }
    if (tPoint != NULL)
        bcMem_free (tPoint);

    return resPoint;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMLinearFeature::DTMLinearFeature (BcDTMFeature* dtmLinearFeature) : DTMFeature(dtmLinearFeature)
    {
    }


//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
array<BGEO::DPoint3d>^   DTMLinearFeature::GetElementPoints (int element)
    {
    BcDTMLinearFeature* _dtmLinearFeature = m_dtmFeature->AsLinear();
    DPoint3d   *tPoint = NULL;
    int        nPoint; 
    array<BGEO::DPoint3d>^ resPoint = nullptr; 
    int status = _dtmLinearFeature->GetDefinitionPoints(tPoint, nPoint);

    DTMException::CheckForErrorStatus(status);
    if (status == SUCCESS)
        {
        resPoint = gcnew array<BGEO::DPoint3d>(nPoint);
        for (int iPoint = 0; iPoint < nPoint; iPoint ++)
            DTMHelpers::Copy (resPoint[iPoint], tPoint[iPoint]);
        }
    if (tPoint != NULL)
        bcMem_free (tPoint);

    return resPoint;
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
DTMComplexLinearFeature::DTMComplexLinearFeature (BcDTMComplexLinearFeature* DTMComplexLinearFeature) : DTMLinearFeature(DTMComplexLinearFeature)
    {
    }

//=======================================================================================
// @bsimethod                                               Sylvain.Pucci      5/2007
//=======================================================================================
int DTMComplexLinearFeature::ElementsCount::get ()
    {
    BcDTMComplexLinearFeature* _DTMComplexLinearFeature = m_dtmFeature->AsComplexLinear();
    return _DTMComplexLinearFeature->GetComponentCount();
    }

//=======================================================================================
// @bsimethod                                               Daryl.Holmwood     3/2010
//=======================================================================================
array<BGEO::DPoint3d>^   DTMComplexLinearFeature::GetElementPoints (int element)
    {
    BcDTMComplexLinearFeature* _DTMComplexLinearFeature = m_dtmFeature->AsComplexLinear();
    DPoint3d   *tPoint = NULL;
    int        nPoint; 
    array<BGEO::DPoint3d>^ resPoint = nullptr; 
    int status = _DTMComplexLinearFeature->GetDefinitionPoints(tPoint, nPoint, element);

    DTMException::CheckForErrorStatus(status);
    if (status == SUCCESS)
        {
        resPoint = gcnew array<BGEO::DPoint3d>(nPoint);
        for (int iPoint = 0; iPoint < nPoint; iPoint ++)
            DTMHelpers::Copy (resPoint[iPoint], tPoint[iPoint]);
        }
    if (tPoint != NULL)
        bcMem_free (tPoint);

    return resPoint;
    }

END_BENTLEY_TERRAINMODELNET_NAMESPACE
