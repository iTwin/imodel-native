/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/RuleSetLocater.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Connection.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

ECPRESENTATION_TYPEDEFS(RuleSetLocater)
ECPRESENTATION_REFCOUNTED_PTR(RuleSetLocater)

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
//! An interface for a class that receives ruleset-related callbacks.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                12/2015
//=======================================================================================
struct IRulesetCallbacksHandler
    {
    //! Virtual destructor.
    virtual ~IRulesetCallbacksHandler() {}

    //! Called when a ruleset is created.
    //! @param[in] locater Locater that created the ruleset.
    //! @param[in] ruleset The ruleset that was created.
    virtual void _OnRulesetCreated(RuleSetLocaterCR locater, PresentationRuleSetR ruleset) {}

    //! Called when a ruleset is about to be disposed.
    //! @param[in] locater Locater that disposed the ruleset.
    //! @param[in] ruleset The ruleset that was created.
    virtual void _OnRulesetDispose(RuleSetLocaterCR locater, PresentationRuleSetCR ruleset) {}
    };

//=======================================================================================
//! Abstract base class for all ruleset locaters. A locater is responsible for finding
//! a ruleset given its ID.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct RuleSetLocater : IRefCounted
{
private:
    IRulesetCallbacksHandler* m_rulesetCallbacksHandler;
    mutable bvector<PresentationRuleSetPtr> m_createdRulesets;
    mutable BeMutex m_mutex;

protected:
    //! Called to find matching rulesets.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, the implementation should return all available rulesets.
    //! @return All matching rulesets.
    //! @note This method is protected with a mutex.
    virtual bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const = 0;

    //! Called to get all available ruleset IDs for this locater.
    //! @note This method is protected with a mutex.
    virtual bvector<Utf8String> _GetRuleSetIds() const = 0;

    //! Called to get the locater priority.
    virtual int _GetPriority() const = 0;

    //! @see InvalidateCache
    //! @note This method is protected with a mutex.
    virtual void _InvalidateCache(Utf8CP rulesetId) = 0;

    //! Called to get connection for which this locater is created. Returns nullptr if locater is designated for all connections.
    virtual IConnectionCP _GetDesignatedConnection() const {return nullptr;}

    //! Implementations should call this function before they dispose a ruleset.
    //! @note This method is protected with a mutex.
    ECPRESENTATION_EXPORT void OnRulesetDisposed(PresentationRuleSetR ruleset) const;

    //! Implementations should call this function when they create a new ruleset instance.
    //! @note This method is protected with a mutex.
    ECPRESENTATION_EXPORT void OnRulesetCreated(PresentationRuleSetR ruleset) const;

    //! A mutex that can be used to implement thread safety.
    BeMutex& GetMutex() const {return m_mutex;}

//__PUBLISH_SECTION_END__
public:
    ECPRESENTATION_EXPORT void SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler);

//__PUBLISH_SECTION_START__
public:
    //! Constructor.
    RuleSetLocater() : m_rulesetCallbacksHandler(nullptr) {}

    //! Destructor.
    virtual ~RuleSetLocater() {}

    //! Dispose a cached ruleset.
    //! @param[in] rulesetId ID of the ruleset which should be disposed from cache. NULL means all rulesets.
    ECPRESENTATION_EXPORT void InvalidateCache(Utf8CP rulesetId = nullptr);

    //! Find all matching rulesets.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, all available rulesets are returned.
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> LocateRuleSets(Utf8CP rulesetId = nullptr) const;

    //! Get IDs of all available rulesets.
    ECPRESENTATION_EXPORT bvector<Utf8String> GetRuleSetIds() const;

    //! Get the locater priority.
    ECPRESENTATION_EXPORT int GetPriority() const;

    //! Get connection for which this locater is designated. Returns nullptr if locater is designated for all connections.
    IConnectionCP GetDesignatedConnection() const {return _GetDesignatedConnection();}
};

