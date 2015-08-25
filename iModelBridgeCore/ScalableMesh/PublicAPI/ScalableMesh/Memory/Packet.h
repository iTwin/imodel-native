/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Memory/Packet.h $
|    $RCSfile: Packet.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/09/07 14:20:36 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <assert.h>

#include <ScalableMesh/Memory/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE

class RawPacket;
class PacketProxyBase;

struct Packet;
struct PacketGroup;

struct RawCapacities;
struct MemoryAllocator;

typedef SharedPtrTypeTrait<RawPacket>::type
                                        RawPacketPtr;
typedef SharedPtrTypeTrait<PacketGroup>::type
                                        PacketGroupPtr;


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Packet
    {
public:
    class                               Impl;

private:
    friend struct                       PacketGroup;
    friend class                        RawPacket;
    friend class                        PacketProxyBase;

    typedef SharedPtrTypeTrait<Impl>::type
                                        ImplPtr;
    ImplPtr                             m_pImpl;

                                        Packet                             (const Packet&           rhs);
    Packet&                             operator=                          (const Packet&           rhs);    

    MEMORY_DLLE size_t                  GetSize                            () const;

    MEMORY_DLLE bool                    IsReadOnly                         () const;
    
    
    MEMORY_DLLE void                    NotifyOfRealloc                    () const;


public:

    MEMORY_DLLE void                    SetReadOnly                        (bool                    readOnly);

    MEMORY_DLLE explicit                Packet                             (size_t                  capacity = 0);
    MEMORY_DLLE explicit                Packet                             (const MemoryAllocator&  allocator,
                                                                            size_t                  capacity = 0);

    MEMORY_DLLE                         ~Packet                            ();

    MEMORY_DLLE const MemoryAllocator&  GetAllocator                       () const;

    MEMORY_DLLE size_t                  GetCapacity                        () const;

    MEMORY_DLLE void                    Reserve                            (size_t                  newCapacity);
    MEMORY_DLLE void                    Clear                              ();

    MEMORY_DLLE void                    BindCapacityTo                     (const Packet&           packet);

    MEMORY_DLLE void                    BindCapacityTo                     (const Packet&           packet,
                                                                            size_t                  ratioNumerator,
                                                                            size_t                  ratioDenominator);

    MEMORY_DLLE void                    BindReferToSameAs                  (const Packet&           packet);
    MEMORY_DLLE void                    BindReferToSameAs                  (Packet&                 packet);

    MEMORY_DLLE void                    BindUseSameAs                      (const Packet&           packet);
    MEMORY_DLLE void                    BindUseSameAs                      (Packet&                 packet);

    MEMORY_DLLE RawPacket&              GetRawPacket                       ();
    MEMORY_DLLE const RawPacket&        GetRawPacket                       () const;

    template <typename T>
    void                                Wrap                               (const T*                dataP,
                                                                            size_t                  size);        
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PacketGroup : private Uncopyable, public ShareableObjectTypeTrait<PacketGroup>::type
    {
private:
    struct                              PacketHolder;
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;


    explicit                            PacketGroup                        (Impl*                   implP);
public:
    typedef const PacketHolder*         const_iterator;
    typedef PacketHolder*               iterator;

    MEMORY_DLLE explicit                PacketGroup                        (size_t                  size);

    MEMORY_DLLE explicit                PacketGroup                        (size_t                  size,
                                                                            const MemoryAllocator&  allocator);

    MEMORY_DLLE explicit                PacketGroup                        (const RawCapacities&    packetCapacities);

    MEMORY_DLLE explicit                PacketGroup                        (const RawCapacities&    packetCapacities,
                                                                            const MemoryAllocator&  allocator);


    MEMORY_DLLE                         ~PacketGroup                       ();

    MEMORY_DLLE const MemoryAllocator&  GetAllocator                       () const;

    MEMORY_DLLE void                    Reserve                            (const RawCapacities&    newPacketCapacities);

    MEMORY_DLLE void                    Clear                              ();

    MEMORY_DLLE Packet&                 operator[]                         (size_t                  idx);
    MEMORY_DLLE const Packet&           operator[]                         (size_t                  idx) const;

    MEMORY_DLLE const_iterator          begin                              () const;
    MEMORY_DLLE const_iterator          end                                () const;

    MEMORY_DLLE iterator                begin                              ();
    MEMORY_DLLE iterator                end                                ();

    MEMORY_DLLE size_t                  GetSize                            () const;

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct RawCapacities
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    typedef size_t                      value_type;
    typedef const value_type*           const_iterator;
    typedef value_type&                 reference;
    typedef const value_type&           const_reference;

    template <typename IterT>
    static RawCapacities                CreateFrom                         (IterT                   capacitiesBegin,
                                                                            IterT                   capacitiesEnd);

    // TDORAY: Would profits using C++0x initializer list cstor
    MEMORY_DLLE explicit                RawCapacities                      ();
    MEMORY_DLLE explicit                RawCapacities                      (size_t s0);
    MEMORY_DLLE explicit                RawCapacities                      (size_t s0, size_t s1);
    MEMORY_DLLE explicit                RawCapacities                      (size_t s0, size_t s1, size_t s2);
    MEMORY_DLLE explicit                RawCapacities                      (size_t s0, size_t s1, size_t s2, size_t s3);

    MEMORY_DLLE                         RawCapacities                      (const RawCapacities&    rhs);
    MEMORY_DLLE RawCapacities&          operator=                          (const RawCapacities&    rhs);

    MEMORY_DLLE                         ~RawCapacities                     ();

    MEMORY_DLLE size_t                  GetSize                            () const;


    MEMORY_DLLE const size_t&           operator[]                         (size_t                  idx) const;

    MEMORY_DLLE const_iterator          begin                              () const;
    MEMORY_DLLE const_iterator          end                                () const;

    
    MEMORY_DLLE void                    push_back                          (size_t                  capacity);
    };

template <typename IterT>
inline RawCapacities RawCapacities::CreateFrom     (IterT   capacitiesBegin,
                                                    IterT   capacitiesEnd)
    {
    RawCapacities capacities;
    std::copy(capacitiesBegin, capacitiesEnd, back_inserter(capacities));
    return capacities;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* TDORAY: Implement an internal non-typed wrap on which the typed wrap depends.
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void Packet::Wrap  (const T*    data,
                    size_t      size)
    {
    SetReadOnly(true);
    GetRawPacket().Wrap(data, size, size);

    NotifyOfRealloc();
    }

END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
