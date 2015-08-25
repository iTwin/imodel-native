/*--------------------------------------------------------------------------------------+
|
|     $Source: Memory/Packet.cpp $
|    $RCSfile: Packet.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/08/19 13:50:31 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Memory/Packet.h>
#include <ScalableMesh/Memory/PacketAccess.h>
#include <ScalableMesh/Memory/Allocation.h>

#include <ScalableMesh/Foundations/Algorithm.h>

using namespace std;

BEGIN_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE


namespace { // BEGIN unamed namespace

class PacketObserver;


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketObservee
    {
    friend class                        Packet::Impl;

    mutable std::vector<PacketObserver*>
                                        m_observers;

    explicit                            PacketObservee                 ();
                                        ~PacketObservee                ();

public:
    typedef const PacketObserver*             ObserverUnregisterKey;

    void                                NotifyOfRealloc                (size_t                  pi_newCapacity) const;


    template <typename NotifyEventFunctor>
    void                                GenericNotifyObservers         (NotifyEventFunctor      pi_event) const;

    ObserverUnregisterKey               Register                       (PacketObserver&               pi_rObserver) const;
    void                                Unregister                     (ObserverUnregisterKey   pi_unregisterKey) const;

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PacketObserver
    {
    friend class                        PacketObservee;

    void                                NotifyRealloc                  (size_t                  pi_newCapacity);
    void                                NotifyStopObserving            ();


    virtual void                        _NotifyRealloc                 (size_t                  pi_newCapacity) = 0;
    virtual void                        _NotifyStopObserving           () = 0;

protected:
    explicit                            PacketObserver                 ();
public:
    virtual                             ~PacketObserver                () = 0;
    };


} // END unamed namespace


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class Packet::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    friend struct                       Packet;



    explicit                            Impl                           (RawPacket*              pi_pRawPacket);
                                        ~Impl                          ();

    RawPacketPtr                        m_pRawPacket;
    std::auto_ptr<PacketObserver>       m_pObserver;
    PacketObservee                      m_observee;
    bool                                m_readOnly;

public:
    void                                ReleaseObservee                ();

    void                                SetObserver                    (PacketObserver*         pi_pObserver);

    void                                BindSameAs                     (Packet::Impl&           pi_rPacketImpl,
                                                                        bool                    pi_readOnly);

    void                                BindCapacityTo                 (Packet::Impl&           pi_rPacketImpl,
                                                                        size_t                  pi_ratioNumerator = 1,
                                                                        size_t                  pi_ratioDenominator = 1);

    void                                NotifyOfRealloc                (size_t                  pi_NewCapacity) const;

    void                                Reserve                        (size_t                  pi_NewCapacity);
    size_t                              GetCapacity                    () const;
    };


namespace { // BEGIN unamed namespace

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class CommonObserverBase : public PacketObserver
    {
    const PacketObservee&                     m_rObservee;

    virtual void                        _NotifyStopObserving           () override
        {
        m_rImpl.ReleaseObservee();
        }

protected:
    explicit                            CommonObserverBase             (Packet::Impl&           pi_rImpl,
                                                                        const PacketObservee&   pi_rObservee)
        :   m_rImpl(pi_rImpl),
            m_rObservee(pi_rObservee)
        {

        m_rObservee.Register(*this);
        }
    
    virtual                             ~CommonObserverBase            ()
        {
        m_rObservee.Unregister(this);
        }

    Packet::Impl&                       m_rImpl;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class BindObserver : public CommonObserverBase
    {

    double                              m_ratio;

    virtual void                        _NotifyRealloc                 (size_t                  pi_newCapacity) override
        {
        m_rImpl.Reserve(static_cast<size_t>(ceil(static_cast<double>(pi_newCapacity)*m_ratio)));
        }

public:
    explicit                            BindObserver                   (Packet::Impl&           pi_rImpl,
                                                                        const PacketObservee&   pi_rObservee,
                                                                        size_t                  pi_ratioNumerator,
                                                                        size_t                  pi_ratioDenominator) 
        :   CommonObserverBase(pi_rImpl, pi_rObservee),
            m_ratio(1.0)
        {
        assert(0 != pi_ratioDenominator);
        m_ratio = static_cast<double>(pi_ratioNumerator) / static_cast<double>(pi_ratioDenominator);
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class PassNotificationAlongObserver : public CommonObserverBase
    {
    virtual void                        _NotifyRealloc                 (size_t                  pi_newCapacity) override
        {
        m_rImpl.NotifyOfRealloc(pi_newCapacity);
        }

public:
    explicit                            PassNotificationAlongObserver  (Packet::Impl&           pi_rImpl,
                                                                        const PacketObservee&   pi_rObservee) 
        :   CommonObserverBase(pi_rImpl, pi_rObservee)
        {
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketObservee::PacketObservee ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketObservee::~PacketObservee ()
    {
    struct NotifyStopObserving
        {
        void operator () (PacketObserver* pi_pObserver) const
            { pi_pObserver->NotifyStopObserving(); }
        };

    GenericNotifyObservers (NotifyStopObserving());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketObservee::NotifyOfRealloc (size_t pi_newCapacity) const
    {
    struct Notify
        {
        const size_t m_newCapacity;
        explicit Notify (size_t pi_newCapacity) : m_newCapacity(pi_newCapacity) {}

        void operator () (PacketObserver* pi_pObserver) const
            { pi_pObserver->NotifyRealloc(m_newCapacity); }
        };

    GenericNotifyObservers (Notify(pi_newCapacity));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename NotifyEventFunctor>
inline void PacketObservee::GenericNotifyObservers (NotifyEventFunctor pi_event) const
    {
    for_each(m_observers.begin(), m_observers.end(), pi_event);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketObservee::ObserverUnregisterKey PacketObservee::Register (PacketObserver& pi_rObserver) const
    {
    m_observers.push_back(&pi_rObserver);
    return &pi_rObserver;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketObservee::Unregister (ObserverUnregisterKey   pi_unregisterKey) const
    {
    m_observers.erase(remove(m_observers.begin(), m_observers.end(), pi_unregisterKey));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketObserver::PacketObserver () 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketObserver::~PacketObserver () 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketObserver::NotifyRealloc (size_t pi_newCapacity)
    {
    _NotifyRealloc(pi_newCapacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketObserver::NotifyStopObserving ()
    {
    _NotifyStopObserving();
    }


} // END unamed namespace




/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::ReleaseObservee ()
    {
    m_pObserver.reset();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::SetObserver (PacketObserver* pi_pObserver)
    {
    m_pObserver = std::auto_ptr<PacketObserver>(pi_pObserver);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::BindSameAs  (Packet::Impl&   pi_rPacketImpl,
                                bool            pi_readOnly)
    {
    SetObserver(new PassNotificationAlongObserver(*this, pi_rPacketImpl.m_observee));
    const bool NeedNotifyRealloc = GetCapacity() < pi_rPacketImpl.GetCapacity();
    m_pRawPacket = pi_rPacketImpl.m_pRawPacket;
    m_readOnly = pi_readOnly;

    if (NeedNotifyRealloc)
        NotifyOfRealloc(GetCapacity());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::BindCapacityTo  (Packet::Impl&   pi_rPacketImpl,
                                    size_t          pi_ratioNumerator,
                                    size_t          pi_ratioDenominator)
    {
    assert(0 != pi_ratioDenominator);

    SetObserver(new BindObserver(*this, pi_rPacketImpl.m_observee, 
                                 pi_ratioNumerator, pi_ratioDenominator));

    if (GetCapacity() < pi_rPacketImpl.GetCapacity())
        Reserve(pi_rPacketImpl.GetCapacity());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::NotifyOfRealloc (size_t pi_NewCapacity) const
    {
    m_observee.NotifyOfRealloc(pi_NewCapacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Impl::Reserve (size_t pi_NewCapacity)
    {
    m_pRawPacket->Reserve(pi_NewCapacity);
    NotifyOfRealloc(m_pRawPacket->GetCapacity());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Packet::Impl::GetCapacity () const
    {
    return m_pRawPacket->GetCapacity();
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Impl::Impl (RawPacket*  pi_pRawPacket)
    :   m_pRawPacket(pi_pRawPacket),
        m_readOnly(false)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Impl::~Impl ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (size_t                  pi_capacity) 
    :   m_pImpl(new Impl(new RawPacket(DefaultMemoryAllocator())))
    {
    Reserve(pi_capacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet     (const MemoryAllocator&  pi_memoryAllocator,
                    size_t                  pi_capacity)
    :   m_pImpl(new Impl(new RawPacket(pi_memoryAllocator)))
    {
    Reserve(pi_capacity);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::~Packet ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet::Packet (const Packet& rhs)
    :   m_pImpl(rhs.m_pImpl)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet& Packet::operator= (const Packet& rhs)
    {
    m_pImpl = rhs.m_pImpl;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MemoryAllocator& Packet::GetAllocator () const
    {
    return *GetRawPacket().m_pAllocator;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Packet::GetSize () const
    {
    return m_pImpl->m_pRawPacket->GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool Packet::IsReadOnly () const
    {
    return m_pImpl->m_readOnly;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::SetReadOnly (bool pi_readOnly)
    {
    m_pImpl->m_readOnly = pi_readOnly;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Packet::GetCapacity () const
    {
    return GetRawPacket().GetCapacity();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Reserve  (size_t pi_NewCapacity)
    {
    if (IsReadOnly())
        throw ReadOnlyPacketException();

    m_pImpl->Reserve(pi_NewCapacity);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::Clear ()
    {
    if (IsReadOnly())
        throw ReadOnlyPacketException();

    GetRawPacket().Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::NotifyOfRealloc () const
    {
    m_pImpl->NotifyOfRealloc(GetCapacity());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindCapacityTo (const Packet&    pi_rPacket)
    {
    m_pImpl->BindCapacityTo(*pi_rPacket.m_pImpl);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindCapacityTo    (const Packet&       pi_rPacket,
                                size_t              pi_ratioNumerator,
                                size_t              pi_ratioDenominator)
    {
    m_pImpl->BindCapacityTo(*pi_rPacket.m_pImpl, pi_ratioNumerator, pi_ratioDenominator);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindReferToSameAs (const Packet& pi_rPacket)
    {
    m_pImpl->BindSameAs(*pi_rPacket.m_pImpl, true);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindReferToSameAs (Packet& pi_rPacket)
    {
    m_pImpl->BindSameAs(*pi_rPacket.m_pImpl, pi_rPacket.IsReadOnly());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindUseSameAs (const Packet& pi_rPacket)
    {
    m_pImpl->BindCapacityTo(*pi_rPacket.m_pImpl);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Packet::BindUseSameAs (Packet& pi_rPacket)
    {
    if (pi_rPacket.IsReadOnly())
        m_pImpl->BindCapacityTo(*pi_rPacket.m_pImpl);
    else
        m_pImpl->BindSameAs(*pi_rPacket.m_pImpl, false);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawPacket& Packet::GetRawPacket ()
    {
    return *m_pImpl->m_pRawPacket;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const RawPacket& Packet::GetRawPacket () const
    {
    return *m_pImpl->m_pRawPacket;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PacketGroup::PacketHolder : public Packet 
    {
    explicit                        PacketHolder                       (const MemoryAllocator&  pi_allocator,
                                                                        size_t                  pi_capacity = 0)
        :   Packet(pi_allocator, pi_capacity) {}
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct PacketGroup::Impl
    {
    explicit                        Impl                               (size_t                  pi_size,
                                                                        const MemoryAllocator&  pi_allocator)
        :   m_pAllocator(pi_allocator.Clone())
        {
        for (size_t i = 0; i < pi_size; ++i)
            {
            m_packets.push_back(PacketHolder(pi_allocator));
            }
        }

    explicit                        Impl                               (const RawCapacities&    packetCapacities,
                                                                        const MemoryAllocator&  allocator)
        :   m_pAllocator(allocator.Clone())
        {
        for (RawCapacities::const_iterator capacityIt = packetCapacities.begin(), capacitiesEnd = packetCapacities.end(); 
             capacityIt != capacitiesEnd; 
             ++capacityIt)
            {
            m_packets.push_back(PacketHolder(allocator, *capacityIt));
            }
        }

    typedef std::vector<PacketHolder>   PacketList;
    PacketList                          m_packets;
    std::auto_ptr<MemoryAllocator>      m_pAllocator;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::PacketGroup   (size_t size)
    :   m_implP(new Impl(size, DefaultMemoryAllocator()))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::PacketGroup   (size_t                  size,
                            const MemoryAllocator&  allocator)
    :   m_implP(new Impl(size, allocator))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::PacketGroup   (const RawCapacities&    packetCapacities)
    :   m_implP(new Impl(packetCapacities, DefaultMemoryAllocator()))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::PacketGroup   (const RawCapacities&    packetCapacities,
                            const MemoryAllocator&  allocator)
    :   m_implP(new Impl(packetCapacities, allocator))
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::~PacketGroup ()
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::PacketGroup   (Impl* implP)
    :   m_implP(implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const MemoryAllocator& PacketGroup::GetAllocator () const
    {
    return *m_implP->m_pAllocator;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketGroup::Clear ()
    {
    for_each(begin(), end(), mem_fun_ref(&Packet::Clear));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void PacketGroup::Reserve (const RawCapacities& pi_NewPacketCapacities)
    {
    assert(GetSize() == pi_NewPacketCapacities.GetSize());
    copy(pi_NewPacketCapacities.begin(), 
         pi_NewPacketCapacities.begin() + (std::min)(pi_NewPacketCapacities.GetSize(), GetSize()), 
         setterIter(m_implP->m_packets.begin(), &Packet::Reserve));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Packet& PacketGroup::operator[] (size_t pi_Idx)         
    { 
    assert(pi_Idx < m_implP->m_packets.size()); 
    return m_implP->m_packets[pi_Idx]; 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Packet& PacketGroup::operator[] (size_t pi_Idx) const   
    { 
    assert(pi_Idx < m_implP->m_packets.size()); 
    return m_implP->m_packets[pi_Idx]; 
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::const_iterator PacketGroup::begin () const 
    { 
    return &(*m_implP->m_packets.begin()); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::const_iterator PacketGroup::end () const 
    { 
    return &(*m_implP->m_packets.end()); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::iterator PacketGroup::begin () 
    { 
    return &(*m_implP->m_packets.begin()); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PacketGroup::iterator PacketGroup::end () 
    { 
    return &(*m_implP->m_packets.end()); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PacketGroup::GetSize () const 
    { 
    return m_implP->m_packets.size(); 
    }



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct RawCapacities::Impl
    {
    std::vector<size_t>                 m_Sizes;
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities (size_t s0)
    :   m_implP(new Impl)
    {
    m_implP->m_Sizes.push_back(s0);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities (size_t s0, size_t s1)
    :   m_implP(new Impl)
    {
    m_implP->m_Sizes.push_back(s0);
    m_implP->m_Sizes.push_back(s1);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities (size_t s0, size_t s1, size_t s2)
    :   m_implP(new Impl)
    {
    m_implP->m_Sizes.push_back(s0);
    m_implP->m_Sizes.push_back(s1);
    m_implP->m_Sizes.push_back(s2);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities (size_t s0, size_t s1, size_t s2,size_t s3)
    :   m_implP(new Impl)
    {
    m_implP->m_Sizes.push_back(s0);
    m_implP->m_Sizes.push_back(s1);
    m_implP->m_Sizes.push_back(s2);
    m_implP->m_Sizes.push_back(s3);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::RawCapacities (const RawCapacities& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities& RawCapacities::operator= (const RawCapacities& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::~RawCapacities ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RawCapacities::GetSize () const 
    {
    return m_implP->m_Sizes.size();
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const size_t& RawCapacities::operator[] (size_t      pi_Idx) const   
    {
    assert(pi_Idx < m_implP->m_Sizes.size()); 
    return m_implP->m_Sizes[pi_Idx];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::const_iterator RawCapacities::begin () const 
    {
    return &(*m_implP->m_Sizes.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
RawCapacities::const_iterator RawCapacities::end () const 
    {
    return &(*m_implP->m_Sizes.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void RawCapacities::push_back (size_t capacity)
    {
    m_implP->m_Sizes.push_back(capacity);
    }


END_BENTLEY_SCALABLEMESH_MEMORY_NAMESPACE