//=======================================================================================
//! Ruleset locater that finds rulesets in the specified directories.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DirectoryRuleSetLocater : RefCounted<RuleSetLocater>
{
private:
    Utf8String m_directoryList;
    mutable bmap<BeFileName, PresentationRuleSetPtr> m_cache;

protected:
    DirectoryRuleSetLocater(Utf8CP directoryList) : m_directoryList(directoryList) {}
    //! Get a semicolon separated list of directories that contain rulesets.
    ECPRESENTATION_EXPORT virtual Utf8String _GetRuleSetDirectoryList() const;
    //! Get all directories to look for rulesets in.
    ECPRESENTATION_EXPORT virtual bvector<BeFileName> _GetRuleSetDirectories() const;
    //! Get all ruleset filenames found in the lookup directories.
    ECPRESENTATION_EXPORT virtual bvector<BeFileName> _GetRuleSetFileNames() const;

    ECPRESENTATION_EXPORT virtual bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
    ECPRESENTATION_EXPORT bvector<Utf8String> _GetRuleSetIds() const override;
    ECPRESENTATION_EXPORT void _InvalidateCache(Utf8CP rulesetId) override;
    int _GetPriority() const override {return 100;}

public:
    //! Create a new locater.
    //! @param[in] directoryList A semicolon separated list of directories that contain rulesets.
    static RefCountedPtr<DirectoryRuleSetLocater> Create(Utf8CP directoryList = nullptr) {return new DirectoryRuleSetLocater(directoryList);}
};

//=======================================================================================
//! Ruleset locater that finds rulesets in the specified file.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                12/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE FileRuleSetLocater : RefCounted<RuleSetLocater>
{
private:
    BeFileName m_path;
    mutable time_t m_cachedLastModifiedTime;
    mutable PresentationRuleSetPtr m_cached;

private:
    FileRuleSetLocater() {}
    FileRuleSetLocater(BeFileNameCR path) : m_path(path) {}

protected:
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
    ECPRESENTATION_EXPORT bvector<Utf8String> _GetRuleSetIds() const override;
    ECPRESENTATION_EXPORT void _InvalidateCache(Utf8CP rulesetId) override;
    int _GetPriority() const override {return 100;}

public:
    //! Create a new locater.
    //! @param[in] path Path to an XML file that contains ruleset definition.
    static RefCountedPtr<FileRuleSetLocater> Create(BeFileNameCR path) {return new FileRuleSetLocater(path);}

    //! Create a new locater.
    static RefCountedPtr<FileRuleSetLocater> Create() {return new FileRuleSetLocater();}

    //! Change the path of the file to look for ruleset definition in.
    //! @param[in] path Path to an XML file that contains ruleset definition.
    ECPRESENTATION_EXPORT void SetPath(BeFileNameCR path);
};

//=======================================================================================
//! Ruleset locater that wraps another locater and changes all its located supplemental
//! ruleset IDs to the lookup ruleset ID.
// @bsiclass                                   Aidas.Vaiksnoras                05/2017
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SupplementalRuleSetLocater : RefCounted<RuleSetLocater>
{
private:
    RuleSetLocaterCPtr m_locater;

private:
    SupplementalRuleSetLocater(RuleSetLocater const& locater) : m_locater(&locater) {}

protected:
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
    bvector<Utf8String> _GetRuleSetIds() const override {return m_locater->GetRuleSetIds();}
    void _InvalidateCache(Utf8CP rulesetId) override {const_cast<RuleSetLocater&>(*m_locater).InvalidateCache(rulesetId);}
    int _GetPriority() const override {return m_locater->GetPriority();}

public:
    //! Create a new locater.
    static RefCountedPtr<SupplementalRuleSetLocater> Create(RuleSetLocater const& locater) {return new SupplementalRuleSetLocater(locater);}
};

