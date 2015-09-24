//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderTileHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : FeatureHeaderTileHandler
//-----------------------------------------------------------------------------

#pragma once

#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureHeaderDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>

#include <STMInternal/Storage/HTGFFDirectoryHandler.h>

namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* @description  Handler for feature headers. Is not to be used directly. Use it
*               through FeatureTileHandler.
*
* @see          FeatureTileHandler
* @see          FeatureHeaderDir
*
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HeaderT>
class FeatureHeaderTileHandler : public HTGFF::DirectoryHandler
    {
public:
    typedef BentleyApi::ImagePP::HFCPtr<FeatureHeaderTileHandler<HeaderT> >
    Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<FeatureHeaderTileHandler<HeaderT> >
    CPtr;

    typedef HPU::Array<HeaderT>
    HeaderArray;

    virtual                     ~FeatureHeaderTileHandler          ();

    bool                        IsPointOnly                        () const;

    size_t                      GetTileMaxHeaderCount              () const;
    uint64_t                   CountHeaders                       () const;
    size_t                      CountHeaders                       (TileID                      pi_ID) const;

    bool                        GetHeaders                         (TileID                      pi_ID,
                                                                    HeaderArray&                pi_rTileHeaders);

    bool                        SetHeaders                         (TileID                      pi_ID,
                                                                    const HeaderArray&          pi_rTileHeaders);

    bool                        AddHeaders                         (TileID&                     po_rID,
                                                                    const HeaderArray&          pi_rTileHeaders);

    bool                        RemoveHeaders                      (TileID                      pi_ID);

private:

    template <typename PointType, typename HeaderT>
    friend class FeatureTileHandler;

    struct Impl;
    struct LinearFeatureImpl;
    struct PointOnlyImpl;

    /*---------------------------------------------------------------------------------**//**
    * Compile time listing of supported point types for this handler
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename T>       struct IsSupportedHeaderTypeTrait                               {
        enum {value = 0};
        };
    template <>                 struct IsSupportedHeaderTypeTrait<FeatureHeader>                {
        enum {value = 1};
        };

    static bool                 IsCompatibleWith                   (const FeatureDir&               pi_pDir);
    static Ptr                  CreateFrom                         (UniformFeatureDir*              pi_pDir);
    static Ptr                  CreateFrom                         (MixedFeatureDir*                pi_pDir);
    static CPtr                 CreateFrom                         (const UniformFeatureDir*        pi_pDir);
    static CPtr                 CreateFrom                         (const MixedFeatureDir*          pi_pDir);

    static FeatureHeaderTileHandler*
    CreateFromImpl                     (UniformFeatureDir*              pi_pDir);
    static FeatureHeaderTileHandler*
    CreateFromImpl                     (MixedFeatureDir*                pi_pDir);

    explicit                    FeatureHeaderTileHandler           (Impl*                           pi_pImpl);

    virtual bool                _Save                              () override;
    virtual bool                _Load                              () override;


    auto_ptr<Impl>              m_pImpl;
    };


#include "IDTMFeatureHeaderTileHandler.hpp"

} //End namespace IDTMFile