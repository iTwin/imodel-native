//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMTypes.hpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

template <typename T>
typename IDTrait<T>::value_type IDTrait<T>::GetNullID ()
    {
    return (std::numeric_limits<typename IDTrait<T>::value_type>::max) ();
    }

inline TileID       GetNullTileID               () {
    return IDTrait<TileID>::GetNullID();
    }
inline NodeID       GetNullNodeID               () {
    return IDTrait<NodeID>::GetNullID();
    }
inline PacketID     GetNullPacketID             () {
    return IDTrait<PacketID>::GetNullID();
    }