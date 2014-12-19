//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFSHIB.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>---------------------------------------------------------------------------
//:> inline methods for class HFSHIB
//:>---------------------------------------------------------------------------

//:>---------------------------------------------------------------------------
//:> public section
//:>---------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Get the connection.

 @return const HFCPtr<HFCConnection>& The connection.
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFCConnection>& HFSHIB::GetConnection() const
    {
    return m_pConnection;
    }

/**----------------------------------------------------------------------------
 Get the interesting attributes.

 @return const HFCPtr<HPMAttributeSet>& The interesting attributes
-----------------------------------------------------------------------------*/
inline const HFCPtr<HPMAttributeSet>& HFSHIB::GetInterestingAttributes() const
    {
    return m_pInterestingAttributes;
    }


/**----------------------------------------------------------------------------
 Return the path of the current folder.

 @return WString The path of the current folder
-----------------------------------------------------------------------------*/
inline WString HFSHIB::GetCurrentPath() const
    {
    return m_pCurrentFolder->GetPath();
    }

/**----------------------------------------------------------------------------
 Return the current folder.

 @return const HFCPtr<HFSItem>& The current folder
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSHIB::GetCurrentFolder() const
    {
    return (const HFCPtr<HFSItem>&)m_pCurrentFolder;
    }

/**----------------------------------------------------------------------------
 Change the current folder.

 @param pi_rPath    The relative path from the current folder.

 @return const HFCPtr<HFSItem>& The new current folder, 0 if the path is
                                invalid
-----------------------------------------------------------------------------*/
inline const HFCPtr<HFSItem>& HFSHIB::ChangeFolder(const WString& pi_rPath)
    {
    HPRECONDITION(!pi_rPath.empty());

    m_pResult = (const HFCPtr<HFSHIBPItem>&)GetItem(pi_rPath);
    if (m_pResult != 0)
        {
        HPOSTCONDITION(m_pResult->IsCompatibleWith(HFSHIBPItem::CLASS_ID));
        HPOSTCONDITION(m_pResult->IsFolder());
        m_pCurrentFolder = (HFCPtr<HFSHIBPItem>&)m_pResult;
        }

    return (const HFCPtr<HFSItem>&)m_pResult;
    }

//:>---------------------------------------------------------------------------
//:> protected section
//:>---------------------------------------------------------------------------



//:>---------------------------------------------------------------------------
//:> private section
//:>---------------------------------------------------------------------------
