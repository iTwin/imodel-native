/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Plugin/PacketBinderV0.h $
|    $RCSfile: PacketBinderV0.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/19 13:50:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
struct Packet;
struct PacketGroup;
END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketBinder
    {
    virtual void                            _Bind                              (const Packet&               src,
                                                                                Packet&                     dst) const = 0;

protected:

    IMPORT_DLLE explicit                    PacketBinder                       ();

public:
    IMPORT_DLLE virtual                     ~PacketBinder                      () = 0;

    IMPORT_DLLE void                        Bind                               (const Packet&               src,
                                                                                Packet&                     dst) const;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketGroupBinder
    {
    virtual void                            _Bind                              (const PacketGroup&           src,
                                                                                PacketGroup&                 dst) const = 0;

protected:
    IMPORT_DLLE explicit                    PacketGroupBinder                  ();

public:
    IMPORT_DLLE virtual                     ~PacketGroupBinder                 () = 0;

    IMPORT_DLLE void                        Bind                               (const PacketGroup&           src,
                                                                                PacketGroup&                 dst) const;
    };

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE
