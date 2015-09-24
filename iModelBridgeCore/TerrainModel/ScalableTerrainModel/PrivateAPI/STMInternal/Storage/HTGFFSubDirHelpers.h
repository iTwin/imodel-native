//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirHelpers.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <STMInternal/Storage/HTGFFDirectory.h>
#include <STMInternal/Storage/HPUPacket.h>
#include <STMInternal/Storage/HPUArray.h>

#include <STMInternal/Storage/HTGFFPacketIdIter.h>

namespace HTGFF {


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketIndexSubDirBase : public Directory
    {
protected:
    explicit                        PacketIndexSubDirBase                  ();
    virtual                         ~PacketIndexSubDirBase                 ();


    PacketIdIter                    beginId                            () const;
    PacketIdIter                    endId                              () const;

    bool                            Get                                (PacketID                        pi_id,
                                                                        Packet&                         po_rPacket) const;

    bool                            Add                                (PacketID&                       pi_id,
                                                                        const Packet&                   pi_rPacket);

    bool                            Set                                (PacketID                        pi_id,
                                                                        const Packet&                   pi_rPacket);
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketIndexSubDir : public PacketIndexSubDirBase
    {
    typedef PacketIndexSubDirBase   super_class;

public:
    using                           super_class::beginId;
    using                           super_class::endId;

    using                           super_class::Get;
    using                           super_class::Add;
    using                           super_class::Set;
    };




/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ArrayIndexSubDirBase : public PacketIndexSubDirBase
    {
    typedef PacketIndexSubDirBase   super_class;

protected:
    typedef PacketID                ArrayID;
    typedef HPU::Array<T>           Array;

    explicit                        ArrayIndexSubDirBase               () {}
    virtual                         ~ArrayIndexSubDirBase              () {}

    bool                            Get                                (ArrayID                         pi_id,
                                                                        Array&                          po_rArray) const;

    bool                            Add                                (ArrayID&                        pi_id,
                                                                        const Array&                    pi_rArray);

    bool                            Set                                (ArrayID                         pi_id,
                                                                        const Array&                    pi_rArray);

    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ArrayIndexSubDir : public ArrayIndexSubDirBase<T>
    {
    typedef ArrayIndexSubDirBase<T> super_class;

public:
    using                           super_class::ArrayID;
    using                           super_class::Array;

    using                           super_class::beginId;
    using                           super_class::endId;

    using                           super_class::Get;
    using                           super_class::Add;
    using                           super_class::Set;
    };




/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class AttributeSubDirBase : public PacketIndexSubDirBase
    {
    struct Block
        {
        explicit                    Block                              ()   :   m_dirty(false) {}

        Packet::Ptr                 m_pPacket;
        bool                        m_dirty;
        };

    typedef vector<Block>           BlockList;
    mutable BlockList               m_cachedBlocks;
    const size_t                    m_storedTypeSize;
    size_t                          m_typedBlockSizeBase2Log;
    size_t                          m_blockSize;
    size_t                          m_elementIndexMask;

    virtual bool                    _Create                            (const CreateConfig&             pi_rCreateConfig,
                                                                        const UserOptions*              pi_pUserOptions) override;
    virtual bool                    _Load                              (const UserOptions*              pi_pUserOptions) override;
    virtual bool                    _Save                              () override;

    virtual void                    _InitializePacket                  (Packet&                         pi_rPacket) const = 0;

    virtual void                    _SetDefaultValue                   (const void*                     pi_pDefaultValue) = 0;

    size_t                          GetBlockIndexFor                   (size_t                          pi_index) const;

    bool                            IsBlockLoaded                      (size_t                          pi_blockIndex) const;

    Packet&                         LoadBlock                          (size_t                          pi_blockIndex);
    const Packet&                   LoadBlock                          (size_t                          pi_blockIndex) const;


protected:
    struct Options : public UserOptions
        {
    protected:
        explicit                    Options                            (const void*                     pi_pDefaultValue);
    private:
        friend class                AttributeSubDirBase;
        const void*                 m_pDefaultValue;
        };

    explicit                        AttributeSubDirBase                (size_t                          pi_storedTypeSize);
    virtual                         ~AttributeSubDirBase               ();


    size_t                          GetElementIndexFor                 (size_t                          pi_index) const;


    Packet&                         EditPacketFor                      (size_t                          pi_index);
    const Packet&                   GetPacketFor                       (size_t                          pi_index) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class AttributeSubDir : public AttributeSubDirBase
    {
    T                               m_defaultValue;

    virtual void                    _InitializePacket                  (Packet&                         pi_rPacket) const override;

    virtual void                    _SetDefaultValue                   (const void*                     pi_pDefaultValue) override;

public:
    struct Options : public AttributeSubDirBase::Options
        {
        explicit                    Options                            (const T&                        pi_defaultValue);

    private:
        const T                     m_defaultValue;
        };

    explicit                        AttributeSubDir                    ();

    const T&                        Get                                (size_t                          pi_index) const;

    T&                              Edit                               (size_t                          pi_index);
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Mathieu.St-Pierre   11/2014
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class VarSizeAttributeSubDir : public Directory
    {                
    T m_defaultValue;        

    typedef HPU::Packet         Packet;
    typedef TagFile::PacketID   PacketID;

    struct Block
        {
        explicit    Block                              ()   :   m_dirty(false) {}

        Packet::Ptr m_pPacket;
        bool        m_dirty;
        };


    //typedef vector<AttributeSubDirBase::Block> BlockList;
    typedef vector<Block> BlockList;
    mutable BlockList m_cachedVarSizeDataPackets;

    bool GetVarSizeData   (T*& varSizeAttr, PacketID pi_packetID) const;
        
    virtual void                    _InitializePacket                  (Packet&                         pi_rPacket) const;

    virtual bool                    _Save                              () override;

    virtual void                    _SetDefaultValue                   (const void*                     pi_pDefaultValue);

public:
    struct Options : public UserOptions
        {
        explicit                    Options                            (const T&                        pi_defaultValue);

    private:
        const T                     m_defaultValue;
        };

    explicit                        VarSizeAttributeSubDir             ();

    virtual                         ~VarSizeAttributeSubDir            ();
    
    const T*                        Get                                (PacketID  pi_packetID) const;
    
    T*                              Edit                               (PacketID& pi_packetID, size_t pi_nbValues);
    };


#include <STMInternal/Storage/HTGFFSubDirHelpers.hpp>


} //End namespace HTGFF