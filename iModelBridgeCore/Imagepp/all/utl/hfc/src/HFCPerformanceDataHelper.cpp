//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCPerformanceDataHelper.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCPerformanceDataHelper
//-----------------------------------------------------------------------------

//###############################
// INCLUDE FILES
//###############################

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCPerformanceDataHelper.h>


//-----------------------------------------------------------------
// HFCPerformanceDataHelper
//
// Constructor.
//-----------------------------------------------------------------
HFCPerformanceDataHelper::HFCPerformanceDataHelper()
    {
    // Do nothing.
    }

//-----------------------------------------------------------------
// ~HFCPerformanceDataHelper
//
//  Destructor. Close the query key if opened.
//-----------------------------------------------------------------
HFCPerformanceDataHelper::~HFCPerformanceDataHelper()
    {
    Close();
    }

//-----------------------------------------------------------------
// HFC_PERFORMANCE_STATUS Open
//
// Open a query key.
//-----------------------------------------------------------------
HFC_PERFORMANCE_STATUS HFCPerformanceDataHelper::Open()
    {
    HFC_PERFORMANCE_STATUS pdhStatus;

    pdhStatus = PdhOpenQuery(0, 0, &m_hQuery);
    if( pdhStatus == HFC_PERFORMANCE_ERROR_SUCCESS )
        EnumObjects();

    return pdhStatus;
    }

//-----------------------------------------------------------------
// void Close
//
// Close a query key and delete all the counter allocated for this
// query.
//-----------------------------------------------------------------
void HFCPerformanceDataHelper::Close()
    {
    PdhCloseQuery(m_hQuery);
    DeleteAllCounters();
    }


//-----------------------------------------------------------------
// HFC_PERFORMANCE_STATUS AddCounter
//
// Add a counter to a query.
//
// const string& pi_rCounterPath: The path of the counter.
// HFC_PERFORMANCE_COUNTER* pi_pCtrHandle: The pointer that will
//                                         receive the counter handle
//-----------------------------------------------------------------
HFC_PERFORMANCE_STATUS HFCPerformanceDataHelper::AddCounter(const WString& pi_rCounterPath,
                                                            HFC_PERFORMANCE_COUNTER* pi_pCtrHandle)
    {
    HFC_PERFORMANCE_STATUS pdhStatus;
    PCIB pNewCib;

    HPRECONDITION(pi_pCtrHandle != 0);

    // Create a new CIB struct and zero it
    pNewCib = new CIB;
    memset(pNewCib, 0, sizeof(CIB));

    // Try to add the counter to the query
    pdhStatus = PdhAddCounter(m_hQuery, pi_rCounterPath.c_str(), 0, &pNewCib->hCounter);

    if( pdhStatus == HFC_PERFORMANCE_ERROR_SUCCESS )
        {
        wcscpy(pNewCib->szCounterPath, pi_rCounterPath.c_str());

        pNewCib->pCounterArray = new HFC_PERFORMANCE_RAW_COUNTER[NUM_STAT_SAMPLES];
        memset(pNewCib->pCounterArray,
               0,
               sizeof(HFC_PERFORMANCE_RAW_COUNTER) * NUM_STAT_SAMPLES);

        pNewCib->dwFirstIndex  = 0;
        pNewCib->dwNextIndex   = 0;
        pNewCib->dwLastIndex   = 0;
        pNewCib->dLastValue    = 0.0f;
        pNewCib->pdhCurrentStats.count    = 0;
        pNewCib->pdhCurrentStats.dwFormat = 0;
        pNewCib->pdhCurrentStats.min.CStatus     = HFC_PERFORMANCE_CSTATUS_INVALID_DATA;
        pNewCib->pdhCurrentStats.min.largeValue  = 0;
        pNewCib->pdhCurrentStats.max.CStatus     = HFC_PERFORMANCE_CSTATUS_INVALID_DATA;
        pNewCib->pdhCurrentStats.max.largeValue  = 0;
        pNewCib->pdhCurrentStats.mean.CStatus    = HFC_PERFORMANCE_CSTATUS_INVALID_DATA;
        pNewCib->pdhCurrentStats.mean.largeValue = 0;

        *pi_pCtrHandle = pNewCib->hCounter;

        // Insert in the counter list
        m_CounterList.push_back(pNewCib);
        }
    else
        delete pNewCib;

    return pdhStatus;
    }

