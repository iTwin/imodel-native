//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFPacketManager.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/HPUPacket.h>

namespace HTGFF {

class PacketIdIter;

template <typename EditorT>
class PacketIter;

/*---------------------------------------------------------------------------------**//**
* @description
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketManagerBase : protected DirectoryManager
    {
    friend class                        DirectoryImpl;

    explicit                            PacketManagerBase              (Directory& pi_rDir) : DirectoryManager(pi_rDir) {}



public:
    typedef uint32_t                   PacketID;
    typedef HPU::Packet                 Packet;

protected:
    // Note: Not meant to be used polymorphically
                                        ~PacketManagerBase             () {}

    template <typename EditorT>
    struct EditorBaseTypeTrait          { typedef typename EditorT::base_type    type; };
    

public:
    size_t                              GetCount                       () const;
    uint64_t                           GetTotalSize                   () const;
    size_t                              GetMaxSize                     () const;

    const DataType&                     GetType                        () const;
    const Compression&                  GetCompression                 () const;

    double                              GetCompressionRatio            () const;

    size_t                              GetSize                        (PacketID                pi_ID) const;

    bool                                Exist                          (PacketID                pi_ID) const;

     PacketIdIter                 beginId                        () const;
     PacketIdIter                 endId                          () const;

     PacketIdIter                 FindIdIterFor                  (PacketID                pi_ID) const;

    bool                                Get                            (PacketID                pi_ID,
                                                                        Packet&                 po_rPacket) const;

    bool                                Set                            (PacketID                pi_ID,
                                                                        const Packet&           pi_rPacket);

    bool                                Add                            (PacketID&               po_rID,
                                                                        const Packet&           pi_rPacket);

    bool                                Remove                         (PacketID                pi_ID);
    };



/*---------------------------------------------------------------------------------**//**
* @description
* @see          Directory
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
class PacketIterManager : public PacketManagerBase
    {
    typedef typename EditorBaseTypeTrait<EditorT>::type 
                                        editor_base_type;
    

    // NOTE: Never constructed
    explicit                            PacketIterManager              ();    
    // Note: Not meant to be used polymorphically
                                        ~PacketIterManager             () {}


public:
    PacketIter<const EditorT>           begin                          (const editor_base_type& pi_rEditorBase) const;
    PacketIter<const EditorT>           end                            (const editor_base_type& pi_rEditorBase) const;

    PacketIter<const EditorT>           Find                           (PacketID                pi_ID,
                                                                        const editor_base_type& pi_rEditorBase) const;

    PacketIter<EditorT>                 begin                          (editor_base_type&       pi_rEditorBase);
    PacketIter<EditorT>                 end                            (editor_base_type&       pi_rEditorBase);

    PacketIter<EditorT>                 Find                           (PacketID                pi_ID,
                                                                        editor_base_type&       pi_rEditorBase);

    };


template <typename EditorT>
const PacketIterManager<EditorT>&       AsTypedIterMgr                 (const PacketManagerBase&
                                                                                                pi_rBase);
template <typename EditorT>
PacketIterManager<EditorT>&             AsTypedIterMgr                 (PacketManagerBase&      pi_rBase);

#include <STMInternal/Storage/HTGFFPacketManager.hpp>

} //End namespace HTGFF