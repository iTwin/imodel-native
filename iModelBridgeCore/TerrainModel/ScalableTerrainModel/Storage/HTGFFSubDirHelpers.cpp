//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFSubDirHelpers.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFSubDirHelpers.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFPacketManager.h>

namespace HTGFF {



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIndexSubDirBase::PacketIndexSubDirBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIndexSubDirBase::~PacketIndexSubDirBase ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter PacketIndexSubDirBase::beginId () const
    {
    return PacketMgr().beginId();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketIdIter PacketIndexSubDirBase::endId () const
    {
    return PacketMgr().endId();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PacketIndexSubDirBase::Get(PacketID    pi_id,
                                Packet&     po_rPacket) const
    {
    return PacketMgr().Get(pi_id, po_rPacket);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PacketIndexSubDirBase::Add    (PacketID&       pi_id,
                                    const Packet&   pi_rPacket)
    {
    return PacketMgr().Add(pi_id, pi_rPacket);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool PacketIndexSubDirBase::Set(PacketID        pi_id,
                                const Packet&   pi_rPacket)
    {
    return PacketMgr().Set(pi_id, pi_rPacket);
    }


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeSubDirBase::Options::Options  (const void* pi_pDefaultValue)
    :   m_pDefaultValue(pi_pDefaultValue)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeSubDirBase::AttributeSubDirBase (size_t pi_storedTypeSize)
    :   m_storedTypeSize(pi_storedTypeSize),
        m_typedBlockSizeBase2Log(12),
        m_blockSize(((size_t)0x1 << m_typedBlockSizeBase2Log) * m_storedTypeSize),
        m_elementIndexMask(((size_t)0x1 << m_typedBlockSizeBase2Log) - 1)
    {
    HASSERT(0 < m_storedTypeSize);
    }



/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeSubDirBase::~AttributeSubDirBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttributeSubDirBase::_Create  (const CreateConfig&     pi_rCreateConfig,
                                    const UserOptions*      pi_pUserOptions)
    {
    HPRECONDITION(m_storedTypeSize == pi_rCreateConfig.m_DataType.GetSize());

    if (0 == pi_pUserOptions)
        {
        HASSERT(!"Missing config options!");
        return false;
        }

    _SetDefaultValue(pi_pUserOptions->SafeReinterpretAs<Options>().m_pDefaultValue);

    // This is how we ensure we can retrieve size on load
    LoadBlock(0);
    return true;
    }


namespace { // BEGIN unnamed namespace

bool IsPowerOf2 (size_t integer)
    {
    return integer && !(integer & (integer - 1));
    }

size_t ComputeBase2Log (size_t integer)
    {
    size_t result = 0;
    while (integer >>= 1) {
        ++result;
        }
    return result;
    }


} // END unnamed namespace

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttributeSubDirBase::_Load (const UserOptions*      pi_pUserOptions)
    {
    if (0 == pi_pUserOptions)
        {
        HASSERT(!"Missing config options!");
        return false;
        }

    _SetDefaultValue(pi_pUserOptions->SafeReinterpretAs<Options>().m_pDefaultValue);

    const size_t blockSize = IsBlockLoaded(0) ? m_cachedBlocks[0].m_pPacket->GetSize() : PacketMgr().GetSize(0);

    size_t typedBlockSize = blockSize / m_storedTypeSize;

    if (!IsPowerOf2(typedBlockSize))
        {
        HASSERT(!"Typed block size is not a power of two");
        return false;
        }

    m_typedBlockSizeBase2Log = ComputeBase2Log(typedBlockSize);
    HASSERT(typedBlockSize == ((size_t)0x1 << m_typedBlockSizeBase2Log));

    m_blockSize = blockSize;
    m_elementIndexMask = typedBlockSize - 1;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool AttributeSubDirBase::_Save ()
    {
    typedef BlockList::iterator BlockIt;

    bool Success = true;

    PacketID id = 0;
    for (BlockIt blockIt = m_cachedBlocks.begin(), endBlockIt = m_cachedBlocks.end();
         blockIt != endBlockIt;
         ++blockIt, ++id)
        {
        if (!blockIt->m_dirty)
            continue;

        if (!Set(id, *blockIt->m_pPacket))
            {
            Success = false;
            continue;
            }

        blockIt->m_dirty = false;
        }

    return Success;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
AttributeSubDirBase::Packet& AttributeSubDirBase::LoadBlock    (size_t         pi_blockIndex)
    {
    if (m_cachedBlocks.size() <= pi_blockIndex)
        m_cachedBlocks.resize(pi_blockIndex + 1);

    Block& rBlock = m_cachedBlocks[pi_blockIndex];

    if (0 != rBlock.m_pPacket)
        return *rBlock.m_pPacket;


    rBlock.m_pPacket = new Packet;

    const PacketID packetId = static_cast<PacketID>(pi_blockIndex);
    if (!PacketMgr().Exist(packetId) || !Get(packetId, *rBlock.m_pPacket))
        {
        rBlock.m_pPacket->Resize(m_blockSize);
        _InitializePacket(*rBlock.m_pPacket);
        }

    HASSERT(m_blockSize == rBlock.m_pPacket->GetSize());

    rBlock.m_dirty = true;
    return *rBlock.m_pPacket;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const AttributeSubDirBase::Packet& AttributeSubDirBase::LoadBlock  (size_t          pi_blockIndex) const
    {
    if (m_cachedBlocks.size() <= pi_blockIndex)
        m_cachedBlocks.resize(pi_blockIndex + 1);

    Block& rBlock = m_cachedBlocks[pi_blockIndex];

    if (0 != rBlock.m_pPacket)
        return *rBlock.m_pPacket;


    rBlock.m_pPacket = new Packet;
    if (!Get(static_cast<PacketID>(pi_blockIndex), *rBlock.m_pPacket))
        {
        HASSERT(!"Could not load array!");
        rBlock.m_pPacket->Resize(m_blockSize);
        _InitializePacket(*rBlock.m_pPacket);
        }

    HASSERT(m_blockSize == rBlock.m_pPacket->GetSize());

    return *rBlock.m_pPacket;
    }


} //End namespace HTGFF