//=======================================================================================
//! Ruleset locater that wraps another locater and returns only non-supplemental rulesets
// @bsiclass                                   Haroldas.Vitunskas              11/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE NonSupplementalRuleSetLocater : RefCounted<RuleSetLocater>
    {
    private:
        RuleSetLocaterCPtr m_locater;

    private:
        NonSupplementalRuleSetLocater(RuleSetLocater const& locater) : m_locater(&locater) {}

    protected:
        ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
        bvector<Utf8String> _GetRuleSetIds() const override { return m_locater->GetRuleSetIds(); }
        void _InvalidateCache(Utf8CP rulesetId) override { const_cast<RuleSetLocater&>(*m_locater).InvalidateCache(rulesetId); }
        int _GetPriority() const override { return m_locater->GetPriority(); }

    public:
        //! Create a new locater.
        static RefCountedPtr<NonSupplementalRuleSetLocater> Create(RuleSetLocater const& locater) { return new NonSupplementalRuleSetLocater(locater); }
    };

//=======================================================================================
//! Ruleset locater that stores in memory rulesets
// @bsiclass                                   Aidas.Kilinskas                  05/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SimpleRuleSetLocater : RefCounted<RuleSetLocater>
    {
    private:
        SimpleRuleSetLocater() {}
        mutable bmap<Utf8String, PresentationRuleSetPtr> m_cached;

    protected:
        ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
        ECPRESENTATION_EXPORT bvector<Utf8String> _GetRuleSetIds() const override;
        void _InvalidateCache(Utf8CP rulesetId) override {}
        int _GetPriority() const override { return 100; }

    public:
        //! Create a new locater.
        static RefCountedPtr<SimpleRuleSetLocater> Create() { return new SimpleRuleSetLocater(); }
        ECPRESENTATION_EXPORT void AddRuleSet(PresentationRuleSetR presentationRuleSet);
        ECPRESENTATION_EXPORT void RemoveRuleSet(Utf8StringCR id);
        ECPRESENTATION_EXPORT void Clear();
    };

//=======================================================================================
//! Ruleset locater that finds rulesets in iModelDb
// @bsiclass                                    Haroldas.Vitunskas             10/2018
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE EmbeddedRuleSetLocater : RefCounted<RuleSetLocater>
    {
    typedef bpair<BeInt64Id, DateTime> RulesetInfo;
    typedef bpair<RulesetInfo, PresentationRuleSetPtr> RulesetRecord;
    typedef bpair<RulesetInfo, Utf8String> RulesetJsonRecord;

    private:
        IConnectionCR m_connection;
        mutable bvector<RulesetRecord> m_cache;

    private:
        EmbeddedRuleSetLocater(IConnectionCR);
        void LoadRuleSets() const;
        size_t QueryRuleSetCount();
        bvector<PresentationRuleSetPtr> GetCachedRuleSets() const;
        bvector<RulesetJsonRecord> QueryRulesets() const;
        RulesetJsonRecord QueryRulesetById(BeInt64Id id) const;

    protected:
        bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;
        bvector<Utf8String> _GetRuleSetIds() const override;
        void _InvalidateCache(Utf8CP rulesetId) override;
        int _GetPriority() const override { return 100; };
        IConnectionCP _GetDesignatedConnection() const override { return &m_connection; }

    public:
        //! Create a new locater.
        //! @param[in] connection Connection to iModelDb that contains embedded rulesets.
        ECPRESENTATION_EXPORT static RefCountedPtr<EmbeddedRuleSetLocater> Create(IConnectionCR connection) { return new EmbeddedRuleSetLocater(connection); }

        //! Invalidates cache if rulesets have changed in Db
        void InvalidateCacheIfNeeded();
    };

