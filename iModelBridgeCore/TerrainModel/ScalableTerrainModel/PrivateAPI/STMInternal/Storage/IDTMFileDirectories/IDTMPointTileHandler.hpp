//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/IDTMFileDirectories/IDTMPointTileHandler.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* @description  Return whether the user can use his point construct with the current
*               file or not.
* @return       true when the construct is compatible, false otherwise
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::IsCompatibleWith (const PointDir& pi_rPointDir)
    {
    HPRECONDITION(sizeof(PointType) == GetTypeSize(pi_rPointDir.GetPointType()));

    return IsSupportedPointTypeTrait<PointTypeIDTrait<PointType>::value>::value
           && PointTypeIDTrait<PointType>::value == pi_rPointDir.GetPointType();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::IsCompatibleWith (const UniformFeatureDir& pi_rDir)
    {
    return 0 != pi_rDir.GetPointDir() && IsCompatibleWith(*pi_rDir.GetPointDir());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
PointTileHandler<PointType>* PointTileHandler<PointType>::CreateFromImpl (PointDir* pi_pDir)
    {
    // Compile time check ensuring that point type is supported. If your custom point type
    // can map on one of the actually supported type id, you should consider defining a
    // point type trait for your point type in your module (see PointTypes.h).
    HSTATICASSERT(IsSupportedPointTypeTrait<PointTypeIDTrait<PointType>::value>::value);

    if (0 == pi_pDir)
        return 0;

    if (!IsCompatibleWith(*pi_pDir))
        return 0;

    return new PointTileHandler(pi_pDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::Ptr PointTileHandler<PointType>::CreateFrom (PointDir* pi_pDir)
    {
    return CreateFromImpl(pi_pDir);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::Ptr PointTileHandler<PointType>::CreateFrom (UniformFeatureDir* pi_pDir)
    {
    return CreateFromImpl(pi_pDir->GetPointDir().GetPtr());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::CPtr PointTileHandler<PointType>::CreateFrom (const PointDir* pi_pDir)
    {
    return CreateFromImpl(const_cast<PointDir*>(pi_pDir));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::CPtr PointTileHandler<PointType>::CreateFrom (const UniformFeatureDir* pi_pDir)
    {
    return CreateFromImpl(const_cast<PointDir*>(pi_pDir->GetPointDir().GetPtr()));
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
TileID PointTileHandler<PointType>::TileEditor::GetID () const
    {
    return GetPacketID();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
const Extent3d64f& PointTileHandler<PointType>::TileEditor::GetExtent () const
    {
    return GetBase().GetDir().GetExtent(GetPacketID());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
ResolutionID PointTileHandler<PointType>::TileEditor::GetResolution () const
    {
    return GetBase().GetDir().GetResolution(GetPacketID());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
size_t PointTileHandler<PointType>::TileEditor::CountPoints () const
    {
    return GetBase().GetDir().CountPoints(GetPacketID());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::TileEditor::GetPoints (PointArray& po_rTilePoints) const
    {
    return GetBase().GetDir().GetPoints<PointType>(GetID(), po_rTilePoints);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::TileCIter PointTileHandler<PointType>::TilesBegin () const
    {
    return PacketMgr<CTile>(*m_pPointDir).begin(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::TileCIter PointTileHandler<PointType>::TilesEnd () const
    {
    return PacketMgr<CTile>(*m_pPointDir).end(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::TileIter PointTileHandler<PointType>::TilesBeginEdit ()
    {
    return PacketMgr<Tile>(*m_pPointDir).begin(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
typename PointTileHandler<PointType>::TileIter PointTileHandler<PointType>::TilesEndEdit ()
    {
    return PacketMgr<Tile>(*m_pPointDir).end(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Get the list of points of an existing tile
* @param        IN  pi_ID           The tile on which we want to operate
* @param        IN  pi_rTilePoints  The points that are to be added for the specified tile
* @return       true on success, false otherwise
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::GetPoints    (TileID          pi_ID,
                                                PointArray&     po_rTilePoints) const
    {
    return m_pPointDir->GetPoints(pi_ID, po_rTilePoints.EditPacket());
    }



/*---------------------------------------------------------------------------------**//**
* @description  Set the list of points of an existing tile
* @param        IN  pi_ID           The tile on which we want to operate
* @param        IN  pi_rTilePoints  The points that are to be added for the specified tile
* @return       true on success, false otherwise
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::SetPoints    (TileID              pi_ID,
                                                const PointArray&   pi_rTilePoints)
    {
    return m_pPointDir->SetPoints(pi_ID, pi_rTilePoints.GetPacket());
    }

/*---------------------------------------------------------------------------------**//**
* @description  Add a list a points to a new tile
* @param        OUT po_rID          The new tile's index
* @param        IN  pi_rTilePoints  The points that are to be added for the specified tile
* @return       true on success, false otherwise
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::AddPoints    (TileID&             po_rID,
                                                const PointArray&   pi_rTilePoints)
    {
    return m_pPointDir->AddPoints(po_rID, pi_rTilePoints.GetPacket());
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename PointType>
bool PointTileHandler<PointType>::RemovePoints (TileID  pi_ID)
    {
    return m_pPointDir->RemovePoints(pi_ID);
    }


} //End namespace IDTMFile