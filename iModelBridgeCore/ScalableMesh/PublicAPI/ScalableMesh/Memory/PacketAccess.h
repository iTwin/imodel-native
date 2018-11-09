/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Memory/PacketAccess.h $
|    $RCSfile: PacketAccess.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/08/26 18:47:29 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Memory/Definitions.h>
#include <ScalableMesh/Memory/Exceptions.h>

#include <ScalableMesh/Memory/Packet.h>

class RawPacket;

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description Base class for all packet proxies.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketProxyBase : private Uncopyable
    {
    template <typename T>
    friend class                        ConstPacketProxyBase;
    template <typename T>
    friend class                        NonConstPacketProxyBase;

    RawPacket*                          m_pImpl;
    explicit                            PacketProxyBase                    (RawPacket&  pi_rImpl);
    explicit                            PacketProxyBase                    ();

protected:
    template <typename ProxyT>
    static const RawPacket&             PrepareRawPacketFor                (const Packet&           packet,
                                                                            ProxyT&                 proxy);
    template <typename ProxyT>
    static RawPacket&                   PrepareRawPacketFor                (Packet&                 packet,
                                                                            ProxyT&                 proxy);


public:
    // Deliberately made destructor non-virtual. It would be inefficient to do otherwise as these are proxies
    // and are not meant to be used polymorphically

    bool                                operator==                         (const PacketProxyBase&  rhs) const 
                                        { return m_pImpl == rhs.m_pImpl; }
    bool                                operator!=                         (const PacketProxyBase&  rhs) const 
                                        { return m_pImpl != rhs.m_pImpl; }

    };

/*---------------------------------------------------------------------------------**//**
* @description Define methods common to all non-mutating packet proxies.   
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ConstPacketProxyBase : public PacketProxyBase
    {
    template <typename T2>
    friend class                        ConstPacketProxy;
    template <typename T3>
    friend class                        NonConstPacketProxyBase;

    explicit                            ConstPacketProxyBase               (const RawPacket&    impl);
    explicit                            ConstPacketProxyBase               ();

    void                                SetImpl                            (const RawPacket&    impl);

protected:
    const RawPacket&                    GetImpl                            () const;

public:
    typedef T                           value_type;
    typedef const value_type*           const_iterator;

    size_t                              GetCapacity                        () const;
    size_t                              GetSize                            () const;

    const T*                            Get                                () const { return cbegin(); }

    const_iterator                      cbegin                             () const;
    const_iterator                      cend                               () const;

    const_iterator                      begin                              () const { return cbegin(); }
    const_iterator                      end                                () const { return cend(); }
    };

/*---------------------------------------------------------------------------------**//**
* @description Define methods common to all mutating packet proxies. Mutating 
*              packet proxies also have all non-mutating access.
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class NonConstPacketProxyBase : public ConstPacketProxyBase<T>
    {    
    template <typename T2>
    friend class                        ClassPacketProxy;
    template <typename T3>
    friend class                        PODPacketProxy;

    explicit                            NonConstPacketProxyBase            (RawPacket&              impl);
    explicit                            NonConstPacketProxyBase            ();

    void                                SetImpl                            (RawPacket&              impl);

protected:
    using ConstPacketProxyBase<T>::GetImpl;

    RawPacket&                          GetImpl                            ();

public:
    typedef typename ConstPacketProxyBase<T>::value_type*                 iterator;

    using                               ConstPacketProxyBase<T>::begin;
    using                               ConstPacketProxyBase<T>::end;

    void                                Clear                              ();

    void                                Reserve (size_t newCapacity);

    void                                push_back                          (const T&                rhs);

    iterator                            begin                              ();
    iterator                            end                                ();
    void                                SetSize                            (size_t                  size);
    void                                SetEnd                             (iterator                newEndIt);
    };


/*---------------------------------------------------------------------------------**//**
* @description  Packet proxy for accessing "Plain Old Data" (POD) packets. 
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class PODPacketProxy : public NonConstPacketProxyBase<T>
    {
    static_assert(is_pod<T>::value, "T should be plain old data type!");

    friend class                        RawPacket;

public:
    typedef NonConstPacketProxyBase<T>::iterator iterator;
    explicit                            PODPacketProxy                     (Packet&                 packet);
    explicit                            PODPacketProxy                     ();

    void                                AssignTo                           (Packet&                 packet);

    void                                SetSize                            (size_t                  size);
    void                                SetEnd                             (typename NonConstPacketProxyBase<T>::iterator                newEndIt);

    typename ConstPacketProxyBase<T>::value_type*                         Edit                               () { return this->begin(); }
    };

/*---------------------------------------------------------------------------------**//**
* @description  Packet proxy for accessing class packets.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ClassPacketProxy : public NonConstPacketProxyBase<T>
    { 
    friend class                        RawPacket;
    friend class                        PODPacketProxy<T>;

public:
    explicit                            ClassPacketProxy                   (Packet&                 packet);
    explicit                            ClassPacketProxy                   ();

    void                                AssignTo                           (Packet&                 packet);

    };


/*---------------------------------------------------------------------------------**//**
* @description  Packet proxy for non mutating access to packets.
* @bsiclass                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class ConstPacketProxy : public ConstPacketProxyBase<T>
    {
    friend class                        RawPacket;
    friend class                        ClassPacketProxy<T>;

public:
    explicit                            ConstPacketProxy                   (const Packet&           packet);
    explicit                            ConstPacketProxy                   ();

    void                                AssignTo                           (const Packet&           packet);

    };





#include <ScalableMesh/Memory/PacketAccess.hpp>

END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
