/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BePointCloud/PointCloudChannelHandler.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

typedef bvector<PointCloudChannelHandlerP>  PointCloudChannelHandlers;
typedef bvector<PointCloudChannelP>         PointCloudChannelVector;
typedef PointCloudChannelVector const&      PointCloudChannelVectorCR;

#define PointCloudChannels_Is_Point_Visible(bValue)     ( (bValue &  0x07) != 0)  //if one of the first 3 bits On
#define PointCloudChannels_Is_Point_Hidden(bValue)      (!PointCloudChannels_Is_Point_Visible (bValue))
#define PointCloudChannels_Hide_Point(bValue)           (bValue  &= ~0x07)   //turn off first 3 bits
#define PointCloudChannels_Show_Point(bValue)           (bValue  |= 0x01)   //turn on 2 bit which is layer 1
#define PointCloudChannels_Is_Point_Selected(bValue)    (((bValue) & IPointCloudDataQuery::POINT_SELECTED) != 0)
#define PointCloudChannels_Select_Point(bValue)         (bValue  |= IPointCloudDataQuery::POINT_SELECTED)
#define PointCloudChannels_Unselect_Point(bValue)       (bValue  &= ~IPointCloudDataQuery::POINT_SELECTED)

struct IPointCloudDataQuery;


/*=================================================================================**//**
* \addtogroup Pointcloud
*/
//@{

/*---------------------------------------------------------------------------------**//**
* PointCloudChannelId
+---------------+---------------+---------------+---------------+---------------+------*/
enum class PointCloudChannelId
    {
    Rgb            = (1<<0),
    Xyz            = (1<<1),
    Intensity      = (1<<2),
    Normal         = (1<<3),
    Filter         = (1<<4),
    Classification = (1<<5),
    };


const WString VIEWSETTINGS_CHANNELHANDLER_KEY = L"ViewSettings_Key";
const WString CLIP_CHANNELHANDLER_KEY = L"Clip_Key";

