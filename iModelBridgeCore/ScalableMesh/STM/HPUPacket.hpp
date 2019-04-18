//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------



HCDPacket& Packet::EditPacket ()
    {
    HPRECONDITION(!IsSharedPacket());
    return *m_pDataPacket;
    }

const HCDPacket& Packet::GetPacket () const
    {
    return *m_pDataPacket;
    }

Packet::iterator Packet::BeginEdit ()
    {
    return Edit();
    }

Packet::iterator Packet::EndEdit ()
    {
    return Edit() + GetSize();
    }

Packet::iterator Packet::Erase  (const_iterator  pi_Position,
                                        size_t          pi_Size)
    {
    return Erase(pi_Position, pi_Position + pi_Size);
    }

void swap   (Packet& pio_rLeft,
                    Packet& pio_rRight)
    {
    pio_rLeft.Swap(pio_rRight);
    }



