//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSDirectoryLister.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFSDirectoryLister
//-----------------------------------------------------------------------------

#pragma once


//############################
// INCLUDE FILES
//############################


// HFC
#include "HFCConnection.h"
#include "HFCFileAttributes.h"

// HFS
#include "HFSDirectoryListItem.h"

BEGIN_IMAGEPP_NAMESPACE

class HFSDirectoryLister : public HFCShareableObject<HFSDirectoryLister>
    {
public:

    HDECLARE_BASECLASS_ID(HFSDirectoryListerId_Base);

    // STL definition
    typedef list<HFSDirectoryListItem, allocator<HFSDirectoryListItem> > HFSENTRY_LIST;
    typedef HFSENTRY_LIST::iterator HFSENTRY_LIST_ITR;

    typedef list<WString, allocator<WString> > HFSPATTERN_LIST;
    typedef HFSPATTERN_LIST::iterator HFSPATTERN_LIST_ITR;

    // Construction - Destruction
    HFSDirectoryLister(HFCPtr<HFCConnection>& pi_pConnection);
    virtual
    ~HFSDirectoryLister();

    // Operations
    virtual bool
    List(const WString& pi_rPath) = 0;

    virtual const HFSDirectoryListItem&
    GetEntry() const;

    virtual uint32_t
    GetCount() const;

    virtual bool
    GotoFirst();

    virtual bool
    GotoNext();

    virtual void
    SetPattern(const WString& pi_rPattern);

    virtual bool
    IsConnected() const;

    virtual bool
    IsError() const;

    virtual WString
    GetServerURL() const;

    virtual bool
    SupportFileDate() const;

    virtual bool
    SupportFileSize() const;

    const WString&
    GetCurrentPath() const;

    WString
    GetPatternList();

protected:

    virtual void
    UnifyPath(WString& pi_rString);

    virtual void
    PrepareForList();

    virtual bool
    IsEntryCompliant(const WString& pi_rFileName);

    WString
    GetFileExtension(const WString& pi_rFileName);

    WChar*
    GetNextLabel(const WChar* pi_pString, size_t* pi_pPos);


    // Attributes
    HFSENTRY_LIST        m_EntryList;
    HFSENTRY_LIST_ITR    m_EntryListItr;
    HFSPATTERN_LIST     m_PatternList;
    HFCPtr<HFCConnection>  m_pConnection;
    WString             m_CurrentPath;
    bool               m_IsError;

private:



    // Not implemented
    HFSDirectoryLister();
    HFSDirectoryLister(const HFSDirectoryLister&);
    HFSDirectoryLister& operator=(const HFSDirectoryLister&);
    };

END_IMAGEPP_NAMESPACE
#include "HFSDirectoryLister.hpp"


