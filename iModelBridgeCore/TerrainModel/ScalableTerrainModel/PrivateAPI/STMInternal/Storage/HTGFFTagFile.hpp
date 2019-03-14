//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFTagFile.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description  Returns whether the current directory is read-only.
* @return       true when read-only, false otherwise.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool TagFile::IsReadOnly () const
    {
    return !GetAccessMode().m_HasCreateAccess && !GetAccessMode().m_HasWriteAccess;
    }

//-----------------------------------------------------------------------------
// Read a packet of data (compress wise).
//-----------------------------------------------------------------------------
inline BentleyApi::ImagePP::HSTATUS TagFile::ReadPacket     (BentleyApi::ImagePP::HCDPacket&  po_rPacket,
                                        PacketID    pi_PacketID) const
    {
    BentleyApi::ImagePP::HFCMonitor Monitor(m_Key);
    HPRECONDITION(0 == po_rPacket.GetCodec() || BentleyApi::ImagePP::HCDCodecIdentity::CLASS_ID == po_rPacket.GetCodec()->GetClassID());
    return !IsDataCompressed() ? ReadRawPacket(po_rPacket, pi_PacketID) : ReadCompressedPacket(po_rPacket, pi_PacketID);
    }

//-----------------------------------------------------------------------------
// Write a packet (compression wise). Will reuse released storage when
// possible if the new packet is of greater size than the former.
//-----------------------------------------------------------------------------
inline BentleyApi::ImagePP::HSTATUS TagFile::WritePacket   (const BentleyApi::ImagePP::HCDPacket&    pi_rPacket,
                                       PacketID            pi_PacketID)
    {
    BentleyApi::ImagePP::HFCMonitor Monitor(m_Key);

    HPRECONDITION(0 == pi_rPacket.GetCodec() || BentleyApi::ImagePP::HCDCodecIdentity::CLASS_ID == pi_rPacket.GetCodec()->GetClassID());
    return !IsDataCompressed() ? WriteRawPacket(pi_rPacket, pi_PacketID)
           : WriteCompressedPacket(pi_rPacket, pi_PacketID);
    }


template <typename T>
void expandCopyBuffer (const T*             pi_pOldBuffer,
                       size_t               pi_OldSize,
                       BentleyApi::ImagePP::HArrayAutoPtr<T>&    pi_rpNewBuffer,
                       size_t               pi_NewSize,
                       const T&             pi_rInitValue)
    {
    HPRECONDITION(pi_NewSize > pi_OldSize);

    // Realloc buffer
    pi_rpNewBuffer = new T[pi_NewSize];
    if (0 == pi_rpNewBuffer)
        {
        HASSERT(!"Was unable to allocate new buffer");
        return;
        }

    // TDORAY: Pretty sure that we can remove the if and use only first case and that this will be more efficient. Test...
    if (0 != pi_OldSize && 0 != pi_pOldBuffer)
        {
        std::fill (std::copy (pi_pOldBuffer, pi_pOldBuffer + pi_OldSize, pi_rpNewBuffer.get()),
                   pi_rpNewBuffer.get() + pi_NewSize,
                   pi_rInitValue);
        }
    else
        {
        std::fill (pi_rpNewBuffer.get(), pi_rpNewBuffer.get() + pi_NewSize, pi_rInitValue);
        }
    }

} //End namespace HTGFF