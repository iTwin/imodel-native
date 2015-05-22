/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Scanner.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteriaP ScanCriteria::Create ()
    {
    return new ScanCriteria ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ScanCriteriaP ScanCriteria::Clone (ScanCriteriaCR sc)
    {
    return new ScanCriteria (sc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::Delete (ScanCriteriaP scP)
    {
    delete scP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::SetDgnModel (DgnModelP model)
    {
    if (NULL == (m_model = model))
        return DGNMODEL_STATUS_BadModel;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetElementCallback (PFScanElementCallback callbackFunc, CallbackArgP callbackArg)
    {
    m_callbackArg  = callbackArg;
    m_callbackFunc = callbackFunc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetRangeTest (DRange3dP srP)
    {
    m_type.testSkewScan = false;
    if (NULL != srP)
        {
        m_range[0] = *srP;
        m_type.testRange = 1;
        }
    else
        {
        m_type.testRange = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetSkewRangeTest (DRange3dP mainRange, DRange3dP skewRange, DPoint3dP skewVector)
    {
    if (NULL != mainRange)
        SetRangeTest (mainRange);

    if ( (NULL != skewRange) && (NULL != skewVector) )
        {
        m_skewRange = *skewRange;
        m_range[1] = *skewRange;
        m_skewVector = *skewVector;
        m_type.testSkewScan = 1;
        m_numRanges = 1;
        }
    else
        {
        m_type.testSkewScan = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void ScanCriteria::SetCategoryTest(DgnCategoryIdSet const& categories)
    {
    m_categories = &categories;
    m_type.testCategory = 1;
    }
