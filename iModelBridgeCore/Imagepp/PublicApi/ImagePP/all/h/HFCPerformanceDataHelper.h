//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCPerformanceDataHelper.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HFCPerformanceDataHelper
//-----------------------------------------------------------------------------

#pragma once

// WIN32 conditional inclusion
#if defined(_WIN32) || defined(WIN32)


//###############################
// INCLUDE FILES
//###############################

#include <pdh.h>
#include <winperf.h>
#include <pdhmsg.h>

#define HFC_PERFORMANCE_COUNTER                    HCOUNTER
#define HFC_PERFORMANCE_QUERY                      HQUERY
#define HFC_PERFORMANCE_STATUS                     PDH_STATUS

#define HFC_PERFORMANCE_PRAW_COUNTER               PPDH_RAW_COUNTER
#define HFC_PERFORMANCE_RAW_COUNTER                PDH_RAW_COUNTER
#define HFC_PERFORMANCE_STATISTICS                 PDH_STATISTICS

#define HFC_PERFORMANCE_FMT_COUNTERVALUE           PDH_FMT_COUNTERVALUE
#define HFC_PERFORMANCE_FMT_DOUBLE                 PDH_FMT_DOUBLE

#define HFC_PERFORMANCE_ERROR_SUCCESS              ERROR_SUCCESS
#define HFC_PERFORMANCE_BAD_COUNTERNAME            PDH_CSTATUS_BAD_COUNTERNAME
#define HFC_PERFORMANCE_NO_COUNTER                 PDH_CSTATUS_NO_COUNTER
#define HFC_PERFORMANCE_NO_COUNTERNAME             PDH_CSTATUS_NO_COUNTERNAME
#define HFC_PERFORMANCE_NO_MACHINE                 PDH_CSTATUS_NO_MACHINE
#define HFC_PERFORMANCE_NO_OBJECT                  PDH_CSTATUS_NO_OBJECT
#define HFC_PERFORMANCE_NOT_FOUND                  PDH_FUNCTION_NOT_FOUND
#define HFC_PERFORMANCE_INVALID_ARGUMENT           PDH_INVALID_ARGUMENT
#define HFC_PERFORMANCE_INVALID_HANDLE             PDH_INVALID_HANDLE
#define HFC_PERFORMANCE_MEMORY_ALLOCATION_FAILURE  PDH_MEMORY_ALLOCATION_FAILURE
#define HFC_PERFORMANCE_CSTATUS_INVALID_DATA       PDH_CSTATUS_INVALID_DATA
#define HFC_PERFORMANCE_NO_DATA                    PDH_NO_DATA

#endif

BEGIN_IMAGEPP_NAMESPACE

const int NUM_STAT_SAMPLES = 100;

class HFCPerformanceDataHelper
    {
public:

    // ObjectsItems struct
    typedef struct _ObjectsItemsData
        {
        WChar* pCounterList;
        WChar* pInstanceList;

        _ObjectsItemsData::_ObjectsItemsData()
            {
            pCounterList  = 0;
            pInstanceList = 0;
            }
        } OID, *POID;

    typedef list< WString, allocator<WString> > PERFORMANCE_OBJECT_LIST;
    typedef PERFORMANCE_OBJECT_LIST::iterator PERFORMANCE_OBJECT_LIST_ITR;

    // Construction - Destruction
    HFCPerformanceDataHelper();
    virtual ~HFCPerformanceDataHelper();

    // Access methods
    HFC_PERFORMANCE_STATUS
    Open();

    void     Close();


    HFC_PERFORMANCE_STATUS
    AddCounter(const WString& pi_rCounterPath,
               HFC_PERFORMANCE_COUNTER* pi_pCtrHandle);

    HFC_PERFORMANCE_STATUS
    DeleteCounter(HFC_PERFORMANCE_COUNTER* pi_pCtrHandle);

    HFC_PERFORMANCE_STATUS
    QueryData();

    bool    GetValue(double& pi_rValue,
                      HFC_PERFORMANCE_COUNTER pi_hCounter);

    const HFCPerformanceDataHelper::PERFORMANCE_OBJECT_LIST&
    GetObjectList() const;

    HFC_PERFORMANCE_STATUS
    EnumObjectItems(const WString& pi_rObjectName,
                    HFCPerformanceDataHelper::POID pi_pRetStc);

    void     DeleteAllCounters();

protected:

private:

    // Not implemented
    HFCPerformanceDataHelper(const HFCPerformanceDataHelper&);
    HFCPerformanceDataHelper& operator=(const HFCPerformanceDataHelper&);

    // Private methods


    // Enum methods
    void     EnumObjects();


    // Attributes

    // Query handle
    HFC_PERFORMANCE_QUERY m_hQuery;

    typedef struct _CounterInfoBlock
        {
        WChar                          szCounterPath[MAX_PATH];
        HFC_PERFORMANCE_COUNTER         hCounter;
        HFC_PERFORMANCE_PRAW_COUNTER    pCounterArray;
        DWORD                           dwFirstIndex;
        DWORD                           dwNextIndex;
        DWORD                           dwLastIndex;
        HFC_PERFORMANCE_STATISTICS      pdhCurrentStats;
        double                         dLastValue;

        // Needed by STL
        bool operator==(const _CounterInfoBlock& pi_rSrc) const {
            return hCounter == pi_rSrc.hCounter;
            }
        bool operator!=(const _CounterInfoBlock& pi_rSrc) const {
            return hCounter != pi_rSrc.hCounter;
            }
        bool operator<(const _CounterInfoBlock&  pi_rSrc) const {
            return hCounter < pi_rSrc.hCounter;
            }
        bool operator>(const _CounterInfoBlock&  pi_rSrc) const {
            return hCounter > pi_rSrc.hCounter;
            }

        } CIB, *PCIB;

    typedef list< PCIB, allocator<PCIB> > PERFORMANCE_LIST;
    typedef PERFORMANCE_LIST::iterator PERFORMANCE_LIST_ITR;

    PERFORMANCE_LIST m_CounterList;
    PERFORMANCE_OBJECT_LIST m_ObjectList;
    };

END_IMAGEPP_NAMESPACE
