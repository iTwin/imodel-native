//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

namespace IDTMFile {

inline PointTypeID FeatureDir::GetPointType () const
    {
    return GetPointDir().GetPointType();
    }

inline const HTGFF::Compression& FeatureDir::GetPointCompressType () const
    {
    return GetPointDir().GetCompressType();
    }

inline size_t FeatureDir::CountTiles () const
    {
    return GetPointDir().CountTiles();
    }

inline const Extent3d64f& FeatureDir::GetExtent (TileID pi_ID) const
    {
    return GetPointDir().GetExtent(pi_ID);
    }

inline Extent3d64f& FeatureDir::EditExtent (TileID pi_ID)
    {
    return GetPointDir().EditExtent(pi_ID);
    }

inline ResolutionID FeatureDir::GetResolution (TileID pi_ID) const
    {
    return GetPointDir().GetResolution(pi_ID);
    }

inline void FeatureDir::SetResolution  (TileID          pi_ID,
                                        ResolutionID    pi_Resolution)
    {
    return GetPointDir().SetResolution(pi_ID, pi_Resolution);
    }


inline size_t FeatureDir::GetTileMaxPointCount () const
    {
    return GetPointDir().GetTileMaxPointCount();
    }

inline uint64_t FeatureDir::CountPoints () const
    {
    return GetPointDir().CountPoints();
    }

inline size_t FeatureDir::CountPoints (TileID pi_ID) const
    {
    return GetPointDir().CountPoints(pi_ID);
    }


} //End namespace IDTMFile