/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/ImageppLib.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/NonCopyableClass.h>
#include <ImagePP/h/HmrMacro.h>


//=======================================================================================
// This is a macro you can use to declare and implement a default ImagePPLibHost when
// no special processing is required.
// You must also include <ImagePP\all\h\HRFFileFormats.h> where you call the macro
// @bsiclass                                                    Marc.Bedard   10/2012
//=======================================================================================
#define IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(pi_ClassName)                 \
struct pi_ClassName : BentleyApi::ImagePP::ImageppLib::Host             \
{                                                                       \
    virtual void _RegisterFileFormat() override                         \
        {                                                               \
        REGISTER_SUPPORTED_FILEFORMAT                                   \
        }                                                               \
};                                      


BEGIN_IMAGEPP_NAMESPACE

//=======================================================================================
// An ImagePPHost is an object that uniquely identifies a usage of the ImagePP libraries for a single purpose. 
// For a given process, there can be more than one ImagePPHost, but each ImagePPHost must be on a different thread.
// ImagePPHost holds a collection of key/pointer pairs that are used to store and retrieve host-based data.
// @bsiclass                                                    Marc.Bedard   10/2012
//=======================================================================================
struct ImagePPHost : NonCopyableClass
    {
public:
    ImagePPHost()           {}
    virtual ~ImagePPHost()  {}

    //! Each "type" of data stored on a ImagePPHost must have a unique key to identify it. To add data to a ImagePPHost, create a single *static*
    //! instance of this class and pass it to the SetHostVariable method. The same static instance should be used to set and retrieve your data from 
    //! the ImagePPHost.
    struct Key
        {
        friend struct ImagePPHost;
        private:
            size_t m_key;
        public:
            Key() {m_key=0;}
        };

    struct IHostObject : NonCopyableClass
        {
    protected:
        virtual ~IHostObject(){}
        virtual void _OnHostTermination (bool isProgramExit) = 0;
    public:
        void OnHostTermination (bool isProgramExit) {_OnHostTermination(isProgramExit);}
        };

    struct HostObjectBase : IHostObject {virtual void _OnHostTermination (bool) override {delete this;}};

    template<typename T>
    struct HostValue : HostObjectBase
        {
        private:
        T   m_value;
        public:
        HostValue (T const& v) : m_value (v) {;}
        T const& GetValue () const {return m_value;}
        void     SetValue (T const& v) {m_value = v;}
        };

//__PUBLISH_SECTION_END__
private:
    size_t GetKeyIndex (ImagePPHost::Key& key);

//__PUBLISH_SCOPE_1_START__
protected:
    struct VarEntry
        {
        friend struct ImagePPHost;
        void* m_val;
        VarEntry() {m_val=0;}
        void* GetValue() {return m_val;}
        void  SetValue(void* val) {m_val = val;}
        };

    struct ObjEntry
        {
        friend struct ImagePPHost;
        IHostObject* m_val;
        ObjEntry() {m_val=0;}
        IHostObject* GetValue() {return m_val;}
        void  SetValue(IHostObject* val) {m_val = val;}
        };

    bvector<VarEntry> m_hostVar;
    bvector<ObjEntry> m_hostObj;
    VarEntry& GetVarEntry(Key& key);
    ObjEntry& GetObjEntry(Key& key);

//__PUBLISH_SECTION_END__     // end of SCOPE_1
//__PUBLISH_SECTION_START__ 
public:

    //! Get the value of a host-based variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The value of the host variable identified by key. If the variable has never been set on this instance of ImagePPHost, 
    //! the value will be 0.
    IMAGEPP_EXPORT void* GetHostVariable (Key& key);

    //! Set the value of a host-based variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The value to be associated with key for this ImagePPHost.
    IMAGEPP_EXPORT void  SetHostVariable (Key& key, void* val);

    //! Get the value of a host-based integer variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The integer value of the host variable identified by key. If the variable has never been set on this instance of ImagePPHost, 
    //! the value will be 0.
    template <typename INT_TYPE>
    INT_TYPE  GetHostIntVariable (Key& key) {return (INT_TYPE)(intptr_t)GetHostVariable (key);}

    //! Set the value of a host-based integer variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The integer value to be associated with key for this ImagePPHost.
    template <typename INT_TYPE>
    void SetHostIntVariable (Key& key, INT_TYPE val) {SetHostVariable (key, (void*)(intptr_t)val);}

    //! Get the value of a host-based boolean variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The boolean value of the host variable identified by key. If the variable has never been set on this instance of ImagePPHost, 
    //! the value will be 0.
    bool  GetHostBoolVariable (Key& key) {return 0 != (intptr_t)GetHostVariable (key);}

    //! Set the value of a host-based boolean variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The boolean value to be associated with key for this ImagePPHost.
    void SetHostBoolVariable (Key& key, bool val) {SetHostVariable (key, (void*)(intptr_t)val);}

    //! Get the value of a host-based variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The value of the host variable identified by key. If the variable has never been set on this instance of ImagePPHost, 
    //! the value will be 0.
    IMAGEPP_EXPORT IHostObject* GetHostObject (Key& key);

    //! Set the value of a host-based variable identified by key. 
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The value to be associated with key for this ImagePPHost.
    IMAGEPP_EXPORT void  SetHostObject (Key& key, IHostObject* val);
    };


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ChannelToBandIndexMapping
    {
    ChannelToBandIndexMapping() {memset(m_index,0,sizeof(m_index));}
    enum ChannelType
        {
        RED     =0,
        GREEN   =1,
        BLUE    =2,
        ALPHA   =3
        };
    unsigned short GetIndex(ChannelType channel) const          { return m_index[channel];   }
    void   SetIndex(ChannelType channel, unsigned short value)  { m_index[channel] = value;  }
    bool   IsAlphaChannelDefined() const                { return m_index[ALPHA] != 0;}

    private:
        unsigned short m_index[4];
    };

