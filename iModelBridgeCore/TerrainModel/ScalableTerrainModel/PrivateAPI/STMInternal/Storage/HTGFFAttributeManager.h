//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFAttributeManager.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/HPUPacket.h>

namespace HTGFF {

/*---------------------------------------------------------------------------------**//**
* @description
*
* TDORAY: Use overloads instead of templates
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class AttributeManager : protected DirectoryManager
    {
    friend class                DirectoryImpl;

    explicit                    AttributeManager               (Directory& pi_rDir) : DirectoryManager(pi_rDir) {}

public:
    typedef uint32_t             AttributeID;
    typedef HPU::Packet         Packet;

    bool                        IsPresent                      (AttributeID             pi_ID) const;

    bool                        Remove                         (AttributeID             pi_ID);

    template<typename T>
    bool                        Get                            (AttributeID             pi_ID,
                                                                T&                      po_rData) const;

    template<typename T>
    bool                        Set                            (AttributeID             pi_ID,
                                                                const T&                pi_rData);

    template<typename T>
    bool                        Get                            (AttributeID             pi_ID,
                                                                T*&                     po_rpData,
                                                                size_t&                 po_rSize) const;

    template<typename T>
    bool                        Set                            (AttributeID             pi_ID,
                                                                const T*                pi_pData,
                                                                size_t                  pi_Size);

    template<typename UnderlyingType, typename T>
    bool                        GetTyped                       (AttributeID             pi_ID,
                                                                T*&                     po_rpData,
                                                                size_t&                 po_Size) const;

    template<typename UnderlyingType, typename T>
    bool                        SetTyped                       (AttributeID             pi_ID,
                                                                const T*                pi_pData,
                                                                size_t                  pi_Size);


    template<typename UnderlyingType>
    bool                        GetPacket                      (AttributeID             pi_ID,
                                                                Packet&                 po_rPacket) const;

    template<typename UnderlyingType>
    bool                        SetPacket                      (AttributeID             pi_ID,
                                                                const Packet&           pi_rPacket);
    };


#include <STMInternal/Storage/HTGFFAttributeManager.hpp>

} //End namespace HTGFF