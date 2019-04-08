//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <limits>

BEGIN_IMAGEPP_NAMESPACE
struct HFCAccessMode;
END_IMAGEPP_NAMESPACE

namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* Generic types section
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @description  Class for stocking an IDTM id.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class IDTrait
    {
public:
    typedef T                           value_type;
    static value_type                   GetNullID                      ()
        {
        return (std::numeric_limits<typename IDTrait<T>::value_type>::max) ();
        }
    };


typedef BentleyApi::ImagePP::HFCAccessMode   AccessMode; // Hide ImagePP's access mode class


typedef uint32_t                         AttributeID;
typedef uint32_t                       PacketID;


/*---------------------------------------------------------------------------------**//**
* Tile Directory interface types section
+---------------+---------------+---------------+---------------+---------------+------*/

typedef PacketID                        TileID;
typedef PacketID                        NodeID;

typedef uint32_t                       ResolutionID;
typedef uint32_t                       GroupID;



inline TileID     GetNullTileID     () {return IDTrait<TileID>::GetNullID();}
inline NodeID     GetNullNodeID     () {return IDTrait<NodeID>::GetNullID();}
inline PacketID   GetNullPacketID   () {return IDTrait<PacketID>::GetNullID();}

#ifdef _WIN32
#pragma pack(push, IDTMFileIdent, 4)
#else
#pragma pack(push, 4)
#endif


//typedef double                              PointDimension64f;

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 2D extent class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Extent2d64f
    {
     bool                             operator==              (const Extent2d64f&        pi_rRight) const;

    double                                  xMin, xMax, yMin, yMax;
    };

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D extent class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Extent3d64f
    {
     bool                             operator==              (const Extent3d64f&        pi_rRight) const;

    double                                  xMin, xMax, yMin, yMax, zMin, zMax;
    };



#ifdef _WIN32
#pragma pack(pop, IDTMFileIdent)
#else
#pragma pack(pop)
#endif

} //End namespace IDTMFile
