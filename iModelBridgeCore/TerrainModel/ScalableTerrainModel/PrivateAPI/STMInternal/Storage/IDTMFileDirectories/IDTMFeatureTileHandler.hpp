//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMFeatureTileHandler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::IsCompatibleWith (const FeatureDir& pi_rFeatureDir)
    {
    return PointHandler::IsCompatibleWith(pi_rFeatureDir.GetPointDir()) && HeaderHandler::IsCompatibleWith(pi_rFeatureDir);
    }


template <typename PointType, typename HeaderT>
template <typename DirT>
FeatureTileHandler<PointType, HeaderT>*
FeatureTileHandler<PointType, HeaderT>::CreateFromImpl (DirT* pi_rpFeatureDir)
    {
    if (0 == pi_rpFeatureDir)
        return 0;

    // NOTE: IsCompatibleWith is called from PointTileHandler::CreateFrom
    PointHandler::Ptr pPointTileHandler = PointHandler::CreateFrom(pi_rpFeatureDir->GetPointDirP());
    if (0 == pPointTileHandler)
        return 0;

    HeaderHandler::Ptr pHeaderTileHandler = HeaderHandler::CreateFrom(pi_rpFeatureDir);
    if (0 == pHeaderTileHandler)
        return 0;

    return new FeatureTileHandler (pi_rpFeatureDir, pHeaderTileHandler, pPointTileHandler);
    }


template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::Ptr
FeatureTileHandler<PointType, HeaderT>::CreateFrom (UniformFeatureDir* pi_rpFeatureDir)
    {
    return CreateFromImpl(pi_rpFeatureDir);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::Ptr
FeatureTileHandler<PointType, HeaderT>::CreateFrom (MixedFeatureDir* pi_rpFeatureDir)
    {
    return CreateFromImpl(pi_rpFeatureDir);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::CPtr
FeatureTileHandler<PointType, HeaderT>::CreateFrom (const UniformFeatureDir* pi_rpFeatureDir)
    {
    return CreateFromImpl(const_cast<UniformFeatureDir*>(pi_rpFeatureDir));
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::CPtr
FeatureTileHandler<PointType, HeaderT>::CreateFrom (const MixedFeatureDir* pi_rpFeatureDir)
    {
    return CreateFromImpl(const_cast<MixedFeatureDir*>(pi_rpFeatureDir));
    }

template <typename PointType, typename HeaderT>
FeatureTileHandler<PointType, HeaderT>::FeatureTileHandler (FeatureDir*                         pi_rpFeatureDir,
                                                            typename const HeaderHandler::Ptr&  pi_rpHeaderTileHandler,
                                                            typename const PointHandler::Ptr&   pi_rpPointTileHandler)
    :   m_pFeatureDir(pi_rpFeatureDir),
        m_pHeaderTileHandler(pi_rpHeaderTileHandler),
        m_pPointTileHandler(pi_rpPointTileHandler)
    {
    }



template <typename PointType, typename HeaderT>
inline TileID FeatureTileHandler<PointType, HeaderT>::TileEditor::GetID () const
    {
    return GetPacketID();
    }

template <typename PointType, typename HeaderT>
const Extent3d64f& FeatureTileHandler<PointType, HeaderT>::TileEditor::GetExtent () const
    {
    return GetBase().GetDir().GetExtent(GetPacketID());
    }

template <typename PointType, typename HeaderT>
ResolutionID FeatureTileHandler<PointType, HeaderT>::TileEditor::GetResolution  () const
    {
    return GetBase().GetDir().GetResolution(GetPacketID());
    }

template <typename PointType, typename HeaderT>
size_t FeatureTileHandler<PointType, HeaderT>::TileEditor::CountFeatures () const
    {
    return GetBase().GetDir().CountFeatures(GetPacketID());
    }

template <typename PointType, typename HeaderT>
size_t FeatureTileHandler<PointType, HeaderT>::TileEditor::CountPoints () const
    {
    return GetBase().GetDir().CountPoints(GetPacketID());
    }

template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::TileEditor::GetFeatures (Array& po_rFeatures) const
    {
    return GetBase().GetFeatures(GetPacketID(), po_rFeatures);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::TileCIter FeatureTileHandler<PointType, HeaderT>::TilesBegin () const
    {
    return m_pFeatureDir->GetPointDir().PacketMgr<CTile>().begin(*this);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::TileCIter FeatureTileHandler<PointType, HeaderT>::TilesEnd () const
    {
    return m_pFeatureDir->GetPointDir().PacketMgr<CTile>().end(*this);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::TileIter FeatureTileHandler<PointType, HeaderT>::TilesBeginEdit ()
    {
    return m_pFeatureDir->GetPointDir().PacketMgr<Tile>().begin(*this);
    }

template <typename PointType, typename HeaderT>
typename FeatureTileHandler<PointType, HeaderT>::TileIter FeatureTileHandler<PointType, HeaderT>::TilesEndEdit ()
    {
    return m_pFeatureDir->GetPointDir().PacketMgr<Tile>().end(*this);
    }


template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::GetFeatures   (TileID  pi_ID,
                                                            Array&      po_rFeatures) const
    {
    if (!m_pHeaderTileHandler->GetHeaders(pi_ID, po_rFeatures.EditHeaders()))
        return false;

    if (!m_pPointTileHandler->GetPoints(pi_ID, po_rFeatures.EditPoints()))
        return false;

    return true;
    }


template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::SetFeatures   (TileID      pi_ID,
                                                            const Array&    pi_rFeatures)
    {
    if (!m_pHeaderTileHandler->SetHeaders(pi_ID, pi_rFeatures.GetHeaders()))
        return false;

    if (!m_pPointTileHandler->SetPoints(pi_ID, pi_rFeatures.GetPoints()))
        {
        HASSERT(!"Feature group lost sync!!!");
        return false;
        }

    return true;
    }

template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::AddFeatures   (TileID&     po_rID,
                                                            const Array&    pi_rFeatures)
    {
    if (!m_pHeaderTileHandler->AddHeaders(po_rID, pi_rFeatures.GetHeaders()))
        return false;

    if (!m_pPointTileHandler->SetPoints(po_rID, pi_rFeatures.GetPoints()))
        {
        HASSERT(!"Feature group lost sync!!!");
        return false;
        }

    return true;
    }


template <typename PointType, typename HeaderT>
bool FeatureTileHandler<PointType, HeaderT>::RemoveFeatures  (TileID pi_ID)
    {
    if (!m_pHeaderTileHandler->RemoveHeaders(pi_ID))
        return false;

    if (!m_pPointTileHandler->RemovePoints(pi_ID))
        {
        HASSERT(!"Feature group lost sync!!!");
        return false;
        }

    return true;
    }
