/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_DWGDB_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgDbProgressMeter : public DWGDB_EXTENDCLASS2(HostAppProgressMeter, HostApplicationProgressMeter)
    {
private:
    IDwgDbProgressMeter*    m_appMeter;

public:
    DwgDbProgressMeter () : m_appMeter(nullptr) {}
    DwgDbProgressMeter (IDwgDbProgressMeter* meter) : m_appMeter(meter) {}

    IDwgDbProgressMeter*    GetApplicationProgressMeter () { return m_appMeter; }

#if defined (DWGTOOLKIT_OpenDwg)
    virtual void        start (const OdString& displayString = OdString()) override;
#elif defined (DWGTOOLKIT_RealDwg)
    virtual void        start (const ACHAR* displayString = NULL) override;
#endif
    virtual void        stop () override;
    virtual void        meterProgress () override;
    virtual void        setLimit (int max) override;
    };  // DwgDbProgressMeter

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          01/16
+===============+===============+===============+===============+===============+======*/
class DwgToolkitHost
#ifdef DWGTOOLKIT_OpenDwg
    : public ExHostAppServices, public ExSystemServices
    {
    DEFINE_T_SUPER (ExHostAppServices)
protected:
    ODRX_USING_HEAP_OPERATORS(ExSystemServices);

public:
    DwgToolkitHost() : ExSystemServices(), ExHostAppServices()
    
#elif DWGTOOLKIT_RealDwg

    : public AcDbHostApplicationServices
    {
    DEFINE_T_SUPER (AcDbHostApplicationServices)
public:
    DwgToolkitHost() : AcDbHostApplicationServices (1)

#endif  // DWGTOOLKIT
        {
        Initialize ();
        }

    ~DwgToolkitHost ();
    void                            SetApplicationHost (IDwgDbHost& appHost);   // app host
    DwgDbProgressMeter*             NewWorkingProgressMeter (IDwgDbProgressMeter* appMeter);    // working progress meter
    // same as getRemoteFile
    DwgDbStatus                     DownloadOrGetCachedFile (WStringR local, WStringCR url, bool ignoreCache = false) const;
    // inverse lookup as in isRemoteFile
    bool                            FindCachedLocalFile (WStringR cached, WStringCR url) const;
    // DwgDb warnings passed to the application host
    void                            Warn (WStringCR message) const;
    static DwgToolkitHost&          GetHost ();     // toolkit host

#if defined (DWGTOOLKIT_OpenDwg)
    virtual OdString               findFile (const OdString& filename, OdDbBaseDatabase* pDb = 0, FindFileHint hint = kDefault) override;
    virtual OdString               getErrorDescription (unsigned int errorCode);
    virtual void                   warning (const OdString& message) override;
    virtual void                   warning (const char* warnVisGroup, OdWarning warningOb) override;
    virtual void                   warning (const char* warnVisGroup, OdWarning warningOb, OdDbObjectId objectId) override;
    virtual void                   warning (OdWarning code) override;

    virtual OdDbHostAppProgressMeter*       newProgressMeter () override;

#elif defined (DWGTOOLKIT_RealDwg)
    virtual Acad::ErrorStatus      findFile (ACHAR* fileOut, int nChars, const ACHAR* fileIn, AcDbDatabase* dwg = nullptr, AcDbHostApplicationServices::FindFileHint hint = kDefault) override;
    virtual void                   fatalError (const ACHAR* format, ...) override;
    virtual void                   alert (const ACHAR* message) const override;
    virtual void                   displayChar (ACHAR c) const override;
    virtual void                   displayString (const ACHAR* chars, int count) const override;
    virtual Adesk::Boolean         readyToDisplayMessages () override;
    virtual const ACHAR*           program () override;
    virtual const ACHAR*           product () override;
    virtual const ProdIdCode       prodcode () override;
    virtual const ACHAR*           getMachineRegistryProductRootKey () override;
    virtual const ACHAR*           getUserRegistryProductRootKey () override;
#if VendorVersion <= 2016
    virtual LCID                   getRegistryProductLCID () override;
#else
    virtual AcLocale               getProductLocale () override;
#endif
    virtual bool                   notifyCorruptDrawingFoundOnOpen (AcDbObjectId id, Acad::ErrorStatus es) override;
    virtual Acad::ErrorStatus      getRoamableRootFolder (const wchar_t*& folder) override;
    virtual Acad::ErrorStatus      getLocalRootFolder (const wchar_t*& folder) override;
    virtual Adesk::Boolean         isRemoteFile (const ACHAR* localFile, ACHAR* url, size_t urlLen) const override;
    virtual Acad::ErrorStatus      getRemoteFile (const ACHAR* url, ACHAR* localFile, size_t localLen, Adesk::Boolean ignoreCache = Adesk::kFalse) const override;
    virtual const ACHAR*           getEnv (const ACHAR* var) override;
    virtual Adesk::Boolean         isURL (const wchar_t* pszURL) const override;
    virtual const wchar_t*         getAlternateFontName () const override;
    virtual bool                   getPassword(const ACHAR* dwgName, PasswordOptions options, wchar_t* password, const size_t bufSize) override;
#ifdef _MSC_VER
    virtual Acad::ErrorStatus      getNewOleClientItem (COleClientItem*& pItem) override;
    virtual Acad::ErrorStatus      serializeOleItem (COleClientItem* pItem, CArchive*) override;
#endif
    virtual AcDbHostApplicationProgressMeter* newProgressMeter () override;

#endif  // DWGTOOLKIT_

private:
    typedef bmap<WString,WString>   T_LocalToUrlMap;

    mutable T_LocalToUrlMap         m_localToUrlMap;
    mutable WString                 m_alternateFontName;
    WCharP                          m_toolkitRegistryRootKey;
    int                             m_progressLimit;
    int                             m_progressPosition;
    IDwgDbHost*                     m_appHost;
    DwgDbProgressMeter*             m_workingProgressMeter;

    void                            Initialize ();
    };

END_DWGDB_NAMESPACE