enum CCITT_PHOTOMETRIC_INTERPRETATION
    {
    CCITT_PHOTOMETRIC_MINISWHITE = 0,   /* min value is white */
    CCITT_PHOTOMETRIC_MINISBLACK = 1    /* min value is black */
    };


/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ImageppLibAdmin : ImagePPHost::IHostObject
{
/*__PUBLISH_SECTION_END__*/
private:


    virtual int  _GetVersion() const {return 1;} // Do not override!
    virtual void _OnHostTermination (bool isProcessShutdown)  {delete this;}

    IMAGEPP_EXPORT virtual bool _Initialize();
    IMAGEPP_EXPORT virtual void _Terminate();

protected:
    mutable bool                        m_initializationComplete;
    mutable BeFileName                  m_localDirPath;               //Create on first use


    IMAGEPP_EXPORT ImageppLibAdmin();
    virtual ~ImageppLibAdmin(){}

    IMAGEPP_EXPORT virtual void  _CompleteInitialization();

    /*__PUBLISH_SECTION_START__*/
public:
    typedef bvector<WString>    CompatibleSoftware;

    IMAGEPP_EXPORT static ImageppLibAdmin* Create ();

    IMAGEPP_EXPORT void OnHostTermination (bool isProgramExit) {_OnHostTermination(isProgramExit);}
    bool Initialize();
    void Terminate();
    void CompleteInitialization();

    /*---------------------------------------------------------------------------------**//**
    * Gets the name of the generating software to use in cache file attributes, (the first entry)
    * If there are other entries, they will be use to detect cache compatibility
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _GetCacheCompatibleSoftwareNames(CompatibleSoftware& softwareName) const        {}

    /*---------------------------------------------------------------------------------**//**
    * Gets the description to use in cache file attributes
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void    _GetCacheDescription(WStringR description) const        {}


    /*---------------------------------------------------------------------------------**//**
    * TR 262120
    * In some cases, the users want to flip the data if the model is defined by 7 to 24 tiepoints.
    * This static method imposes a different interpretation for the model of ALL GeoTIFF
    * files with a model defined by (7 to 24)TiePoints.
    * 
    * In the previous version of MicroStation, before V8i, we added a flip in Y when the
    * model was defined by (7 to 24)TiePoints, now we don't do that by default.
    * 
    * By default, false (no Flip in Y)
    * return true if the model is stored with (7 to 24)TiePoints, we add a flip in Y
    * false: If the model is stored with (7 to 24)TiePoints, we don't add a flip in Y
    *
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _IsSetFlipInY_IfModelDefinedBy7to24TiePoints() const  {return false;}
    
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _IsIntergraphLUTColorCorrectionEnable() const        {return true;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual double  _GetHMRFileFactor() const                            {return 1.0;}

    /*---------------------------------------------------------------------------------**//**
    *Data path must be provided by host admin implementation
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _GetGDalDataPath(WStringR path) const           {return BSIERROR;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _GetECWDataPath(WStringR path) const            {return BSIERROR;}


    /*---------------------------------------------------------------------------------**//**
    *TR 244928, CR 247184
    *- Temporary patch to allow the user to specify the mapping between
    *  bands found in an image file and the color channels of the pixel type used
    *  to display the image.
    *Return true if channel mapping is defined    
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _GetChannelToBandIndexMapping(ChannelToBandIndexMapping& mapping) const {return false;}

    /*---------------------------------------------------------------------------------**//**
    * retrun true if Intergraph tags should be ignored when determining if the file is a
    * GeoTIFF image or an Intergraph TIFF image.
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _IsIgnoreGeotiffIntergraphTags() const                                  {return false;}

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool    _IsErMapperUseFeetInsteadofSurveyFeet() const                           {return false;}

    /*---------------------------------------------------------------------------------**//**
    * This method imposes a storage method for the model to change the standard
    * of ALL GeoTIFF files. Normally the Geotiff model should store with TiePoint(s), but
    * many application support only (1 TiePoint +/- scale, and the matrix), for this reason
    * we offer the possibility to override it.
    * This setting must not be changed while GeoTIFF files are opened
    * since result is then unpredictable.
    * This method don't override the StoreUsingModelTransformationTag method.
    * 
    * By default, true (no geotiff standard)
    * @param pi_LimitedTiePoints true : The model stored in the file will be, Matrix or
    * Translation(1Tiepoint), stretch(1TiePoint+Scale)
    * false: GeoTiff standard Multi Tiepoints
    * @bsimethod                                                            05/2012
    ----------------------------------------------------------------------------- */
    virtual bool    _IsGeoTiffFileStoreLimitedTiePointsModel() const                        {return true;}

    /*---------------------------------------------------------------------------------**//**
    * Retrieves the path of the directory for temporary files.
    *
    * Scans for environment variables in order:
    *  1. HMRTempDirectory and then BeGetTempPath
    *
    * If one these is found, the method returns true and sets the value of the tempFileName
    * to the value of the variable..
    * If none is found, the method returns false and the tempFileName is left intact.
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus  _GetDefaultTempDirectory(BeFileName& tempFileName) const
        {
        BentleyStatus Result = BSIERROR;

#ifdef _WIN32 
        // Try to get the HMR temp directory ("HMRTempDirectory")
        WCharP pDir = 0;
        if (pDir = _wgetenv (L"HMRTempDirectory"))
            {
            // Use default returned value.
            tempFileName = BeFileName(pDir);
            return BSISUCCESS;
            }
        if (pDir = _wgetenv (L"TEMP"))
            {
            // Use default returned value.
            tempFileName = BeFileName(pDir);
            return BSISUCCESS;
            }
#else
        BeAssert(!"_GetDefaultTempDirectory must be implemented by Imagepp Host in ImageppLibAdmin implementation.");
#endif

        return Result;
        }

    /*---------------------------------------------------------------------------------**//**
    * Return default directory path to store transaction files (undo/redo)
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _GetFileTransactionRecorderDirPath(BeFileName& tempPath) const   
        {
        // Try to get the HMR temp directory ("HMRTempDirectory")
        return _GetDefaultTempDirectory(tempPath);
        }

    /*---------------------------------------------------------------------------------**//**
    * Return default directory path to store image cache files 
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange=false) const   
        {
        //If not empty and it exist, return it 
        if (!checkForChange && !m_localDirPath.IsEmpty() && BeFileName::IsDirectory(m_localDirPath.GetName()))
            {
            tempPath = m_localDirPath;
            return BSISUCCESS;
            }

        // Try to get the HMR temp directory ("HMRTempDirectory")
        _GetDefaultTempDirectory(m_localDirPath);

        if (m_localDirPath.IsEmpty())
            return BSIERROR;

        //Create the path if it does not exist
        if (!BeFileName::IsDirectory(m_localDirPath.GetName()))
            {
            if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(m_localDirPath.GetName()))
                {
                return BSIERROR;
                }
            }
        tempPath = m_localDirPath;
        return BSISUCCESS;
        }

    /*---------------------------------------------------------------------------------**//**
    * HIP-small file caching settings
    * Threshold size to cache locally
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual size_t _GetInternetImagingHIPSmallFileThreshold() const   
        {
        return 0;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual BentleyStatus _GetInternetImagingHIPSmallFileLocation(BeFileName& tempFileName) const   
        {
        return _GetDefaultTempDirectory(tempFileName);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void _GetInternetImagingReconnectionSettings(uint32_t& retries, uint32_t& delay) const   
        {
        retries = 0;
        delay   = 0;
        }
    /*---------------------------------------------------------------------------------**//**
    // Time out to use when attempting to connect to a server
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual uint32_t _GetInternetImagingConnectionTimeOut() const   
        {
        return 30000;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual size_t _GetDirectoryCacheSize() const   
        {
        return 0; // 0 means no limit size
        }

    /*---------------------------------------------------------------------------------**//**
    * The number of tiles unavailable and useless tiles from previous request
    * that is allowed before canceling a previous request
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual size_t _GetLookAheadCancelThreshold() const   
        {
        return UINT_MAX; 
        }

    /*---------------------------------------------------------------------------------**//**
    * Forbid 1 bit multi-resolution cache file when keeping the cache
    * to ensure backward compatibility with older Imagepp software.
    * We don't create MultiRes cache by default to be back compatible,
    * old application are not able to open for example a CIT multi-res with
    * a cache multi-res.
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _Is1BitMultiresCacheSupportEnable() const   
        {
        return false; 
        }
    /*---------------------------------------------------------------------------------**//**
    * The default ratio to meter of a specific HRFTWFPageFile instance.
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual double _GetDefaultRatioToMeter() const   
        {
        return 1.0; 
        }

    /*---------------------------------------------------------------------------------**//**
    * IPP has always inverted cals compared to others softwares. So default is CCITT_PHOTOMETRIC_MINISBLACK to keep
    * old behavior but the "real" type should be CCITT_PHOTOMETRIC_MINISWHITE
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual CCITT_PHOTOMETRIC_INTERPRETATION _GetDefaultPhotometricInterpretation() const   
        {
        return BentleyApi::ImagePP::CCITT_PHOTOMETRIC_MINISBLACK; 
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            12/2014
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _IsSamplerMultiThreadingEnabled() const   
        {
        return true; 
        }
};

/*=================================================================================**//**
ImageppLib defines interfaces that a "host" application must implement to use Imagepp Libraries.
The host must initialize the ImageppLib library by calling ImageppLib::Initialize for each host on a separate thread.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class ImageppLib
{
public:

    //! The object that a host application must create and supply in order to use imagepp functions and methods  
    //! see also the macro IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST in ImageppLib.h.
    struct Host : ImagePPHost
        {
        protected:
            ImageppLibAdmin*            m_imageppLibAdmin;

            //! Supply the ImageppAdmin for this session. This method is guaranteed to be called once and never again.
            IMAGEPP_EXPORT virtual ImageppLibAdmin& _SupplyImageppLibAdmin();


            //! Register supported file format for this session. This method is guaranteed to be called once and never again.
            //! This method must be implemented by the host application 
            //! Implementation should look like this: 
            //! virtual void _RegisterFileFormat() override { REGISTER_SUPPORTED_FILEFORMAT }
            //! where REGISTER_SUPPORTED_FILEFORMAT is a macro in HRFFileFormats.h
            IMAGEPP_EXPORT virtual void             _RegisterFileFormat()=0;

        public:
            Host()
                {
                m_imageppLibAdmin = 0;
                }

            ImageppLibAdmin&            GetImageppLibAdmin()             {return *m_imageppLibAdmin;}

        //! Returns true if this Host has been initialized; otherwise, false
        bool IsInitialized () {return 0 != m_imageppLibAdmin;}

        void Initialize ();

        //! Terminate this Host. After this method is called, no further imagepp methods can ever be called on this thread again (including Initialize).
        //! This method should be called on thread termination.
        //! @param[in] onProgramExit Whether the entire program is exiting. If true, some cleanup operations can be skipped for faster program exits.
        IMAGEPP_EXPORT void Terminate(bool onProgramExit);
        };

    //! Must be called once per Host before calling any method in imagepp. Applications can have more than one Host. 
    IMAGEPP_EXPORT static void Initialize (Host& host);

    //! Query if the imagepp library has been initialized on this thread.
    IMAGEPP_EXPORT static bool IsInitialized ();

    //! Get the imagepp library host for this thread. 
    IMAGEPP_EXPORT static Host& GetHost();
};


END_IMAGEPP_NAMESPACE
