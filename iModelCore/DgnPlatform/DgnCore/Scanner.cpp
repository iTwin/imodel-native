/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Scanner.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

ScanCriteria* ScanCriteria::s_legacyScanCriteria = NULL;

#define     ELELOCKED       0x0100      /* set if locked */
#define     ELEATTR         0x0800      /* attribute data present */

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
    m_newCriteria  = 1;

    if (NULL == (m_model = model))
        return DGNMODEL_STATUS_BadModelPtr;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::SetReturnType (int returnType, int oneElementOnly, int nestCells)
    {
    switch (returnType)
        {
        case MSSCANCRIT_ITERATE_ELMREF:
            m_type.iteration = 2;
            break;
        case MSSCANCRIT_ITERATE_ELMDSCR:
            m_type.iteration = 1;
            break;
        case MSSCANCRIT_ITERATE_ELMREF_UNORDERED:
            m_type.iteration = 3;
            break;
        default:
            {
            BeAssert(0);
            return ERROR;
            }
        }

    m_type.returnOneElem = oneElementOnly;

    // it doesn't make any sense to iterate unless we're also "nesting" cells
    m_type.nestCells = (0 != m_type.iteration) ? true : nestCells;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetElmDscrCallback (PFScanElemDscrCallback callbackFunc, void* callbackArg)
    {
    m_callbackArg  = callbackArg;
    m_callbackFunc = callbackFunc;
    m_type.iteration = (NULL != callbackFunc) ? 1 : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetElemRefCallback (PFScanElemRefCallback callbackFunc, CallbackArgP callbackArg)
    {
    m_callbackArg  = callbackArg;
    m_callbackFunc = (PFScanElemDscrCallback) callbackFunc;

    if (NULL != callbackFunc)
        {
        //  Don't override a value already set by SetReturnType
        if (3 != m_type.iteration)
            m_type.iteration = 2;
        }
    else
        m_type.iteration = 0;
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
StatusInt       ScanCriteria::SetExtendedRangeTest (DRange3dP srP, int rangeNum)
    {
    if ((0 > rangeNum) || (MAX_SC_RANGE <= rangeNum))
        return ERROR; //STYLETABLE_ERROR_BadIndex;

    if (NULL != srP)
        {
        m_range[rangeNum] = *srP;
        m_type.testRange = 1;

        /* set to highest range number ever used */
        if (m_numRanges < (rangeNum+1))
            m_numRanges = rangeNum+1;
        }
    else
        {
        DRange3dP setInvalidSrP = &m_range[rangeNum];

        /* invalidate range by setting low.x > high.x */
        setInvalidSrP->low.x = 1000;
        setInvalidSrP->high.x = 0;
        if (m_numRanges == (rangeNum+1) && (m_numRanges > 0))
            {
            m_numRanges--;
            if (0 == m_numRanges)
                m_type.testRange = 0;
            }
        }
    m_type.testMultiRange = (m_numRanges > 1) ? 1 : 0;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::GetExtendedRangeTest (DRange3dP srP, int rangeNum) const
    {
    if ((0 > rangeNum) || (MAX_SC_RANGE <= rangeNum))
        return ERROR; //STYLETABLE_ERROR_BadIndex;

    if (m_type.testMultiRange)
        {
        if (rangeNum >= m_numRanges)
            return ERROR; //STYLETABLE_ERROR_BadIndex;
        *srP = m_range[rangeNum];
        }
    else if (m_type.testRange)
        {
        if (rangeNum > 0)
            return  ERROR; //STYLETABLE_ERROR_BadIndex;
        *srP = m_range[0];
        }
    else
        {
        return  ERROR; //STYLETABLE_ERROR_BadIndex;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetAttributeTest (UShort entity, long  occurrence, ExtendedAttrBuf* extAttrBuf)
    {
    if (0 != (m_entity = entity))
        m_type.testAttributeEntity = 1;
    else
        m_type.testAttributeEntity = 0;

    if (0 != (m_occurrence = occurrence))
        m_type.testAttributeOccurrence = 1;
    else
        m_type.testAttributeOccurrence = 0;

    if ((NULL != extAttrBuf) && (0 < extAttrBuf->numWords) && (2048 >= extAttrBuf->numWords))
        {
        int memsize;

        // free existing attribute buffer if there is one.
        if (NULL != m_extAttrBuf)
            memutil_free (m_extAttrBuf);

        // allocate the extended attribute buffer based on the words the user passes in.
        memsize = sizeof(UShort) + (extAttrBuf->numWords * 2) * sizeof(UShort);
        m_extAttrBuf = (ExtendedAttrBuf *) memutil_malloc (memsize, 'SCRT');
        memcpy (m_extAttrBuf, extAttrBuf, memsize);
        m_type.testAttributeExtended = 1;
        }
    else
        {
        if (NULL != m_extAttrBuf)
            memutil_free (m_extAttrBuf);
        m_extAttrBuf = NULL;
        m_type.testAttributeExtended = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman 12/04
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetXAttributeTest (XAttributeHandlerId* handlerId, UInt32 attrId)
    {
    m_type.testXAttributes = (NULL == handlerId ? 0 : 1);
    m_xAttrHandlerId = *handlerId;
    m_xAttrId        = attrId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetPriorityTest (UInt32 minPriority, UInt32 maxPriority)
    {
    if (minPriority <= maxPriority)
        {
        m_minPriority = minPriority;
        m_maxPriority = maxPriority;
        m_type.testPriority = 1;
        }
    else
        m_type.testPriority = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetTimeTest (double minModTime, double maxModTime)
    {
    if (minModTime <= maxModTime)
        {
        m_minModTime = minModTime;
        m_maxModTime = maxModTime;
        m_type.testModTime = 1;
        }
    else
        m_type.testModTime = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetCellNameTest (WCharCP cellName)
    {
    if (NULL != cellName)
        {
        m_cellName = BeStringUtilities::Wcsdup (cellName);
        m_type.testCellName = 1;
        }
    else
        {
        if (NULL != m_cellName)
            free (m_cellName);
        m_type.testCellName = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            JeffBrown 04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::ClearLevelMask ()
    {
    if (NULL == m_levelBitMask)
        {
        m_levelBitMask = BitMask::Create (false);
        m_type.freeLevelBitMaskWhenDone = true;
        }
    m_levelBitMask->SetAll (false); // FYI, this also sets the default bit value to false
                                    // (which we want it to be, anyway).
    m_type.testLevel = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::AddSingleLevelTest (LevelId level)
    {
    if (!m_type.testLevel || m_levelBitMask == NULL)
        ClearLevelMask();

    m_levelBitMask->Set (level.GetValue() - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JeffBrown    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::RemoveSingleLevelTest (LevelId level)
    {
    if (m_type.testLevel && m_levelBitMask != NULL)
        m_levelBitMask->Clear (level.GetValue() - 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetLevelTest (BitMaskP levelBitMask, bool freeWhenDone)
    {
    if (NULL == levelBitMask || 0 == levelBitMask->GetCapacity())
        {
        if ((NULL != m_levelBitMask) && m_type.freeLevelBitMaskWhenDone)
            BitMask::FreeAndClear (&m_levelBitMask);

        m_levelBitMask = NULL;
        m_type.freeLevelBitMaskWhenDone = false;
        m_type.testLevel = 0;
        }
    else if (levelBitMask != m_levelBitMask)
        {
        if ((NULL != m_levelBitMask) && m_type.freeLevelBitMaskWhenDone)
            BitMask::FreeAndClear (&m_levelBitMask);

        m_levelBitMask = levelBitMask;
        m_type.testLevel = 1;
        m_type.freeLevelBitMaskWhenDone = freeWhenDone;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetPropertiesTest (UShort propertiesVal, UShort propertiesMask)
    {
    if (0 != propertiesMask)
        {
        m_propertiesVal          = propertiesVal;
        m_propertiesMask         = propertiesMask;
        m_type.testProperties    = 1;
        }
    else
        {
        m_propertiesVal          = 0;
        m_propertiesMask         = 0;
        m_type.testProperties    = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ShaunSewall     10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::GetProperties (int* propertiesValP, int* propertiesMaskP)
    {
    if (NULL != propertiesValP)
        *propertiesValP = m_propertiesVal;

    if (NULL != propertiesMaskP)
        *propertiesMaskP = m_propertiesMask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                            JeffBrown 04/01
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::ClearClassMask ()
    {
    m_type.testClass = true;
    m_classMask = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      09/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::AddSingleClassTest (DgnElementClass elementClass)
    {
    int nClass = static_cast <int> (elementClass);

    if (nClass < 0 || nClass >= sizeof(int) * 8)
        return ERROR;

    if (!m_type.testClass)
        ClearClassMask();

    m_classMask |= (1 << nClass);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JeffBrown    04/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ScanCriteria::RemoveSingleClassTest (DgnElementClass elementClass)
    {
    int nClass = static_cast <int> (elementClass);

    if (nClass < 0 || nClass >= sizeof(int) * 8)
        return ERROR;

    if (m_type.testClass)
        m_classMask &= ~(1 << nClass);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetClassTest (int classMask)
    {
    if (0 != (m_classMask = classMask))
        m_type.testClass = 1;
    else
        m_type.testClass = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLuz       10/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ScanCriteria::IsLevelActive (int level)
    {
    if (m_type.testLevel)
        {
        if (0 != level)
            {
            if (!m_levelBitMask->Test (level - 1))
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    08/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ScanCriteria::IsClassActive (int dgnClass)
    {
    if (m_type.testClass)
        {
        if ( !(m_classMask & (1 << dgnClass)) )
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BJB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
void            ScanCriteria::SetNestOverride (short* mask, bool operation)
    {
    short       *destP = NULL, *srcP = NULL;

    for (destP=m_overrideNest, srcP=mask; destP < &m_overrideNest[8]; destP++, srcP++)
        {
        if (operation)
            *destP |= *srcP;
        else
            *destP &= ~(*srcP);
        }
    }
