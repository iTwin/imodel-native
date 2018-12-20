/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Memory/PacketAccess.hpp $
|    $RCSfile: PacketAccess.hpp,v $
|   $Revision: 1.5 $
|       $Date: 2011/09/07 14:20:38 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class RawPacket : public Uncopyable, public ShareableObjectTypeTrait<RawPacket>::type
    {
    template <typename T> struct        ProxyTypeTrait;

    class                               ObjectsLifeCycle;
    class                               PODObjectsLifeCycle;
    template <typename T> class         ClassObjectsLifeCycle;
    template <typename T> class         NotOwnerObjectLifeCycle;

    template <typename T> class         LifeCycleTrait;
    template <typename ProxyT> struct   PrepareForAssignTrait;

    friend struct                       Packet;
    
    explicit                            RawPacket                      (const MemoryAllocator&  memoryAllocator);
                                        ~RawPacket                     ();

    std::auto_ptr<const MemoryAllocator>     
                                        m_pAllocator;
    std::auto_ptr<ObjectsLifeCycle>     m_pObjLifeCycle;

public:
    void*                               m_pBufferBegin;
    void*                               m_pBufferEnd;
    void*                               m_pEnd;

    template <typename T>
    void                                PrepareForEdition              (bool                    readOnly);
    template <typename T>
    void                                PrepareForReadOnly             (bool                    readOnly) const;

    template <typename T>
    void                                Wrap                           (const T*                dataP,
                                                                        size_t                  size,
                                                                        size_t                  capacity);

    template <typename ProxyType>
    void                                PrepareForAssignTo             (ProxyType&              proxy,
                                                                        bool                    readOnly) const;
    template <typename ProxyType>
    void                                PrepareForAssignTo             (ProxyType&              proxy,
                                                                        bool                    readOnly);

    MEMORY_DLLE size_t                  GetCapacity                    () const;
    MEMORY_DLLE void                    Reserve                        (size_t                  newCapacity);

    MEMORY_DLLE void                    Clear                          ();

    size_t                              GetSize                        () const
                                        { return (const byte*)m_pEnd - (const byte*)m_pBufferBegin; }


    MEMORY_DLLE bool                    IsPODLifeCycleCompatibleWith   (size_t                  typeSize) const;

    void                                ChangeLifeCycle                (ObjectsLifeCycle*       lifeCycle)
        {
        assert(0 == GetSize());
        m_pObjLifeCycle = std::auto_ptr<ObjectsLifeCycle>(lifeCycle);
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> struct RawPacket::ProxyTypeTrait                      
    {
    template <bool IS_POD> struct   Impl                                {typedef ClassPacketProxy<T>        type;};
    template <> struct              Impl<true>                          {typedef PODPacketProxy<T>          type;};
    typedef typename Impl<is_pod<T>::value>::type                                                           type;
    };
template <typename T> struct RawPacket::ProxyTypeTrait<const T*>        {/*Fail... Do no support pointers*/};
template <typename T> struct RawPacket::ProxyTypeTrait<T*>              {/*Fail... Do no support pointers*/};
template <typename T> struct RawPacket::ProxyTypeTrait<const T>         {typedef ConstPacketProxy<T>        type;};


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class RawPacket::ObjectsLifeCycle
    {
public:
    typedef const std::type_info*       ClassID;

    size_t                              m_typeSize;
    ClassID                             m_classID;

    explicit                            ObjectsLifeCycle                   (size_t              typeSize,
                                                                            ClassID             classID);
    virtual                             ~ObjectsLifeCycle                  () = 0 {}

    virtual void                        Construct                          (void*               begin,
                                                                            void*&              end,
                                                                            size_t              count) const  = 0;

    virtual void                        ConstructFrom                      (void*               begin,
                                                                            void*&              end,
                                                                            const void*         srcBegin,
                                                                            const void*         srcEnd) const = 0;

    virtual void                        Destroy                            (void*               begin,
                                                                            void*&              end) const = 0;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class RawPacket::PODObjectsLifeCycle : public ObjectsLifeCycle
    {
public:
    static ClassID                      s_GetClassID                       ();
    MEMORY_DLLE explicit                PODObjectsLifeCycle                (size_t              typeSize);

    virtual void                        Construct                          (void*               begin,
                                                                            void*&              end,
                                                                            size_t              count) const override;

    virtual void                        ConstructFrom                      (void*               begin,
                                                                            void*&              end,
                                                                            const void*         srcBegin,
                                                                            const void*         srcEnd) const override;

    virtual void                        Destroy                            (void*               begin,
                                                                            void*&              end) const override;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class RawPacket::ClassObjectsLifeCycle : public ObjectsLifeCycle
    {
    struct UniqueTokenType {};
public:
    static ClassID                      s_GetClassID                       ();
    explicit                            ClassObjectsLifeCycle              (size_t              typeSize);

    virtual void                        Construct                          (void*               begin,
                                                                            void*&              end,
                                                                            size_t              count) const override;

    virtual void                        ConstructFrom                      (void*               begin,
                                                                            void*&              end,
                                                                            const void*         srcBegin,
                                                                            const void*         srcEnd) const override;

    virtual void                        Destroy                            (void*               begin,
                                                                            void*&              end) const override;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> class RawPacket::LifeCycleTrait
    {
    template <bool IS_POD> struct   Impl                                {typedef ClassObjectsLifeCycle<T>   type;};
    template <> struct              Impl<true>                          {typedef PODObjectsLifeCycle        type;};

public:
    typedef typename Impl<is_pod<T>::value>::type                                                           type;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
class RawPacket::NotOwnerObjectLifeCycle : public ObjectsLifeCycle
    {
public:
    static ClassID                      s_GetClassID                       ()
        {
        typedef LifeCycleTrait<T>::type ClassIDProviderLifeCycle;
        return ClassIDProviderLifeCycle::s_GetClassID();
        }

    explicit                            NotOwnerObjectLifeCycle            (size_t              typeSize)
        :   ObjectsLifeCycle(pi_typeSize, s_GetClassID()) 
                                        {}

    virtual void                        Construct                          (void*               begin,
                                                                            void*&              end,
                                                                            size_t              count) const override 
                                        { assert(!"Should never be called"); throw ReadOnlyPacketException(); }

    virtual void                        ConstructFrom                      (void*               begin,
                                                                            void*&              end,
                                                                            const void*         srcBegin,
                                                                            const void*         srcEnd) const override 
                                        { assert(!"Should never be called"); throw ReadOnlyPacketException(); }

    virtual void                        Destroy                            (void*               begin,
                                                                            void*&              end) const override 
                                        { /*Do nothing*/ }
    };



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProxyT> struct RawPacket::PrepareForAssignTrait
    {
    static void Prepare (RawPacket& pio_rPacket, ProxyT& po_rProxy, bool pi_readOnly)
        {
        pio_rPacket.PrepareForEdition<typename ProxyT::value_type>(pi_readOnly);
        }
    };
template <typename T> struct RawPacket::PrepareForAssignTrait <ConstPacketProxy<T>>
    {
    static void Prepare (RawPacket& pio_rPacket, ConstPacketProxy<T>& po_rProxy, bool pi_readOnly)
        {
        pio_rPacket.PrepareForReadOnly<T>();
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void RawPacket::PrepareForEdition (bool pi_readOnly)
    {
    typedef typename LifeCycleTrait<T>::type LifeCycle;

    if (pi_readOnly)
        throw ReadOnlyPacketException();

    if (0 == m_pObjLifeCycle->m_typeSize)
        {
        ChangeLifeCycle(new LifeCycle(sizeof(T)));
        }
    else 
        {
        if (m_pObjLifeCycle->m_classID != LifeCycle::s_GetClassID())
            throw BadPacketCastException();
        else if (m_pObjLifeCycle->m_classID == PODObjectsLifeCycle::s_GetClassID())
            {
            if (!IsPODLifeCycleCompatibleWith(sizeof(T)))
                throw BadPacketCastException();
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void RawPacket::PrepareForReadOnly (bool pi_readOnly) const
    {
    typedef typename LifeCycleTrait<T>::type LifeCycle;

    if (0 == m_pObjLifeCycle->m_typeSize)
        throw UndefinedPacketCastException();
    else 
        {
        if (m_pObjLifeCycle->m_classID != LifeCycle::s_GetClassID())
            throw BadPacketCastException();
        else if (m_pObjLifeCycle->m_classID == PODObjectsLifeCycle::s_GetClassID() && !IsPODLifeCycleCompatibleWith(sizeof(T)))
            throw BadPacketCastException();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProxyType>
void RawPacket::PrepareForAssignTo (ProxyType&  pi_rProxy,
                                    bool        pi_readOnly) const
    {
    PrepareForReadOnly<typename ProxyType::value_type>(pi_readOnly);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProxyType>
void RawPacket::PrepareForAssignTo (ProxyType&  pi_rProxy,
                                    bool        pi_readOnly)
    {
    PrepareForAssignTrait<ProxyType>::Prepare(*this, pi_rProxy, pi_readOnly);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void RawPacket::Wrap   (const T*    pi_pData,
                        size_t      pi_size,
                        size_t      pi_capacity)
    {
    m_pBufferBegin = pi_pData;
    m_pBufferEnd = (pi_pData + pi_capacity);
    m_pEnd = (pi_pData + pi_size);

    ChangeLifeCycle(new NotOwnerObjectLifeCycle(sizeof(T)));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline RawPacket::ObjectsLifeCycle::ClassID RawPacket::PODObjectsLifeCycle::s_GetClassID ()
    {
    return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
RawPacket::ObjectsLifeCycle::ClassID RawPacket::ClassObjectsLifeCycle<T>::s_GetClassID ()
    {
    static ClassID CLASS_ID = &typeid(UniqueTokenType());
    return CLASS_ID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
RawPacket::ClassObjectsLifeCycle<T>::ClassObjectsLifeCycle (size_t pi_typeSize)
    :   ObjectsLifeCycle(pi_typeSize, s_GetClassID())
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void RawPacket::ClassObjectsLifeCycle<T>::Construct    (void*   pi_begin,
                                                        void*&  pio_end,
                                                        size_t  pi_count) const
    {
    assert(pi_begin == pio_end);

    T*& pEnd = (T*&)pio_end;

    for (; 0 < pi_count; --pi_count)
        ::new ((void*)pEnd++) T();

    assert(distance((T*)pi_begin, pEnd) == pi_count);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void RawPacket::ClassObjectsLifeCycle<T>::ConstructFrom    (void*       pi_begin,
                                                            void*&      pio_end,
                                                            const void* pi_srcBegin,
                                                            const void* pi_srcEnd) const
    {
    assert(pi_begin == pio_end);

    T*& pEnd = (T*&)pio_end;

    const T* pSrcObj = (const T*)pi_srcBegin;
    const T* pSrcEnd = (const T*)pi_srcEnd;

    for (; pSrcObj != pSrcEnd; ++pSrcObj)
        ::new ((void*)pEnd++) T(*pSrcObj);

    assert(distance((T*)pi_begin, pEnd) == distance(pSrcObj, pSrcEnd));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
virtual void RawPacket::ClassObjectsLifeCycle<T>::Destroy  (void*   pi_begin,
                                                            void*&  pio_end) const
    {
    T* pBegin = (T*)pi_begin;
    T*& pEnd = (T*&)pio_end;

    while (pEnd > pBegin)
        (--pEnd)->~T();

    assert(pEnd == pBegin);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline PacketProxyBase::PacketProxyBase (RawPacket&  pi_rImpl)
    : m_pImpl(&pi_rImpl) 
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
inline PacketProxyBase::PacketProxyBase ()
    :   m_pImpl(0)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProxyT>
const RawPacket& PacketProxyBase::PrepareRawPacketFor  (const Packet&   packet,
                                                        ProxyT&         proxy)
    {
    const RawPacket& rawPacket = packet.GetRawPacket();
    rawPacket.PrepareForAssignTo(proxy, packet.IsReadOnly());

    return rawPacket;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ProxyT>
RawPacket& PacketProxyBase::PrepareRawPacketFor    (Packet& packet,
                                                    ProxyT& proxy)
    {
    RawPacket& rawPacket = packet.GetRawPacket();
    rawPacket.PrepareForAssignTo(proxy, packet.IsReadOnly());

    return rawPacket;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ConstPacketProxyBase<T>::ConstPacketProxyBase ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ConstPacketProxyBase<T>::ConstPacketProxyBase (const RawPacket& pi_rImpl) 
    :   PacketProxyBase(const_cast<RawPacket&>(pi_rImpl)) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void ConstPacketProxyBase<T>::SetImpl (const RawPacket& pi_rImpl)
    {
    assert(0 == m_pImpl);
    m_pImpl = const_cast<RawPacket*>(&pi_rImpl);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
const RawPacket& ConstPacketProxyBase<T>::GetImpl () const
    {
    assert(0 != m_pImpl);
    return *m_pImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
size_t ConstPacketProxyBase<T>::GetCapacity () const
    {
    return distance((const T*)GetImpl().m_pBufferBegin, (const T*)GetImpl().m_pBufferEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
size_t ConstPacketProxyBase<T>::GetSize () const
    {
    return distance(begin(), end());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
typename ConstPacketProxyBase<T>::const_iterator ConstPacketProxyBase<T>::cbegin () const
    {
    return (const T*)(GetImpl().m_pBufferBegin);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
typename ConstPacketProxyBase<T>::const_iterator ConstPacketProxyBase<T>::cend () const
    {
    return (const T*)(GetImpl().m_pEnd);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
NonConstPacketProxyBase<T>::NonConstPacketProxyBase (RawPacket&  pi_rImpl)
    :   ConstPacketProxyBase(pi_rImpl)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
NonConstPacketProxyBase<T>::NonConstPacketProxyBase ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void NonConstPacketProxyBase<T>::SetImpl (RawPacket& pi_rImpl)
    {
    assert(0 == m_pImpl);
    m_pImpl = &pi_rImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
RawPacket& NonConstPacketProxyBase<T>::GetImpl ()
    {
    assert(0 != m_pImpl);
    return *m_pImpl;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
typename NonConstPacketProxyBase<T>::iterator NonConstPacketProxyBase<T>::begin ()
    {
    return (T*)(GetImpl().m_pBufferBegin);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
typename NonConstPacketProxyBase<T>::iterator NonConstPacketProxyBase<T>::end ()
    {
    return (T*)(GetImpl().m_pEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void NonConstPacketProxyBase<T>::push_back (const T& rhs)
    {
    assert(GetImpl().m_pEnd < GetImpl().m_pBufferEnd);

    ::new (GetImpl().m_pEnd) T(rhs);
    ((T*&)GetImpl().m_pEnd)++;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void NonConstPacketProxyBase<T>::Clear ()
    {
    GetImpl().Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ConstPacketProxy<T>::ConstPacketProxy (const Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ConstPacketProxy<T>::ConstPacketProxy ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void ConstPacketProxy<T>::AssignTo (const Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ClassPacketProxy<T>::ClassPacketProxy (Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
ClassPacketProxy<T>::ClassPacketProxy ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void ClassPacketProxy<T>::AssignTo (Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
PODPacketProxy<T>::PODPacketProxy (Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
PODPacketProxy<T>::PODPacketProxy ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void PODPacketProxy<T>::AssignTo (Packet& packet)
    {
    SetImpl(PrepareRawPacketFor(packet, *this));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void PODPacketProxy<T>::SetSize (size_t                  pi_Size)
    {
    SetEnd(((iterator)GetImpl().m_pBufferBegin) + pi_Size);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
void PODPacketProxy<T>::SetEnd (iterator pi_newEndIt)
    {
    assert(pi_newEndIt >= GetImpl().m_pBufferBegin && pi_newEndIt <= GetImpl().m_pBufferEnd);
    GetImpl().m_pEnd = pi_newEndIt;
    }