//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStat.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCStat
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStat::HFCStat(const HFCURL& pi_rURL)
    {
    // Copy the URL
    m_pURL = HFCURL::Instanciate(pi_rURL.GetURL());

    // find the implementation for this URL
    if (m_pURL != 0)
        m_pImpl = FindImplementation(*m_pURL);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStat::HFCStat(const HFCPtr<HFCURL>& pi_rpURL)
    {
    HPRECONDITION(pi_rpURL != 0);

    // Copy the URL
    m_pURL = pi_rpURL;

    // find the implementation for this URL
    m_pImpl = FindImplementation(*m_pURL);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStat::~HFCStat()
    {
    }



//-----------------------------------------------------------------------------
// Public
// Get Resource Creation time
//-----------------------------------------------------------------------------
inline time_t    HFCStat::GetCreationTime() const
    {
    if (m_pImpl != 0)
        return m_pImpl->GetCreationTime(*m_pURL);
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Public
// Get Resource LastAccess time
//-----------------------------------------------------------------------------
inline time_t    HFCStat::GetLastAccessTime() const
    {
    if (m_pImpl != 0)
        return m_pImpl->GetLastAccessTime(*m_pURL);
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Public
// Get Resource modification time
//-----------------------------------------------------------------------------
inline time_t    HFCStat::GetModificationTime() const
    {
    if (m_pImpl != 0)
        return m_pImpl->GetModificationTime(*m_pURL);
    else
        return 0;
    }


//-----------------------------------------------------------------------------
// Public
// Set the resource modification time
//-----------------------------------------------------------------------------
inline void HFCStat::SetModificationTime(time_t    pi_NewTime) const
    {
    if (m_pImpl != 0)
        m_pImpl->SetModificationTime(*m_pURL, pi_NewTime);
    }

//-----------------------------------------------------------------------------
// Public
// Get the resource size
//-----------------------------------------------------------------------------
inline uint64_t HFCStat::GetSize() const
    {
    if (m_pImpl != 0)
        return m_pImpl->GetSize(*m_pURL);
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Public
// Resource existence
//-----------------------------------------------------------------------------
inline bool HFCStat::IsExistent() const
    {
    if (m_pImpl != 0)
        return m_pImpl->IsExistent(*m_pURL);
    else
        return false;
    }

//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
inline HFCStat::AccessStatus HFCStat::DetectAccess() const
    {
    if (m_pImpl != 0)
        return m_pImpl->DetectAccess(*m_pURL);
    else
        return HFCStat::AccessError;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
inline HFCAccessMode HFCStat::GetAccessMode() const
    {
    if (m_pImpl != 0)
        return m_pImpl->GetAccessMode(*m_pURL);
    else
        return HFC_NO_ACCESS;
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
inline const HFCStatImpl*
HFCStat::FindImplementation(const HFCURL& pi_rURL) const
    {
    const HFCStatImpl* pResult = 0;

    if (s_pImplList != 0)
        {
        for (ImplList::const_iterator Itr = s_pImplList->begin();
             (pResult == 0) && (Itr != s_pImplList->end());
             Itr++)
            {
            if ((*Itr)->CanHandle(pi_rURL))
                pResult = (*Itr);
            }
        }

    return (pResult);
    }


//-----------------------------------------------------------------------------
// public
//
//-----------------------------------------------------------------------------
inline HFCStatImpl::HFCStatImpl()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStatImpl::~HFCStatImpl()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
inline bool HFCStatImpl::IsExistent(const HFCURL& pi_rURL) const
    {
    // Default implementation uses DetectAccess.
    HFCStat::AccessStatus Status = DetectAccess(pi_rURL);
    if(Status == HFCStat::AccessGranted || Status == HFCStat::AccessDenied)
        return true;

    return false;
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
inline void HFCStatImpl::RegisterImpl(const HFCStatImpl* pi_pImpl) const
    {
    HPRECONDITION(pi_pImpl != 0);

    if (HFCStat::s_pImplList == 0)
        HFCStat::s_pImplList = new HFCStat::ImplList;

    HFCStat::s_pImplList->push_back(pi_pImpl);
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
inline void HFCStatImpl::UnregisterImpl(const HFCStatImpl* pi_pImpl) const
    {
    if (HFCStat::s_pImplList == 0)
        return;

    for (HFCStat::ImplList::iterator Itr = HFCStat::s_pImplList->begin(); (Itr != HFCStat::s_pImplList->end()); Itr++)
        {
        if ((*Itr) == pi_pImpl)
            {
            HFCStat::s_pImplList->erase(Itr);
            return;
            }
        }
    }
END_IMAGEPP_NAMESPACE