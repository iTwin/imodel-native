//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMTypes.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <limits>

struct HFCAccessMode;

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
    static value_type                   GetNullID                      ();
    };


typedef HFCAccessMode                   AccessMode; // Hide ImagePP's access mode class


typedef uint32_t                         AttributeID;
typedef uint32_t                       PacketID;


/*---------------------------------------------------------------------------------**//**
* Tile Directory interface types section
+---------------+---------------+---------------+---------------+---------------+------*/

typedef PacketID                        TileID;
typedef PacketID                        NodeID;

typedef uint32_t                       ResolutionID;
typedef uint32_t                       GroupID;



TileID     GetNullTileID     ();
NodeID     GetNullNodeID     ();
PacketID   GetNullPacketID   ();

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
    _HDLLg bool                             operator==              (const Extent2d64f&        pi_rRight) const;

    double                                  xMin, xMax, yMin, yMax;
    };

/*---------------------------------------------------------------------------------**//**
* @description  Native IDTM 3D extent class.
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Extent3d64f
    {
    _HDLLg bool                             operator==              (const Extent3d64f&        pi_rRight) const;

    double                                  xMin, xMax, yMin, yMax, zMin, zMax;
    };


#ifdef _WIN32
#pragma pack(pop, IDTMFileIdent)
#else
#pragma pack(pop)
#endif



#include "IDTMTypes.hpp"

} //End namespace IDTMFile