//=======================================================================================
//! An interface for ruleset locaters' manager which keeps all locaters and sends
//! ruleset requests based on ruleset locater priorities.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct IRulesetLocaterManager
{
private:
    IRulesetCallbacksHandler* m_rulesetCallbacksHandler;

protected:
    IRulesetLocaterManager() : m_rulesetCallbacksHandler(nullptr) {}
    virtual void _InvalidateCache(Utf8CP rulesetId) {}
    virtual void _RegisterLocater(RuleSetLocater& locater) = 0;
    virtual void _UnregisterLocater(RuleSetLocater const& locater) = 0;
    virtual bvector<PresentationRuleSetPtr> _LocateRuleSets(IConnectionCR, Utf8CP rulesetId) const = 0;
    virtual bvector<Utf8String> _GetRuleSetIds() const = 0;

public:
    virtual ~IRulesetLocaterManager() {}
    IRulesetCallbacksHandler* GetRulesetCallbacksHandler() const {return m_rulesetCallbacksHandler;}
    void SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler) {m_rulesetCallbacksHandler = handler;}

    //! Tells each managed ruleset locater to dispose its cached rulesets.
    //! @param[in] rulesetId ID of the ruleset which should be disposed from cache. NULL means all rulesets.
    void InvalidateCache(Utf8CP rulesetId = nullptr) {_InvalidateCache(rulesetId);}

    //! Register a new ruleset locater.
    void RegisterLocater(RuleSetLocater& locater) {_RegisterLocater(locater);}

    //! Unregister a locater.
    void UnregisterLocater(RuleSetLocater const& locater) {_UnregisterLocater(locater);}

    //! Find all rulesets that are supported by the specified connection.
    //! @param[in] connection The connection to check whether the ruleset is supported.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, all available rulesets are returned.
    //! @note A ruleset is considered supported if all its schemas are supported in the specified connection.
    bvector<PresentationRuleSetPtr> LocateRuleSets(IConnectionCR connection, Utf8CP rulesetId) const {return _LocateRuleSets(connection, rulesetId);}

    //! Get IDs of all available rulesets.
    bvector<Utf8String> GetRuleSetIds() const {return _GetRuleSetIds();}
};

//=======================================================================================
//! Ruleset locaters' manager which keeps all locaters and sends ruleset requests based
//! on ruleset locater priorities.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RuleSetLocaterManager : IRulesetLocaterManager, IRulesetCallbacksHandler, IConnectionsListener
{
    struct CacheKey
        {
        Utf8String m_connectionId;
        Utf8String m_rulesetId;
        CacheKey() {}
        CacheKey(Utf8String connectionId, Utf8String rulesetId) : m_connectionId(connectionId), m_rulesetId(rulesetId) {}
        bool operator<(CacheKey const& other) const
            {
            return m_connectionId < other.m_connectionId
                || m_connectionId == other.m_connectionId && m_rulesetId < other.m_rulesetId;
            }
        };

    struct CacheValue
        {
        int m_locaterPriority;
        PresentationRuleSetPtr m_ruleset;
        CacheValue() {}
        CacheValue(PresentationRuleSetPtr ruleset, int locaterPriority) : m_locaterPriority(locaterPriority), m_ruleset(ruleset) {}
        };

private:
    bvector<RuleSetLocaterPtr> m_locaters;
    mutable bool m_isLocating;
    mutable bmap<CacheKey, bvector<CacheValue>> m_rulesetsCache;
    IConnectionManagerCR m_connections;
    mutable BeMutex m_mutex;

private:
    void _OnConnectionEvent(ConnectionEvent const&) override;
    void RemoveCachedRulesets(Utf8CP rulesetId);

protected:
    // IRulesetLocaterManager
    ECPRESENTATION_EXPORT void _InvalidateCache(Utf8CP rulesetId) override;
    ECPRESENTATION_EXPORT void _RegisterLocater(RuleSetLocater&) override;
    ECPRESENTATION_EXPORT void _UnregisterLocater(RuleSetLocater const&) override;
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(IConnectionCR, Utf8CP rulesetId) const override;
    ECPRESENTATION_EXPORT bvector<Utf8String> _GetRuleSetIds() const override;

    // IRulesetCallbacksHandler
    ECPRESENTATION_EXPORT void _OnRulesetDispose(RuleSetLocaterCR locater, PresentationRuleSetCR ruleset) override;
    ECPRESENTATION_EXPORT void _OnRulesetCreated(RuleSetLocaterCR locater, PresentationRuleSetR ruleset) override;

public:
    ECPRESENTATION_EXPORT RuleSetLocaterManager(IConnectionManagerCR connections);
    ECPRESENTATION_EXPORT ~RuleSetLocaterManager();
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
