//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/IDTMFileDirectories/IDTMFeatureHeaderDir.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeatureHeaderDir
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HTGFFDirectory.h>

#include <Imagepp/all/h/IDTMFileDirectories/FeatureHeaderTypes.h>

#include <ImagePP/all/h/IDTMTypes.h>
namespace IDTMFile {


_HDLLg size_t               GetTypeSize                        (FeatureHeaderTypeID     pi_Type);
_HDLLg const HTGFF::DataType&
                            GetTypeDescriptor                  (FeatureHeaderTypeID     pi_Type);


class FeatureHeaderDir : public HTGFF::Directory
    {
public:
    _HDLLg static uint32_t  s_GetVersion                       ();

    _HDLLg FeatureHeaderTypeID  
                            GetHeaderType                      () const;

    _HDLLg size_t           GetTileMaxHeaderCount              () const;

    _HDLLg uint64_t         CountHeaders                       () const;
    _HDLLg size_t           CountHeaders                       (TileID                  pi_ID) const;

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
    _HDLLg bool             GetHeaders                         (TileID                  pi_ID,
                                                                Packet&                 po_rHeaders) const;
    _HDLLg bool             SetHeaders                         (TileID                  pi_ID,
                                                                const Packet&           pi_rHeaders);
    _HDLLg bool             AddHeaders                         (TileID&                 po_rID,
                                                                const Packet&           pi_rHeaders);
    _HDLLg bool             RemoveHeaders                      (TileID                  pi_ID);

    bool                    _Load                              (const UserOptions*      pi_pUserOptions) override;

    size_t                  m_HeaderTypeSize;
    FeatureHeaderTypeID     m_HeaderType;
    };





} //End namespace IDTMFile