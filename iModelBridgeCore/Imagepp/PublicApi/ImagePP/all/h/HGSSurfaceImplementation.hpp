//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceImplementation.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline method for HGSSurfaceImplementation
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 AddOption

 @param pi_rpOption
-----------------------------------------------------------------------------*/
inline void HGSSurfaceImplementation::AddOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption)
    {
    HPRECONDITION(pi_rpOption != 0);

    bool Result = m_Options.insert(HGSSurfaceOptions::value_type(pi_rpOption->GetClassID(), pi_rpOption)).second;

    HASSERT(Result);
    }

/**----------------------------------------------------------------------------
 RemoveOption

 @param pi_OptionID
-----------------------------------------------------------------------------*/
inline void HGSSurfaceImplementation::RemoveOption(HCLASS_ID pi_OptionID)
    {
    HGSSurfaceOptions::iterator Itr(m_Options.find(pi_OptionID)) ;
    if (Itr != m_Options.end())
        m_Options.erase(Itr);
    }

/**----------------------------------------------------------------------------
 GetOption

 @param pi_OptionID

 @return const HFCPtr<HGSSurfaceOption>
-----------------------------------------------------------------------------*/
inline const HFCPtr<HGSSurfaceOption> HGSSurfaceImplementation::GetOption(HCLASS_ID  pi_OptionID) const
    {
    HGSSurfaceOptions::const_iterator Itr(m_Options.find(pi_OptionID));

    if (Itr == m_Options.end())
        return 0;
    else
        return (*Itr).second;
    }

/**----------------------------------------------------------------------------
 SetOption

 If the option already exist, the option will be replaced, otherwise the
 option will be added

 @param pi_rpOption
-----------------------------------------------------------------------------*/
inline void HGSSurfaceImplementation::SetOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption)
    {
    HPRECONDITION(pi_rpOption != 0);

    HGSSurfaceOptions::iterator Itr(m_Options.find(pi_rpOption->GetClassID()));

    if (Itr == m_Options.end())
        {
        bool Result = m_Options.insert(HGSSurfaceOptions::value_type(pi_rpOption->GetClassID(),
                                                                      pi_rpOption)).second;
        HASSERT(Result);
        }
    else
        {
        (*Itr).second = pi_rpOption;
        }
    }

/**----------------------------------------------------------------------------
 GetOptions

 @return const HGSSurfaceOptions&
-----------------------------------------------------------------------------*/
inline const HGSSurfaceOptions& HGSSurfaceImplementation::GetOptions() const
    {
    return m_Options;
    }

/**----------------------------------------------------------------------------
 GetSurfaceDescriptor

 @return const HFCPtr<HGSSurfaceDescriptor>&
-----------------------------------------------------------------------------*/
inline const HFCPtr<HGSSurfaceDescriptor>& HGSSurfaceImplementation::GetSurfaceDescriptor() const
    {
    return m_pSurfaceDescriptor;
    }

/**----------------------------------------------------------------------------
 HasPhysicalPalette

 @return bool
-----------------------------------------------------------------------------*/
inline bool HGSSurfaceImplementation::HasPhysicalPalette() const
    {
    // default
    return false;
    }

/**----------------------------------------------------------------------------
 SetPhysicalPalette

 @param pi_rPalette
-----------------------------------------------------------------------------*/
inline void HGSSurfaceImplementation::SetPhysicalPalette(const HRPPixelPalette& pi_rpPalette)
    {
    // default implementation, do nothing
    HASSERT(0);
    }


//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 SetSurfaceDescriptor

 @param pi_rpDescriptor The descriptor of the surface implementation.
-----------------------------------------------------------------------------*/
inline void HGSSurfaceImplementation::SetSurfaceDescriptor(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor)
    {
    HPRECONDITION(pi_rpDescriptor != 0);

    m_pSurfaceDescriptor = pi_rpDescriptor;
    }
