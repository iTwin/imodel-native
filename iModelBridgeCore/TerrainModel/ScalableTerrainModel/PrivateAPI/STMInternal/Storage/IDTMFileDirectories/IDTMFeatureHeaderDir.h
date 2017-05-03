//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeatureHeaderDir
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/IDTMFileDirectories/FeatureHeaderTypes.h>

#include <STMInternal/Storage/IDTMTypes.h>
namespace IDTMFile {


 size_t               GetTypeSize                        (FeatureHeaderTypeID     pi_Type);
 const HTGFF::DataType&
                            GetTypeDescriptor                  (FeatureHeaderTypeID     pi_Type);


class FeatureHeaderDir : public HTGFF::Directory
    {
public:
     static uint32_t  s_GetVersion                       ();

     FeatureHeaderTypeID  
                            GetHeaderType                      () const;

     size_t           GetTileMaxHeaderCount              () const;

     uint64_t         CountHeaders                       () const;
     size_t           CountHeaders                       (TileID                  pi_ID) const;

    explicit                FeatureHeaderDir                   ();  // Should be private, Android problem.

private:
    friend class            HTGFF::Directory;
    friend class            FeatureDir;
    template <typename HeaderT>
    friend class            FeatureHeaderTileHandler;
    friend class            FeatureHeaderPacketHandler;

    /*---------------------------------------------------------------------------------**//**
    * Untyped data accessors. Only available through FeatureHeaderTileHandler.
    +---------------+---------------+---------------+---------------+---------------+------*/
     bool             GetHeaders                         (TileID                  pi_ID,
                                                                Packet&                 po_rHeaders) const;
     bool             SetHeaders                         (TileID                  pi_ID,
                                                                const Packet&           pi_rHeaders);
     bool             AddHeaders                         (TileID&                 po_rID,
                                                                const Packet&           pi_rHeaders);
     bool             RemoveHeaders                      (TileID                  pi_ID);

    bool                    _Load                              (const UserOptions*      pi_pUserOptions) override;

    size_t                  m_HeaderTypeSize;
    FeatureHeaderTypeID     m_HeaderType;
    };





} //End namespace IDTMFile