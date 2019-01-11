//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFPacketManager.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @description  Returns how many packets are stored in the current directory.
* @return       Stored packet count for this directory.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t PacketManagerBase::GetCount () const
    {
    return GetFile().GetPacketCount();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline uint64_t PacketManagerBase::GetTotalSize () const
    {
    return GetFile().GetTotalUncompresedPacketSize();
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t PacketManagerBase::GetMaxSize () const
    {
    return GetFile().GetMaxUncompressedPacketSize();
    }


/*---------------------------------------------------------------------------------**//**
* @description  Returns the size of the specified packet.
* @param        pi_ID   ID identifying the packet.
* @return       Size of the packet, or 0 if the packet does not exist.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline size_t PacketManagerBase::GetSize (PacketID   pi_ID) const
    {
    return GetFile().GetUncompressedPacketSize(pi_ID);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketManagerBase::Exist (PacketID pi_ID) const
    {
    return GetFile().DoesPacketExist(pi_ID);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Get an existing packet's data from the directory
* @param        IN  pi_ID           the ID referring to the desired packet
* @param        OUT pi_rDataPacket  the packet data
* @return       true on success false otherwise
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketManagerBase::Get (PacketID    pi_ID,
                                    Packet&     po_rPacket) const
    {
    if (po_rPacket.IsReadOnly())
        return false;


    return BentleyApi::ImagePP::H_SUCCESS == GetFile().ReadPacket(po_rPacket.EditPacket(), pi_ID);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Set a new value to an existing packet in the directory
* @param        IN pi_ID            the ID referring to the modified packet
* @param        IN pi_rDataPacket   the packet data
* @return       true on success false otherwise
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketManagerBase::Set (PacketID        pi_ID,
                                    const Packet&   pi_rPacket)
    {
    // TDORAY: Consider putting that back
    //if (!GetImpl().DoesPacketExist(pi_ID))
    //return false;

    return BentleyApi::ImagePP::H_SUCCESS == GetFile().WritePacket(pi_rPacket.GetPacket(), pi_ID);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Add a new packet in the directory
* @param        OUT po_ID           the ID referring to the added packet
* @param        IN  pi_rDataPacket  the packet that is added in the directory
* @return       true on success false otherwise
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketManagerBase::Add (PacketID&       po_rID,
                                    const Packet&   pi_rPacket)
    {
    // Append packet
    TagFile::PacketID ID;
    const bool Success = (BentleyApi::ImagePP::H_SUCCESS == GetFile().AppendPacket(pi_rPacket.GetPacket(), ID));

    // Return ID only on success
    if (Success)
        po_rID = ID;

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
inline bool PacketManagerBase::Remove (PacketID pi_ID)
    {
    return BentleyApi::ImagePP::H_SUCCESS == GetFile().RemovePacket(pi_ID);
    }


template <typename EditorT>
const PacketIterManager<EditorT>& AsTypedIterMgr (const PacketManagerBase& pi_rBase)
    {
    return static_cast<const SubDirIterManager<EditorT>&>(pi_rBase);
    }

template <typename EditorT>
PacketIterManager<EditorT>& AsTypedIterMgr (PacketManagerBase& pi_rBase)
    {
    return static_cast<SubDirIterManager<EditorT>&>(pi_rBase);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<const EditorT> PacketIterManager<EditorT>::begin (const editor_base_type& pi_rEditorBase) const
    {
    return PacketIter<const EditorT>::Create(beginId(), pi_rEditorBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<const EditorT> PacketIterManager<EditorT>::end (const editor_base_type& pi_rEditorBase) const
    {
    return PacketIter<const EditorT>::Create(endId(), pi_rEditorBase);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<const EditorT>   PacketIterManager<EditorT>::Find   (PacketID                pi_ID,
                                                            const editor_base_type& pi_rEditorBase) const
    {
    return PacketIter<const EditorT>::Create(FindIdIterFor(pi_ID), pi_rEditorBase);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT> PacketIterManager<EditorT>::begin (editor_base_type& pi_rEditorBase)
    {
    return PacketIter<EditorT>::Create(beginId(), pi_rEditorBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT> PacketIterManager<EditorT>::end (editor_base_type& pi_rEditorBase)
    {
    return PacketIter<EditorT>::Create(endId(), pi_rEditorBase);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   3/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT> PacketIterManager<EditorT>::Find   (PacketID            pi_ID,
                                                    editor_base_type&   pi_rEditorBase)
    {
    return PacketIter<EditorT>::Create(FindIdIterFor(pi_ID), pi_rEditorBase);
    }


