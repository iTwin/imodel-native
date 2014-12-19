//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/net/hcs/src/HCSPersistentFileInfoList.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HCSPersistentFileInfoList
//-----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCFileInfo.h>
#include <Imagepp/all/h/HCSPersistentFileInfoList.h>

HPM_REGISTER_CLASS(HCSPersistentFileInfoList, HPMPersistentObject)

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HCSPersistentFileInfoList::HCSPersistentFileInfoList()
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCSPersistentFileInfoList::~HCSPersistentFileInfoList()
    {
    HFCFileInfo::HFCFileInfoList::iterator iteratorHandle;

    // Clear the list
    iteratorHandle = m_FileList.begin();
    while( iteratorHandle != m_FileList.end() )
        {
        HFCFileInfo* data = *iteratorHandle;

        m_FileList.remove(*iteratorHandle);
        delete data;
        iteratorHandle = m_FileList.begin();
        }
    }

//-----------------------------------------------------------------------------
// public
// Return internal list
//-----------------------------------------------------------------------------
const HFCFileInfo::HFCFileInfoList& HCSPersistentFileInfoList::GetList()
    {
    return m_FileList;
    }

//-----------------------------------------------------------------------------
// public
// Add an item to the list
//-----------------------------------------------------------------------------
void HCSPersistentFileInfoList::Add(HFCFileInfo* pi_FileInfo)
    {
    HPRECONDITION(pi_FileInfo != 0);
    m_FileList.push_back(pi_FileInfo);
    }

//-----------------------------------------------------------------------------
// public
// Remove an item from the list
//-----------------------------------------------------------------------------
void HCSPersistentFileInfoList::Remove(HFCFileInfo* pi_FileInfo)
    {
    HPRECONDITION(pi_FileInfo != 0);
    m_FileList.remove(pi_FileInfo);
    }
