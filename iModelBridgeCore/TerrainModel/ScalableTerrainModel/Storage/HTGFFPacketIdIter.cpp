//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFPacketIdIter.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFPacketIdIter.h>

#include <STMInternal/Storage/HTGFFDirectory.h>

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter::PacketIdIter ()
    :   m_pOffset(0),
        m_pOffsetBegin(0),
        m_pOffsetEnd(0)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter::PacketIdIter (const uint64_t*  pi_pOffset,
                            const uint64_t*  pi_pOffsetBegin,
                            const uint64_t*  pi_pOffsetEnd,
                            bool            pi_forward)
    :   m_pOffset(pi_pOffset),
        m_pOffsetBegin(pi_pOffsetBegin),
        m_pOffsetEnd(pi_pOffsetEnd)
    {
    // Ensure that we point on a valid packet
    if (pi_forward)
        {
        if ((m_pOffset < m_pOffsetEnd) && (0 == *m_pOffset))
            Increment();
        }
    else
        {
        if ((m_pOffset >= m_pOffsetBegin) &&
            ((m_pOffset == m_pOffsetEnd) || (0 == *m_pOffset)))
            Decrement();
        }

    HPRECONDITION(m_pOffsetBegin <= m_pOffset);
    HPRECONDITION(m_pOffsetEnd >= m_pOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketIdIter::Increment ()
    {
    HPRECONDITION(m_pOffsetEnd != m_pOffset);

    while ((m_pOffset < m_pOffsetEnd) && (0 == *(++m_pOffset))) {};
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketIdIter::Decrement ()
    {
    HPRECONDITION(m_pOffsetBegin != m_pOffset);

    while (m_pOffset > m_pOffsetBegin && (0 == *(--m_pOffset))) {};
    }


} //End namespace HTGFF