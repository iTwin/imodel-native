//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------



inline BentleyApi::ImagePP::HCDPacket& Packet::EditPacket ()
    {
    HPRECONDITION(!IsSharedPacket());
    return *m_pDataPacket;
    }

inline const BentleyApi::ImagePP::HCDPacket& Packet::GetPacket () const
    {
    return *m_pDataPacket;
    }

inline Packet::iterator Packet::BeginEdit ()
    {
    return Edit();
    }

inline Packet::iterator Packet::EndEdit ()
    {
    return Edit() + GetSize();
    }

inline Packet::iterator Packet::Erase  (const_iterator  pi_Position,
                                        size_t          pi_Size)
    {
    return Erase(pi_Position, pi_Position + pi_Size);
    }

inline void swap   (Packet& pio_rLeft,
                    Packet& pio_rRight)
    {
    pio_rLeft.Swap(pio_rRight);
    }