//-----------------------------------------------------------------
// HFC_PERFORMANCE_STATUS QueryData
//
// Execute the query.
//-----------------------------------------------------------------
HFC_PERFORMANCE_STATUS HFCPerformanceDataHelper::QueryData()
    {
    DWORD dwType;
    PCIB  pCib;
    HFC_PERFORMANCE_STATUS      pdhStatus;
    HFC_PERFORMANCE_RAW_COUNTER pRaw;
    HFC_PERFORMANCE_FMT_COUNTERVALUE pValue;

    // Get the current values of the query data
    pdhStatus = PdhCollectQueryData(m_hQuery);

    if( pdhStatus == HFC_PERFORMANCE_ERROR_SUCCESS )
        {
        for(PERFORMANCE_LIST_ITR Itr = m_CounterList.begin(); Itr != m_CounterList.end(); Itr++)
            {
            pCib = *Itr;

            // update "Last value"
            pdhStatus = PdhGetFormattedCounterValue(pCib->hCounter,
                                                    HFC_PERFORMANCE_FMT_DOUBLE,
                                                    &dwType,
                                                    &pValue);

            pCib->dLastValue = pValue.doubleValue;

            // update "Raw Value" and statistics
            pdhStatus = PdhGetRawCounterValue(pCib->hCounter, &dwType, &pRaw);
            pCib->pCounterArray[pCib->dwNextIndex] = pRaw;

            pdhStatus = PdhComputeCounterStatistics(pCib->hCounter,
                                                    HFC_PERFORMANCE_FMT_DOUBLE,
                                                    pCib->dwFirstIndex,
                                                    ++pCib->dwLastIndex,
                                                    pCib->pCounterArray,
                                                    &pCib->pdhCurrentStats);

            // update pointers & indeces
            if( pCib->dwLastIndex < NUM_STAT_SAMPLES )
                {
                pCib->dwNextIndex = ++pCib->dwNextIndex % NUM_STAT_SAMPLES;
                }
            else
                {
                --pCib->dwLastIndex;
                pCib->dwNextIndex  = pCib->dwFirstIndex;
                pCib->dwFirstIndex = ++pCib->dwFirstIndex % NUM_STAT_SAMPLES;
                }
            }
        }

    return pdhStatus;
    }

//-----------------------------------------------------------------
// char* EnumObjects
//
// Enumerate all the object on the machine. The return is a string
// with multiple elements separated by /0.
//-----------------------------------------------------------------
void HFCPerformanceDataHelper::EnumObjects()
    {
    DWORD  BufLen  = 0;
    WCharP pBuffer = 0;
    WCharP pObject;

    // Clear the list
    m_ObjectList.erase(m_ObjectList.begin(), m_ObjectList.end());

    // Get the len of the buffer
    if( PdhEnumObjects(0, 0, 0, &BufLen, PERF_DETAIL_WIZARD, false) == ERROR_SUCCESS )
        {
        pBuffer = new WChar[BufLen];
        if( PdhEnumObjects(0, 0, pBuffer, &BufLen, PERF_DETAIL_WIZARD, false) == ERROR_SUCCESS )
            {
            BufLen  = 0;
            pObject = pBuffer;
            while( wcscmp(pObject, L"") != 0 )
                {
                m_ObjectList.push_back(pObject);
                BufLen += (DWORD)wcslen(pObject) + 1;
                pObject = pBuffer + BufLen;
                }
            }
        }

    delete pBuffer;
    pBuffer = 0;
    }

//-----------------------------------------------------------------
// const PERFORMANCE_OBJECT_LIST& GetObjectList
//
// Return the list of available object on the current machine.
//-----------------------------------------------------------------
const HFCPerformanceDataHelper::PERFORMANCE_OBJECT_LIST& HFCPerformanceDataHelper::GetObjectList() const
    {
    return m_ObjectList;
    }

