//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurface.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> inline method for HGSSurface
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Add a new option.

 @param pi_rpOption
-----------------------------------------------------------------------------*/
inline void HGSSurface::AddOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption)
    {
    m_pImplementation->AddOption(pi_rpOption);
    }

/**----------------------------------------------------------------------------
 Remove option.

 @param pi_OptionID
-----------------------------------------------------------------------------*/
inline void HGSSurface::RemoveOption(HCLASS_ID  pi_OptionID)
    {
    m_pImplementation->RemoveOption(pi_OptionID);
    }

/**----------------------------------------------------------------------------
 Get a specific option.

 @param pi_OptionID

 @return const HFCPtr<HGSSurfaceOption>&
-----------------------------------------------------------------------------*/
inline const HFCPtr<HGSSurfaceOption> HGSSurface::GetOption(HCLASS_ID  pi_OptionID) const
    {
    return m_pImplementation->GetOption(pi_OptionID);
    }

/**----------------------------------------------------------------------------
 Set a specific option.

 If the option already exist into the surface, the option will be
 replaced, otherwise, the option will be added.


 @param pi_rpOption
-----------------------------------------------------------------------------*/
inline void HGSSurface::SetOption(const HFCPtr<HGSSurfaceOption>& pi_rpOption)
    {
    m_pImplementation->SetOption(pi_rpOption);
    }

/**----------------------------------------------------------------------------
 Get options.

 @return const HGSSurfaceOptions&
-----------------------------------------------------------------------------*/
inline const HGSSurfaceOptions& HGSSurface::GetOptions() const
    {
    m_pImplementation->GetOptions();
    }

/**----------------------------------------------------------------------------
 Get the surface descriptor.

 @return const HFCPtr<HGSSurfaceDescriptor>&
-----------------------------------------------------------------------------*/
inline const HFCPtr<HGSSurfaceDescriptor>& HGSSurface::GetSurfaceDescriptor() const
    {
    HPRECONDITION(m_pImplementation != 0);

    return m_pImplementation->GetSurfaceDescriptor();
    }




//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------
