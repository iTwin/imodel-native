/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/ImageppLib.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>

#include <ImagePP/all/h/ImageppLib.h>

#include <ImagePP/all/h/HFCRasterGeoCoordinateServices.h>

#include <ImagePP/all/h/HFCStat.h>

#define RUNONCE_CHECK(var) {if (var) return; var=true;}
#define TERMINATE_HOST_OBJECT(obj, isProgramExit) {if (obj) {obj->OnHostTermination(isProgramExit); obj = NULL;}}

/* BEIJING_WIP_THREADS __declspec(thread) */ ImageppLib::Host*        t_ImageppLibHost;
HFCExclusiveKey  s_Key;

struct ImagePPHostCriticalSection
    {
    ImagePPHostCriticalSection  () {s_Key.ClaimKey();}
    ~ImagePPHostCriticalSection () {s_Key.ReleaseKey();}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ImagePPHost::GetKeyIndex (ImagePPHost::Key& key)
    {
    static size_t s_highestKey = 0; // MT: DgnCoreCriticalSection

    BeAssert (key.m_key >= 0 && key.m_key<=s_highestKey); // make sure we're given a valid key

    if (0 == key.m_key)
        {
        ImagePPHostCriticalSection __Serialize__;

        if (0 == key.m_key)
            key.m_key = ++s_highestKey;
        }

    return key.m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPHost::VarEntry& ImagePPHost::GetVarEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex (key);

    if (m_hostVar.size() < keyIndex+1)
        m_hostVar.resize(keyIndex+1, VarEntry());

    return  m_hostVar[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPHost::ObjEntry& ImagePPHost::GetObjEntry(Key& key)
    {
    size_t keyIndex = GetKeyIndex (key);

    if (m_hostObj.size() < keyIndex+1)
        m_hostObj.resize(keyIndex+1, ObjEntry());

    return  m_hostObj[keyIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePPHost::IHostObject* ImagePPHost::GetHostObject(Key& key)
    {
    return  GetObjEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImagePPHost::SetHostObject (Key& key, IHostObject* val)
    {
    GetObjEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void* ImagePPHost::GetHostVariable (Key& key)
    {
    return  GetVarEntry(key).m_val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImagePPHost::SetHostVariable (Key& key, void* val)
    {
    GetVarEntry(key).m_val = val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageppLibAdmin* ImageppLibAdmin::Create()
    {
    return new ImageppLibAdmin ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageppLibAdmin::ImageppLibAdmin()
:m_initializationComplete(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageppLibAdmin::_CompleteInitialization () 
    {
    RUNONCE_CHECK (m_initializationComplete)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageppLibAdmin::_Initialize() 
    {
    //Will complete initialization on first call
    CompleteInitialization();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageppLibAdmin::_Terminate() 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageppLibAdmin::Initialize()              { return _Initialize();}
void ImageppLibAdmin::Terminate()               { return _Terminate(); }
void ImageppLibAdmin::CompleteInitialization()  { return _CompleteInitialization(); }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageppLibAdmin& ImageppLib::Host::_SupplyImageppLibAdmin()
    {
    return *ImageppLibAdmin::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageppLib::Host::Initialize()
    {
    assert (NULL == m_imageppLibAdmin);   
    m_imageppLibAdmin = &_SupplyImageppLibAdmin();

    GetImageppLibAdmin().Initialize();

    _RegisterFileFormat();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageppLib::Host::Terminate(bool onProgramExit)
    {
    ImagePPHostCriticalSection __Serialize__;

    if (NULL == t_ImageppLibHost)
        return;

    for(bvector<ObjEntry>::iterator itr=m_hostObj.begin(); itr!=m_hostObj.end(); ++itr)
        {
        IHostObject* pValue(itr->GetValue());
        TERMINATE_HOST_OBJECT(pValue, onProgramExit);
        }

    m_hostObj.clear();
    m_hostVar.clear();

    GetImageppLibAdmin().Terminate();

    TERMINATE_HOST_OBJECT(m_imageppLibAdmin, onProgramExit);

    t_ImageppLibHost = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     staticInitialize ()
    {
    static bool s_onetime = false;

    if(!s_onetime)
        {
        //These HFCStat are always present
        //Other specific implementation will be registered by
        //corresponding creator, e.g.:ECWP, ECWPS and InternetImaging file.
        HFCStatHttpFile::Register();
        HFCStatFile::Register();
        HFCStatMemFile::Register();
        HFCStatEmbedFile::Register();
        s_onetime = true;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ImageppLib::Initialize(ImageppLib::Host& host)
    {
    ImagePPHostCriticalSection __Serialize__;

    BeAssert (NULL == t_ImageppLibHost);  // cannot be called twice on the same thread
    if (NULL != t_ImageppLibHost)
        return;

    staticInitialize ();

    t_ImageppLibHost = &host;
    t_ImageppLibHost->Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ImageppLib::IsInitialized ()
    {
    ImagePPHostCriticalSection __Serialize__;

    return NULL != t_ImageppLibHost;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ImageppLib::Host& ImageppLib::GetHost() 
    {
    ImagePPHostCriticalSection __Serialize__;

    BeAssert (IsInitialized() && L"ImageppLib library needs to be initialize!");  
    return *t_ImageppLibHost;
    }


/*---------------------------------------------------------------------------------**//**
//! This method call basegeocoord.dll to implement default IRasterGeoCoordinateServices.
//! Host can call this method in its ImageppLibAdmin::_GetIRasterGeoCoordinateServicesImpl override method
//! provided that basegeocoord.dll is delivered by application and initialized by it.
* @bsimethod                                    Marc.Bedard                     06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterGeoCoordinateServices* ImageppLib::GetDefaultIRasterGeoCoordinateServicesImpl()
    {
    return HFCRasterGeoCoordinateServices::GetInstance();
    }