/*---------------------------------------------------------------------------------**//**
* Derive from this class to handle your IPointCloudChannel. You can also register this handler
* with the PointCloudChannelHandlerManager to handle any point channel if you don't require to
* save any per point additional data.
* dat
* @see IPointCloudChannel::SetHandler
* @see PointCloudChannelHandlerManager::RegisterHandler
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudChannelHandler
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * @return    PointCloudChannelId that the channel handler requires
        * Default implementation specifies that the channel handler needs the Xyz channel
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT virtual uint32_t _GetRequiredChannels () const {return ((uint32_t)PointCloudChannelId::Xyz);}
        
        /*---------------------------------------------------------------------------------**//**
        * @return    PointCloudChannelId that the channel handler modifies
        * Default implementation specifies that the channel handler doesn't modify any point channels
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT virtual uint32_t _GetModifiedChannels () const {return 0;}
    };


#if defined (POINTCLOUD_WIP_GR06_ElementHandle)

/*---------------------------------------------------------------------------------**//**
* Interface class to provide an query channel handler implementation to a derived 
* class of both IPointCloudChannelQueryHandler and PointCloudChannelHandler
* This interface will allow you to change the point buffers being returned in queries.
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudChannelQueryHandler
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * @param query  IN              The query being executed
        * @param eh  IN                 Pointcloud element handle on which the query is executed
        * @param pPointCloudChannel IN  NULL or point channel buffer of the PointChannel to which this handler was set to.
        *                               If you didn't assign this handler to a PointChannel, this parameter will be NULL
        * @param pPointCloudBuffers  IN The buffers containing the different channels
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _OnQuery (IPointCloudDataQueryCR query, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) = 0;
        
        
        /*---------------------------------------------------------------------------------**//**
        * @return    A displayable name to identify this handler. This name could be shown in a dialog so that
        *            tools or users can filter the handlers and prevent some from being called. If an empty name is returned,
        *            then this query handler will never be visible in tools and UI
        * @see IPointCloudChannelHandlerFilter
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual WString _GetDisplayName () const  {return L"";};


        /*---------------------------------------------------------------------------------**//**
        * @return       A key identifier for this handler. This key should not be localized and may be
        *               used to filter out this channel handler
        * @see IPointCloudChannelHandlerFilter
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual WString _GetKey () const = 0;

        /*---------------------------------------------------------------------------------**//**
        * @return    true if wants to handle the query on this element
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _CanHandle (IPointCloudDataQueryCR query, ElementHandleCR eh) {return true;}
    };

/*---------------------------------------------------------------------------------**//**
* Interface class to provide a display channel handler implementation to a derived 
* class of both IPointCloudChannelDisplayHandler and PointCloudChannelHandler
* This interface will allow you to change the display of points. The points in the geometry buffer are
* in native POD coordinates, not in UOR. Use the inverse transform of ViewContext::GetCurrLocalToFrustumTrans
* to transform your geometry to match the points' geometry. It is not allowed to change the number of points in the given buffers.
* To change the visibility of a point, change the hidden state bit in pPointCloudBuffers.GetSelectionBuffer ().
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudChannelDisplayHandler
    {
    public:
        /*---------------------------------------------------------------------------------**//**
        * @return    True if pointcloud classification was handled
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _OnPreProcessClassification (ViewContextR viewContext, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) {return false;}

        /*---------------------------------------------------------------------------------**//**
        * @param viewContext  IN        The viewcontext in which the points are to be drawn
        * @param eh  IN                 Pointcloud element handle to be drawn
        * @param pPointCloudChannel IN  NULL or point channel buffer of the PointChannel to which this handler was set to.
        *                               If you didn't assign this handler to a PointChannel, this parameter will be NULL
        * @param pPointCloudBuffers  IN The buffers containing the different channels
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual void _OnDisplay(ViewContextR viewContext, ElementHandleCR eh, IPointCloudChannelP pPointCloudChannel, IPointCloudQueryBuffersR pPointCloudBuffers) = 0;

        /*---------------------------------------------------------------------------------**//**
        * @return    true if wants to handle the display of this element in this view context
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        virtual bool _CanHandle (ViewContextR viewContext, ElementHandleCR eh) {return true;}

    };

#endif // POINTCLOUD_WIP_GR06_ElementHandle


/*---------------------------------------------------------------------------------**//**
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IChannelsFileBuffer : public IRefCounted
    {
    protected:
        virtual uint64_t _GetSize () const    = 0;
        virtual uint8_t* _GetBuffer () const  = 0;

    public:
        BEPOINTCLOUD_EXPORT static IChannelsFileBufferPtr  CreateChannelsFileBuffer (IPointCloudChannelPtrMapCR channels);
        BEPOINTCLOUD_EXPORT uint64_t GetSize () const;
        BEPOINTCLOUD_EXPORT uint8_t* GetBuffer () const;
    };

/*---------------------------------------------------------------------------------**//**
* PointCloud Channel Handler Manager
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudChannelHandlerManager
    {
    private:
        PointCloudChannelHandlers m_handlers;

        PointCloudChannelHandlerManager(PointCloudChannelHandlerManagerCR);             // disabled
        PointCloudChannelHandlerManagerR operator=(PointCloudChannelHandlerManagerCR);  // disabled

    public:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        PointCloudChannelHandlerManager();
        ~PointCloudChannelHandlerManager();

        BEPOINTCLOUD_EXPORT PointCloudChannelHandlers const& GetHandlers() const;

        BEPOINTCLOUD_EXPORT static    PointCloudChannelHandlerManagerR GetManager ();
        BEPOINTCLOUD_EXPORT           StatusInt              RegisterHandler(PointCloudChannelHandlerP handler);
        BEPOINTCLOUD_EXPORT           StatusInt              UnregisterHandler(PointCloudChannelHandlerP handler);
    };




/*---------------------------------------------------------------------------------**//**
* points channel enabling storage of arbitrary per point  data. The point channel data is not persistent, the channel should be
* saved to disk using the SaveToFile and LoadFromFile methods of IPointCloudChannelManager
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudChannel : public IRefCounted
    {
    protected:
        virtual WStringCR               _GetName () const  = 0;
        virtual WStringCR               _GetPersistentName () const  = 0;
        virtual WStringCR               _GetInfo () const  = 0;
        virtual WStringCR               _GetCreatorTaskId () const  = 0;
        virtual unsigned int            _GetTypeSize () const = 0;
        virtual unsigned int            _GetMultiple () const  = 0;
        virtual void*                   _GetDefaultValue () const = 0;
        virtual bool                    _IsOutOfCore () const  = 0;
        virtual void                        _SetHandler (PointCloudChannelHandlerP handler) = 0;
        virtual PointCloudChannelHandlerP  _GetHandler () const  = 0;

    public:
        /*---------------------------------------------------------------------------------**//**
        * @return    The unique name for the channel. 
        * @return    The unique name used for the persistence of the channel. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT WStringCR GetPersistentName () const;

        /*---------------------------------------------------------------------------------**//**
        * @return    The name for the channel supplied when it was created. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT WStringCR GetName () const;

        /*---------------------------------------------------------------------------------**//**
        * @return    The user info for the channel supplied when it was created. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT WStringCR GetInfo () const;

        /*---------------------------------------------------------------------------------**//**
        * @return    The TaskId of the application that created the channel. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT WStringCR GetCreatorTaskId () const;

        /*---------------------------------------------------------------------------------**//**
        * @return    The size of the per point value data type being stored in bytes. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT unsigned int GetTypeSize () const;

        /*---------------------------------------------------------------------------------**//**
        * @return    The number of values per point. 
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT unsigned int GetMultiple () const;
        /*---------------------------------------------------------------------------------**//**
        * @return    The default value specified as a void pointer to a buffer containing the values.  
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT void* GetDefaultValue () const;
        
        /*---------------------------------------------------------------------------------**//**
        * @return    True if the data is stored out-of-core.  
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT bool  IsOutOfCore () const;
        
        /*---------------------------------------------------------------------------------**//**
        * @param handler IN Pointer to a derived implementation of PointCloudChannelHandler  
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT  void SetHandler (PointCloudChannelHandlerP handler);

        /*---------------------------------------------------------------------------------**//**
        * @return Pointer to a derived implementation of PointCloudChannelHandler  
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT  PointCloudChannelHandlerP  GetHandler () const;
    };

#define MAX_CHANNELDATASTRINGLENGTH 998


#if defined (POINTCLOUD_WIP_GR06_ElementHandle)

/*---------------------------------------------------------------------------------**//**
* Point Channel Manager
* @bsiinterface
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudChannelManager
    {
    private:
        IPointCloudChannelPtrMap LoadChannels (const uint32_t* channelHandles, int32_t numChannels);

        typedef bmap<WString, DgnElementP> MapWStringElementRef;
        typedef bmap<WString, DgnPlatform::PersistentElementPath> MapWStringPep;

        IPointCloudChannelPtrMap  m_channels;
        MapWStringElementRef     m_embeddedChannels;
        //MapWStringPep            m_embeddedChannels;

    public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    static BEPOINTCLOUD_EXPORT PointCloudChannelManager& GetManager ();

    /*---------------------------------------------------------------------------------**//**
    * Creates a points channel enabling storage of arbitrary per point  data. 
    * @param name IN A name for the channel.
    * @param userInfo IN NULL or A userInfo for the channel. Use this to store user data as a string
    * @param typesize IN The size of the per point value data type being stored in bytes.
    * @param multiple IN The number of values per point.
    * @param defValue IN The default value specified as a void pointer to a buffer containing the values. 
                         If there are multiple values per point the buffer must contain all these values and be of size typesize x multiple.
    * @param outofCore IN Set to true to store the data out-of-core. This will reduce memory overhead significantly at the cost of access performance.
    * @return    Reference counted pointer to an IPointCloudChannel instance. NULL if an error occured in the creation, such as a a non-unique name or name and userInfo too long
    * @note     Combined length of name and userInfo must not be greater than MAX_CHANNELDATASTRINGLENGTH
    * @see  MAX_CHANNELDATASTRINGLENGTH
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudChannelPtr   CreateChannel (WStringCR name, WStringCP userInfo, unsigned int typesize, unsigned int multiple, void* defValue, bool outofCore);

    /*---------------------------------------------------------------------------------**//**
    * Frees resources associated with the channel.  
    * @param channelP IN Reference counted pointer to an IPointCloudChannel instance.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT void   RemoveChannel (IPointCloudChannelPtr channelP);

    /*---------------------------------------------------------------------------------**//**
    * Gets the map of all channels.
    * @return    Map of reference counted pointers to IPointCloudChannel instances.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudChannelPtrMapCR  GetChannels ();
    
    /*---------------------------------------------------------------------------------**//**
    * Loads and adds channels from a file. Once the channels are modified, the channels file will need to be rewritten.
    * @param filename IN Full path of the channels file to open.
    * @return    Map of reference counted pointers to IPointCloudChannel instances that were just loaded so that the handler can be set.
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudChannelPtrMap    LoadFromFile (WStringCR filename);

    /*---------------------------------------------------------------------------------**//**
    * Saves the channels into a channels file.
    * @param channels IN Map of reference counted pointers to IPointCloudChannel instances  to save
    * @param filename IN Full path of the channels file to create.
    * @return    SUCCESS or ERROR
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT StatusInt  SaveToFile (IPointCloudChannelPtrMapCR channels, WStringCR filename);

    /*---------------------------------------------------------------------------------**//**
    * Change the default location of the out-of-core folder
    * @param path IN Full folder path.
    * @return    SUCCESS or ERROR
    * @bsimethod
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT StatusInt  SetOutofCoreFolder (WStringCR path);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT StatusInt LoadEmbeddedChannelsByName (IPointCloudChannelPtrMap& channels, WStringCR name);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT StatusInt FindChannelsByName (IPointCloudChannelPtrMap& channels, WStringCR channelName);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT bool IsEmbeddedChannelRegistered(DgnElementP host);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudChannelPtr CreateChannelCopy (IPointCloudChannelPtr existingChannel, WStringCR nameName, WStringCR userInfo);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IChannelsFileBufferPtr CreateChannelsFileBuffer (IPointCloudChannelPtr channels);

    /*---------------------------------------------------------------------------------**//**
    +---------------+---------------+---------------+---------------+---------------+------*/
    BEPOINTCLOUD_EXPORT IPointCloudChannelPtrMap  LoadFromBuffer (void* buffer, uint64_t bufferSize);

    };

#endif

//@}

END_BENTLEY_BEPOINTCLOUD_NAMESPACE

