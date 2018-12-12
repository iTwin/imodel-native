//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFPacketIdIter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/h/HIterators.h>

namespace HTGFF {

class Directory;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketIdIter : public BentleyApi::ImagePP::BidirectionalIterator<PacketIdIter, uint32_t>
    {
public:
     explicit         PacketIdIter                   ();

    // Hide base class version in order to return by value instead than by reference
    value_type              operator*                      ()                   {
        return Dereference();
        }
    value_type              operator*                      () const             {
        return Dereference();
        }

    friend class            TagFile;

    typedef value_type      PacketID;

    value_type              Dereference                    () const;

     void             Increment                      ();
     void             Decrement                      ();

    bool                    EqualTo                        (const iterator_t&       pi_rRight) const;

    explicit                PacketIdIter                   (const uint64_t*          pi_pOffset,
                                                            const uint64_t*          pi_pOffsetBegin,
                                                            const uint64_t*          pi_pOffsetEnd,
                                                            bool                    pi_forward);

    const uint64_t*          m_pOffset;
    const uint64_t*          m_pOffsetBegin;
    const uint64_t*          m_pOffsetEnd;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline PacketIdIter::value_type PacketIdIter::Dereference () const
    {
    HPRECONDITION(0 != m_pOffsetBegin);
    return static_cast<value_type>(std::distance(m_pOffsetBegin, m_pOffset));
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketIdIter::EqualTo (const iterator_t& pi_rRight) const
    {
    return m_pOffset == pi_rRight.m_pOffset;
    }


} //End namespace HTGFF