//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCBinStream.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCFile
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCURL.h>

// Static member initialization

HFCBinStream::StreamTypeList* HFCBinStream::s_pStreamTypeList = 0;

// The destroyer that frees the scheme list.  The creators registered in it are
// not deleted because these are static objects.

static struct BinStreamTypeListDestroyer
    {
    ~BinStreamTypeListDestroyer()
        {
        if (HFCBinStream::s_pStreamTypeList)
            delete HFCBinStream::s_pStreamTypeList;
        }
    } s_BinStreamTypeListDestroyer;


//-----------------------------------------------------------------------------
// The constructor for this class.
//-----------------------------------------------------------------------------
HFCBinStream::HFCBinStream()
    {
    m_LastException = NO_EXCEPTION;
    m_BinStreamOpen = false;
    }

//-----------------------------------------------------------------------------
// The destructor for this class.
//-----------------------------------------------------------------------------
HFCBinStream::~HFCBinStream()
    {
    // Nothing to do!
    }

//-----------------------------------------------------------------------------
// This is a kind of "virtual constructor".  Can be used instead of constructor
// of this class or of any derived class when come the time to get an object
// specialized for a given URL (the scheme type of the URL is used to determine
// the type of the stream).  It creates a correctly-typed object corresponding
// to given URL.  Return zero if the scheme type is unknown.  No URL means
// empty stream type, a supported case.
//-----------------------------------------------------------------------------
HFCBinStream* HFCBinStream::Instanciate(HFCPtr<HFCURL> pi_pURL,
                                        HFCAccessMode  pi_AccessMode,
                                        short pi_NbRetry)
    {
    HFCBinStream* pNewObj = 0;
    WString SchemeType((pi_pURL != 0) ? pi_pURL->GetSchemeType() : WString());
    StreamTypeList::iterator itr = GetStreamTypeList().find(SchemeType);
    if (itr != GetStreamTypeList().end())
        {
        pNewObj = (*itr).second->Create(pi_pURL, pi_AccessMode, pi_NbRetry);
        }

    return pNewObj;
    }

//-----------------------------------------------------------------------------
// Static method that provide access to the stream-type list of block streams.
//-----------------------------------------------------------------------------
HFCBinStream::StreamTypeList& HFCBinStream::GetStreamTypeList()
    {
    if (!s_pStreamTypeList)
        s_pStreamTypeList = new StreamTypeList;
    return *s_pStreamTypeList;
    }

//-----------------------------------------------------------------------------
// Returns the last exception ID, 0 if no exception.
//-----------------------------------------------------------------------------
ExceptionID HFCBinStream::GetLastExceptionID() const
    {
    return m_LastException;
    }

//-----------------------------------------------------------------------------
// Returns true if the BinStream can be used
//-----------------------------------------------------------------------------
bool HFCBinStream::IsOpened() const
    {
    return m_BinStreamOpen;
    }