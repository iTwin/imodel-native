/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudQueryBuffer.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEPOINTCLOUD_TYPEDEFS(ChannelHandlerOnQueryCaller);
BEPOINTCLOUD_TYPEDEFS(ChannelHandlerOnDisplayCaller);
BEPOINTCLOUD_TYPEDEFS(UserChannelQueryBuffers);
BEPOINTCLOUD_TYPEDEFS(PointCloudQueryBuffers)

#define RANGECALC_QUERYCAPACITY (1048576)
#define PICK_QUERYCAPACITY      (1048576)

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

struct PointCloudChannel;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ExclusiveKey
{
    public:
                    ExclusiveKey()  { 
                                    // POINTCLOUD_WIP - is there any Graphite method that replaces that ?
                                    // InitializeCriticalSection(&m_cs); 
                                    }
       virtual     ~ExclusiveKey()  {
                                    // POINTCLOUD_WIP - is there any Graphite method that replaces that ?
                                    // DeleteCriticalSection(&m_cs);
                                    }

       void         Claim()         { m_cs.Enter(); }
       void         Release()       { m_cs.Leave();  }

    private:

        // Disabled methods
        ExclusiveKey(const ExclusiveKey&);
        ExclusiveKey& operator=(const ExclusiveKey&);

        mutable BeMutex m_cs;
};

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct Monitor
    {
    private:
        ExclusiveKey* m_pKey;

    public:
        Monitor(ExclusiveKey& key) : m_pKey(&key)   { m_pKey->Claim(); }
        ~Monitor()                                  
            {
            if (m_pKey != NULL)
                m_pKey->Release(); 
            }

        void Release()
            {
            if (m_pKey != NULL)
                {
                m_pKey->Release();
                m_pKey = NULL;
                }
            }
        void Assign(ExclusiveKey& key)
            {
            // Release the previous object
            if (m_pKey != NULL)
                m_pKey->Release();

            // Assign and claim
            m_pKey = &key;
            m_pKey->Claim();
            }

    private:
        Monitor(); // disabled
        Monitor& operator=(Monitor const&); // disabled
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> struct ChannelBufferPool
    {
    private:
        typedef bvector<T*> Buffers;
        Buffers         m_buffers;
        uint64_t        m_capacity; // Number of points
        ExclusiveKey    m_pointCloudBuffersExclusiveKey;

    private:
        // Non copyable
        ChannelBufferPool (ChannelBufferPool const&);
        ChannelBufferPool& operator= (ChannelBufferPool const&);

    public:
        ChannelBufferPool(uint64_t capacity) : m_capacity(capacity)
            {
            }

        ~ChannelBufferPool()
            {
            Monitor monitor(m_pointCloudBuffersExclusiveKey);

            for (typename Buffers::iterator itr(m_buffers.begin()); itr != m_buffers.end(); ++itr)
                delete(*itr);
            }

        uint64_t GetCapacity() const { return m_capacity; }
        uint64_t GetSize() const     { return m_buffers.size(); }

        T*      Allocate(uint32_t capacity)
            {
            Monitor monitor(m_pointCloudBuffersExclusiveKey);

            for (typename Buffers::iterator itr(m_buffers.begin()); itr != m_buffers.end(); ++itr)
                {
                if ((*itr)->GetCapacity() == capacity)
                    {
                    T* pBuf = *itr;
                    m_buffers.erase(itr);
                    return pBuf;
                    }
                }

            monitor.Release();
            return new T (capacity);
            }

        void    Deallocate(T* buffer)
            {
            if (0 == buffer->GetCapacity())
                {
                delete buffer;
                return;
                }

            Monitor monitor(m_pointCloudBuffersExclusiveKey);

            m_buffers.insert(m_buffers.begin(), buffer);

            while (m_buffers.size() > m_capacity && !m_buffers.empty())
                {
                delete(m_buffers.back());
                m_buffers.pop_back();
                }
            }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointChannelBuffer : public RefCountedBase
    {
    protected:
        virtual void* _Allocate(size_t size) = 0;
        virtual void  _Deallocate(Byte* p, size_t size) = 0;

    public:
        virtual void*  _GetBufferP() const = 0;
        virtual uint32_t _GetCapacity() const = 0;
        virtual uint32_t _GetNumPoints() const = 0;           // Number of points
        virtual void   _SetNumPoints(uint32_t count) = 0;     // Number of points
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointChannelBuffer : public NonCopyableClass, public IPointChannelBuffer
    {
    private:
        PointChannelBuffer(uint32_t capacity, uint32_t typeSize, uint32_t multiple);
        PointChannelBuffer();  // Disabled
        virtual ~PointChannelBuffer();

    protected:
        virtual void* _Allocate(size_t size) override;
        virtual void  _Deallocate(Byte* p, size_t size) override;

    public:
        virtual void*  _GetBufferP() const override;
        virtual uint32_t _GetCapacity() const override;           // Number of points
        virtual uint32_t _GetNumPoints() const override;          // Number of points
        virtual void   _SetNumPoints(uint32_t count) override;    // Number of points

        size_t GetTypeSize() const;
        uint32_t GetMultiple() const;
        size_t GetBufferSize() const; // Number of bytes


        Byte* GetValueP(uint32_t idx) const;
        void  SetValue(Byte* value, uint32_t idx);

        static PointChannelBuffer* Create(uint32_t capacity, uint32_t typeSize, uint32_t multiple);

    private:
        size_t m_typeSize;
        uint32_t m_multiple;
        uint32_t m_capacity;
        uint32_t m_count;
        Byte*  m_buffer;
    };


/*---------------------------------------------------------------------------------**//**
* ChannelBuffer template
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> struct ChannelBuffer
    {
    friend ChannelBufferPool<ChannelBuffer<T> >;

    public:
        typedef T  value_type;
        typedef T* pointer;

        typedef ChannelBufferPool<ChannelBuffer<T> > Pool;

    private:
        BeAtomic<uint32_t>    m_refCount;
        T*                  m_channel;
        uint32_t            m_size;
        uint32_t            m_capacity;

        static Pool         m_pool;

    private:
        // Non copyable
        ChannelBuffer (ChannelBuffer const&);
        ChannelBuffer& operator= (ChannelBuffer const&);

    private:
        ChannelBuffer() : m_refCount(0), m_channel(NULL), m_size(0), m_capacity(0) {}
        ChannelBuffer(uint32_t capacity) : m_refCount(0), m_channel(AllocateChannel(capacity)), m_size(0), m_capacity(capacity) {}
        ~ChannelBuffer() { DeallocateChannel(m_channel, m_capacity); }

        pointer AllocateChannel(size_t size)                 { return (size > 0) ? new T[size] : NULL; }
        void    DeallocateChannel(pointer p, size_t size)    { delete[] p; }

    public:
        unsigned long AddRef ()             {   
                                        return ++m_refCount; 
                                    }

        int32_t Release ()            { 
                                    uint32_t retval = --m_refCount;
                                    if (0 == retval)
                                        {
                                        //  Make the buffer available again
                                        m_pool.Deallocate (const_cast<ChannelBuffer<T>*>(this));
                                        }

                                    return retval;
                                    }


        T*      GetChannelBuffer()                  { return m_channel; }
        uint32_t GetSize() const                     { return m_size; }
        void    SetSize(uint32_t size)                { BeAssert(size <= m_capacity); m_size = size; }
        uint32_t GetCapacity() const                 { return m_capacity; }
        void    ValidateCapacity(uint32_t capacity)   { if (m_capacity < capacity) ChangeCapacity(capacity); }

        void    ChangeCapacity(uint32_t capacity)            
            {
            if (m_capacity != capacity)
                {
                m_size = 0;
                DeallocateChannel(m_channel, m_capacity);
                m_capacity = capacity;
                m_channel = AllocateChannel(capacity);
                }
            }

        void Init(T const& channelValue)
            {
            for (uint32_t i=0; i<m_size; ++i)
                m_channel[i] = channelValue;
            }

        void Copy (uint32_t srcIdx, uint32_t dstIndex)
            {
            m_channel[dstIndex] = m_channel[srcIdx];
            }

        static ChannelBuffer<T>* Create(uint32_t capacity) { return m_pool.Allocate(capacity); }
    };

typedef ChannelBuffer<PointCloudColorDef>   PointCloudRgbChannel;
typedef ChannelBuffer<DPoint3d>             PointCloudXyzChannel;
typedef ChannelBuffer<int16_t>              PointCloudIntensityChannel;
typedef ChannelBuffer<FPoint3d>             PointCloudNormalChannel;
typedef ChannelBuffer<unsigned char>        PointCloudFilterChannel;
typedef ChannelBuffer<unsigned char>        PointCloudClassificationChannel;
typedef ChannelBuffer<unsigned char>        PointCloudSymbologyChannel;
typedef ChannelBuffer<Byte>                 PointCloudByteChannel;

typedef ChannelBufferPool<PointCloudRgbChannel>         PointCloudRgbChannelPool;
typedef ChannelBufferPool<PointCloudXyzChannel>         PointCloudXyzChannelPool;
typedef ChannelBufferPool<PointCloudIntensityChannel>   PointCloudIntensityChannelPool;
typedef ChannelBufferPool<PointCloudNormalChannel>      PointCloudNormalChannelPool;
typedef ChannelBufferPool<PointCloudByteChannel>        PointCloudByteChannelPool;

typedef std::vector<std::pair<RefCountedPtr<IPointCloudSymbologyChannel>, RefCountedPtr<PointCloudSymbologyChannel> > > PointCloudSymbologyChannelVector;
typedef PointCloudSymbologyChannelVector const&                                                                         PointCloudSymbologyChannelVectorCR;

// typedef std::vector<RefCountedPtr<PointCloudChannel> >          PointCloudChannelVector;
// typedef PointCloudChannelVector const&                          PointCloudChannelVectorCR;

/*---------------------------------------------------------------------------------**//**
* PointCloudQueryBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudQueryBuffers : public NonCopyableClass, public IPointCloudQueryBuffers
    {
    public:

    private:
        RefCountedPtr<PointCloudRgbChannel>             m_rgb;
        RefCountedPtr<PointCloudXyzChannel>             m_xyz;
        RefCountedPtr<PointCloudIntensityChannel>       m_intensities;
        RefCountedPtr<PointCloudNormalChannel>          m_normals;
        RefCountedPtr<PointCloudFilterChannel>       m_filters;
        RefCountedPtr<PointCloudClassificationChannel>  m_classifications;
        uint32_t                                        m_numPoints;
        uint32_t                                        m_capacity;
        uint32_t                                        m_channelFlags;
        bool                                            m_rgbOverriden;

        PointCloudSymbologyChannelVector                m_symbologyChannels;
        RefCountedPtr<UserChannelQueryBuffers>          m_userChannelBuffers;



    protected:
        PointCloudQueryBuffers();
        PointCloudQueryBuffers(uint32_t capacity, uint32_t channelFlags, PointCloudChannelVectorCR channels);
        virtual ~PointCloudQueryBuffers();

        virtual DPoint3d*           _GetXyzBuffer() const override              { return m_xyz.get()             ? m_xyz->GetChannelBuffer() : NULL; }
        virtual PointCloudColorDefP _GetRgbBuffer() const override              { return m_rgb.get()             ? m_rgb->GetChannelBuffer() : NULL; }
        virtual int16_t*            _GetIntensityBuffer() const override        { return m_intensities.get()     ? m_intensities->GetChannelBuffer() : NULL; }
        virtual FPoint3d*           _GetNormalBuffer() const override           { return m_normals.get()         ? m_normals->GetChannelBuffer() : NULL; }
        virtual unsigned char*      _GetFilterBuffer() const override           { return m_filters.get()         ? m_filters->GetChannelBuffer() : NULL; }
        virtual unsigned char*      _GetClassificationBuffer() const override   { return m_classifications.get() ? m_classifications->GetChannelBuffer() : NULL; }
        virtual void*               _GetChannelBuffer(IPointCloudChannelP pChannel) const override;
        virtual uint32_t            _GetNumPoints() const override              { return m_numPoints; }
        virtual uint32_t            _GetCapacity() const override;

        virtual void                _SetNumPoints(uint32_t numPoints) override;

        virtual void                _AddChannel(PointCloudChannelId channelId) override;
        virtual unsigned char*      _CreatePointCloudSymbologyChannel(IPointCloudSymbologyChannelP symb) override;

    public:
        BEPOINTCLOUD_EXPORT bool HasXyz() const                                                     { return m_xyz.IsValid(); }
        BEPOINTCLOUD_EXPORT bool HasRgb() const                                                     { return m_rgb.IsValid(); }
        BEPOINTCLOUD_EXPORT bool HasIntensity() const                                               { return m_intensities.IsValid(); }
        BEPOINTCLOUD_EXPORT bool HasNormal() const                                                  { return m_normals.IsValid(); }
        BEPOINTCLOUD_EXPORT bool HasFilter() const                                                  { return m_filters.IsValid(); }
        BEPOINTCLOUD_EXPORT bool HasClassification() const                                          { return m_classifications.IsValid(); }

        BEPOINTCLOUD_EXPORT PointCloudXyzChannel* GetXyzChannel() const                             { return m_xyz.get(); }
        BEPOINTCLOUD_EXPORT PointCloudRgbChannel* GetRgbChannel() const                             { return m_rgb.get(); }
        BEPOINTCLOUD_EXPORT PointCloudIntensityChannel* GetIntensityChannel() const                 { return m_intensities.get(); }
        BEPOINTCLOUD_EXPORT PointCloudNormalChannel* GetNormalChannel() const                       { return m_normals.get(); }
        BEPOINTCLOUD_EXPORT PointCloudFilterChannel* GetFilterChannel() const                       { return m_filters.get(); }
        BEPOINTCLOUD_EXPORT PointCloudClassificationChannel* GetClassificationChannel() const       { return m_classifications.get(); }
       
        BEPOINTCLOUD_EXPORT void SetXyzChannel(PointCloudXyzChannel* channel)                       { m_xyz = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void SetRgbChannel(PointCloudRgbChannel* channel)                       { m_rgb = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void SetIntensityChannel(PointCloudIntensityChannel* channel)           { m_intensities = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void SetNormalChannel(PointCloudNormalChannel* channel)                 { m_normals = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void SetFilterChannel(PointCloudFilterChannel* channel)                 { m_filters = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void SetClassificationChannel(PointCloudClassificationChannel* channel) { m_classifications = channel; if (channel) channel->ChangeCapacity(m_capacity);}
        BEPOINTCLOUD_EXPORT void ChangeCapacity(uint32_t capacity);

        BEPOINTCLOUD_EXPORT PointCloudSymbologyChannelVectorCR  GetPointCloudSymbologyChannelVector() const { return m_symbologyChannels; }
        BEPOINTCLOUD_EXPORT void                                ClearSymbologyChannels()                    { m_symbologyChannels.clear(); }
        BEPOINTCLOUD_EXPORT void                                SetUserChannelBuffers(UserChannelQueryBuffers* buffers);
        BEPOINTCLOUD_EXPORT UserChannelQueryBuffers*            GetUserChannelBuffers()                     { return m_userChannelBuffers.get(); }

        BEPOINTCLOUD_EXPORT void                                RemoveHiddenPoints(bool updateFilterChannel = true);
        BEPOINTCLOUD_EXPORT uint32_t                            GetPoints(PThandle queryHandle);

        BEPOINTCLOUD_EXPORT void                                SetRgbOverrideColor (PointCloudColorDefCR overrideColor);

        BEPOINTCLOUD_EXPORT static PointCloudQueryBuffersPtr    Create(uint32_t capacity, uint32_t channelFlags);
        BEPOINTCLOUD_EXPORT static PointCloudQueryBuffersPtr    Create(uint32_t capacity, uint32_t channelFlags, PointCloudChannelVectorCR channels);
    };

/*---------------------------------------------------------------------------------**//**
* UserChannelQueryBuffers
+---------------+---------------+---------------+---------------+---------------+------*/
struct UserChannelQueryBuffers : public NonCopyableClass, public RefCountedBase
    {
    typedef std::pair<RefCountedPtr<PointCloudChannel>, RefCountedPtr<PointChannelBuffer> >   UserChannelBuffer;
    typedef bvector<UserChannelBuffer>                                                        UserChannelBuffers;
    typedef UserChannelBuffers const&                                                         UserChannelBuffersCR;

    private:
        PointCloudChannelVector m_channelVector;
        UserChannelBuffers       m_channelBuffers;
        uint32_t                 m_capacity;

        UserChannelQueryBuffers(uint32_t capacity);
        UserChannelQueryBuffers(uint32_t capacity, PointCloudChannelVectorCR userChannels);

    public:

        static UserChannelQueryBuffersPtr   Create(uint32_t capacity);
        static UserChannelQueryBuffersPtr   Create(uint32_t capacity, PointCloudChannelVectorCR userChannels);
    
        std::vector<void*>                  GetChannelBuffers() const;
        std::vector<PThandle>               GetChannelHandles() const;
        void*                               GetChannelBuffer(IPointCloudChannelP pChannel) const;
        void                                ChangeCapacity(uint32_t capacity);
        uint32_t                            GetCapacity() const;
        uint32_t                            GetNumberOfPoints() const;
        void                                SetNumberOfPoints(uint32_t numberOfPoints);
        void                                SetPointChannels(PointCloudChannelVectorCR channels);
        PointCloudChannelVectorCR           GetPointChannels();
        UserChannelBuffersCR                GetPointChannelBuffers();
    };

#if defined (POINTCLOUD_WIP_GR06_ElementHandle)

/*---------------------------------------------------------------------------------**//**
* CallChannelHandler
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChannelHandlerCaller : public RefCountedBase
    {
    ChannelHandlerCaller(ElementHandleCR eh);
    virtual ~ChannelHandlerCaller() {};

    protected:
        ElementHandleCR                     m_eh;
        PointCloudChannelVector             m_channels;
        PointCloudChannelHandlers           m_globalChannelHandlers;
        bool                                m_init;
        IPointCloudChannelHandlerFilterP    m_filterP;


        virtual bool _Handles(PointCloudChannelHandler* chHandler) const = 0;
        virtual void _CallAllHandlers(PointCloudQueryBuffersR pBuffers) = 0;
        virtual void _Initialize ();

    public:
        void   Call (PointCloudQueryBuffersR pBuffers) { _CallAllHandlers(pBuffers); }
        
        uint32_t CombineFlags (uint32_t channelFlags);
        void   AddModifyingChannelFlags (uint32_t& channelFlags);
        void   SetChannelHandlerFilter (IPointCloudChannelHandlerFilterP filterP);

        void GetChannelHandlers (PointCloudChannelHandlers& handlers);
        PointCloudChannelVectorCR GetChannelVector ()
            {
            if (!m_init)
                _Initialize();
            return m_channels;
            }
        PointCloudChannelHandlers const& GetGlobalChannelHandlers ()
            {
            if (!m_init)
                _Initialize();
            return m_globalChannelHandlers;
            }
    };


/*---------------------------------------------------------------------------------**//**
* CallChannelHandlersOnQuery
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChannelHandlerOnDisplayCaller : public ChannelHandlerCaller
    {
    DEFINE_T_SUPER(ChannelHandlerCaller)

    public:
        static ChannelHandlerOnDisplayCallerPtr Create(ElementHandleCR eh, ViewContextR context);

        bool  CallOnClassif (PointCloudQueryBuffersR pBuffers);


    protected:
        ViewContextR m_context;

        ChannelHandlerOnDisplayCaller(ElementHandleCR eh, ViewContextR context)
            :ChannelHandlerCaller (eh), m_context (context) {};
        virtual ~ChannelHandlerOnDisplayCaller() {};

        virtual void _CallAllHandlers (PointCloudQueryBuffersR pBuffers) override;
        virtual bool _Handles(PointCloudChannelHandler* handler) const override;

        virtual void _Initialize() override;

        bvector<IPointCloudChannelDisplayHandler*>  m_displayHandlers;

    };

/*---------------------------------------------------------------------------------**//**
* CallChannelHandlersOnQuery
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChannelHandlerOnQueryCaller : public ChannelHandlerCaller
    {
    DEFINE_T_SUPER(ChannelHandlerCaller)

    public:
        static ChannelHandlerOnQueryCallerPtr Create(ElementHandleCR eh, IPointCloudDataQueryCR query);

    protected:
        IPointCloudDataQueryCR m_query;

        ChannelHandlerOnQueryCaller(ElementHandleCR eh, IPointCloudDataQueryCR query)
            :ChannelHandlerCaller (eh), m_query(query) {};
        virtual ~ChannelHandlerOnQueryCaller() {};

        virtual void _CallAllHandlers (PointCloudQueryBuffersR pBuffers) override;
        virtual bool _Handles(PointCloudChannelHandler* handler) const override;

        virtual void _Initialize() override;

        bvector<IPointCloudChannelQueryHandler*>  m_queryHandlers;
    };

#endif


END_BENTLEY_BEPOINTCLOUD_NAMESPACE
