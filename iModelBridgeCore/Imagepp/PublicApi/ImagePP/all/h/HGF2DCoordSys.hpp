//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DCoordSys.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
inline HGF2DCoordSys::HGF2DCoordSys()
    {
    m_pCoordSysImpl = new HGF2DCoordSysImpl();
    }

inline HGF2DCoordSys::HGF2DCoordSys(const HGF2DTransfoModel&        pi_rModelToRef,
                                    const HFCPtr<HGF2DCoordSys>&    pi_rpRefSys)
    {
    m_pRefCoordSys = pi_rpRefSys;
    m_pCoordSysImpl = new HGF2DCoordSysImpl(pi_rModelToRef,
                                            pi_rpRefSys->m_pCoordSysImpl);
    }

inline HGF2DCoordSys::HGF2DCoordSys(const HGF2DCoordSys& pi_rObj)
    {
    m_pRefCoordSys = pi_rObj.m_pRefCoordSys;
    m_pCoordSysImpl = new HGF2DCoordSysImpl(*pi_rObj.m_pCoordSysImpl);
    }

inline HGF2DCoordSys::~HGF2DCoordSys()
    {
    m_pCoordSysImpl->IncrementRef();
    m_pCoordSysImpl->RemoveReferences();
    m_pCoordSysImpl->DecrementRef();
    m_pCoordSysImpl = 0;
    m_pRefCoordSys = 0;
    }

// Conversion interface

inline void HGF2DCoordSys::ConvertFrom(const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                       double                        pi_XIn,
                                       double                        pi_YIn,
                                       double*                       po_pNewX,
                                       double*                       po_pNewY) const
    {
    m_pCoordSysImpl->ConvertFrom(pi_rpCoordSys->m_pCoordSysImpl,
                                 pi_XIn,
                                 pi_YIn,
                                 po_pNewX,
                                 po_pNewY);
    }

inline void HGF2DCoordSys::ConvertFrom (const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                        double*                       pio_pX,
                                        double*                       pio_pY) const
    {
    m_pCoordSysImpl->ConvertFrom(pi_rpCoordSys->m_pCoordSysImpl,
                                 pio_pX,
                                 pio_pY);
    }

inline void HGF2DCoordSys::ConvertTo(const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                     double                        pi_XIn,
                                     double                        pi_YIn,
                                     double*                       po_pNewX,
                                     double*                       po_pNewY) const
    {
    m_pCoordSysImpl->ConvertTo(pi_rpCoordSys->m_pCoordSysImpl,
                               pi_XIn,
                               pi_YIn,
                               po_pNewX,
                               po_pNewY);
    }


inline void HGF2DCoordSys::ConvertTo(const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                     double*                       pio_pX,
                                     double*                       pio_pY) const
    {
    m_pCoordSysImpl->ConvertTo(pi_rpCoordSys->m_pCoordSysImpl,
                               pio_pX,
                               pio_pY);
    }


inline HFCPtr<HGF2DTransfoModel> HGF2DCoordSys::GetTransfoModelTo(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys) const
    {
    return m_pCoordSysImpl->GetTransfoModelTo(pi_pCoordSys->m_pCoordSysImpl);
    }

inline void HGF2DCoordSys::SetReference(const HGF2DTransfoModel&      pi_rModel,
                                        const HFCPtr<HGF2DCoordSys>&  pi_rpRefSys)
    {
    m_pRefCoordSys = pi_rpRefSys;
    m_pCoordSysImpl->SetReference(pi_rModel,
                                  pi_rpRefSys->m_pCoordSysImpl);
    }

inline const HFCPtr<HGF2DCoordSys>& HGF2DCoordSys::GetReference() const
    {
    return m_pRefCoordSys;
    }

inline bool HGF2DCoordSys::IsUsedAsReference() const
    {
    return m_pCoordSysImpl->IsUsedAsReference();
    }

// Geometric property relation flags
inline bool HGF2DCoordSys::HasLinearityPreservingRelationTo(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return m_pCoordSysImpl->HasLinearityPreservingRelationTo(pi_rpCoordSys->m_pCoordSysImpl);
    }

inline bool HGF2DCoordSys::HasShapePreservingRelationTo(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return m_pCoordSysImpl->HasShapePreservingRelationTo(pi_rpCoordSys->m_pCoordSysImpl);
    }

inline bool HGF2DCoordSys::HasDirectionPreservingRelationTo(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return m_pCoordSysImpl->HasDirectionPreservingRelationTo(pi_rpCoordSys->m_pCoordSysImpl);
    }

inline bool HGF2DCoordSys::HasParallelismPreservingRelationTo(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return m_pCoordSysImpl->HasParallelismPreservingRelationTo(pi_rpCoordSys->m_pCoordSysImpl);
    }

inline bool HGF2DCoordSys::HasStretchRelationTo(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return m_pCoordSysImpl->HasStretchRelationTo(pi_rpCoordSys->m_pCoordSysImpl);
    }




#ifdef __HMR_DEBUG

inline size_t HGF2DCoordSys::GetBranchCount() const
    {
    return m_pCoordSysImpl->GetBranchCount();
    }

inline size_t HGF2DCoordSys::GetCoordSysCount() const
    {
    return m_pCoordSysImpl->GetCoordSysCount();
    }

#endif

/** -----------------------------------------------------------------------------
    This method returns the reference coordinate system for a coordinate system.
    It returns a null pointer if the coordinate system has no reference system.

    @return A smart pointer to the reference coordinate system.

    @code
        HFCPtr<HGF2DCoordSys>    pImageASys(new HGF2DCoordSys ());

        HGF2DIdentity Model ());
        HFCPtr<HGF2DCoordSys>    pOtherSys (new HGF2DCoordSys (&Model,
                                                       pImageASys));

        HFCPtr<HGF2DcoordSys> pReference (pOtherSys->GetReference());
    @end

    @see SetReference()
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DCoordSysImpl>& HGF2DCoordSysImpl::GetReference() const
    {
    return m_pRefCoordSys;
    }

//-----------------------------------------------------------------------------
// IsUsedAsReference
// Indicates if this world is used as a reference by any coordinate system
//-----------------------------------------------------------------------------
inline bool HGF2DCoordSysImpl::IsUsedAsReference() const
    {
    return(!m_ListIsRefTo.empty());
    }

END_IMAGEPP_NAMESPACE