//-----------------------------------------------------------------
// HFCPerformanceDataHelper::OID EnumObjectItems
//
// Enumerate items of an object.
//
// const string& pi_rObjectName: The object to enumerate.
// HFCPerformanceDataHelper::POID pi_pRetStc: A pointer to the
//                                            return struct.
//-----------------------------------------------------------------
HFC_PERFORMANCE_STATUS  HFCPerformanceDataHelper::EnumObjectItems(const WString& pi_rObjectName,
                                                                  HFCPerformanceDataHelper::POID pi_pRetStc)
    {
    HFC_PERFORMANCE_STATUS pdhStatus;
    DWORD  CounterListLen  = 0;
    DWORD  InstanceListLen = 0;
    WCharP pCounterList    = 0;
    WCharP pInstanceList   = 0;

    HPRECONDITION(pi_pRetStc != 0);

    // Get the len of the buffer
    pdhStatus = PdhEnumObjectItems(0,
                                   0,
                                   pi_rObjectName.c_str(),
                                   0,
                                   &CounterListLen,
                                   0,
                                   &InstanceListLen,
                                   PERF_DETAIL_WIZARD,
                                   0);

    if( pdhStatus == HFC_PERFORMANCE_ERROR_SUCCESS )
        {
        // Allocate memory
        if( CounterListLen > 0 )
            pCounterList  = new WChar[CounterListLen];

        if( InstanceListLen > 0 )
            pInstanceList = new WChar[InstanceListLen];

        // Get objects
        pdhStatus = PdhEnumObjectItems(0,
                                       0,
                                       pi_rObjectName.c_str(),
                                       pCounterList,
                                       &CounterListLen,
                                       pInstanceList,
                                       &InstanceListLen,
                                       PERF_DETAIL_WIZARD,
                                       0);

        if( pdhStatus != HFC_PERFORMANCE_ERROR_SUCCESS )
            {
            delete pCounterList;
            delete pInstanceList;
            pCounterList  = 0;
            pInstanceList = 0;
            }
        else
            {
            pi_pRetStc->pCounterList  = pCounterList;
            pi_pRetStc->pInstanceList = pInstanceList;
            }
        }

    return pdhStatus;
    }

//-----------------------------------------------------------------
// bool GetValue
//
// Retreive the value of a counter.
//
// double& pi_rValue: A reference that will contain the value.
// HFC_PERFORMANCE_COUNTER pi_hCounter: The counter handle to query.
//
//-----------------------------------------------------------------
bool HFCPerformanceDataHelper::GetValue(double& pi_rValue,
                                         HFC_PERFORMANCE_COUNTER pi_hCounter)
    {
    bool Found = false;
    PERFORMANCE_LIST_ITR Itr;

    Itr = m_CounterList.begin();

    while( !Found && Itr != m_CounterList.end() )
        {
        if( (*Itr)->hCounter == pi_hCounter )
            {
            pi_rValue = (*Itr)->dLastValue;
            Found = true;
            }
        else
            Itr++;
        }

    return Found;
    }


//-----------------------------------------------------------------
// HFC_PERFORMANCE_STATUS DeleteCounter
//
// Remove a counter from the query.
//
// HFC_PERFORMANCE_COUNTER* pi_pCtrHandle: The counter handle to
//                                         remove.
//-----------------------------------------------------------------
HFC_PERFORMANCE_STATUS HFCPerformanceDataHelper::DeleteCounter(HFC_PERFORMANCE_COUNTER* pi_pCtrHandle)
    {
    PCIB  pCib;
    bool Found = false;
    PERFORMANCE_LIST_ITR Itr;
    HFC_PERFORMANCE_STATUS pdhStatus = HFC_PERFORMANCE_NO_COUNTER;

    Itr = m_CounterList.begin();

    while( !Found && Itr != m_CounterList.end() )
        {
        pCib = *Itr;

        if( pCib->hCounter == pi_pCtrHandle )
            Found = true;
        else
            Itr++;
        }

    if( Found )
        {
        if( pCib->pCounterArray != 0 )
            delete pCib->pCounterArray;

        delete pCib;
        }

    return pdhStatus;
    }

//-----------------------------------------------------------------
// void DeleteAllCounters
//
// Remove all counter from the query.
//-----------------------------------------------------------------
void HFCPerformanceDataHelper::DeleteAllCounters()
    {
    PCIB pCib;

    while( m_CounterList.size() > 0 )
        {
        pCib = m_CounterList.front();
        if( pCib->pCounterArray != 0 )
            delete pCib->pCounterArray;

        delete pCib;
        m_CounterList.pop_front();
        }
    }
