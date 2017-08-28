/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/RuleSetLocater.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRules.h>

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
    //! @param[in] ruleset The ruleset that was created.
    virtual void _OnRulesetCreated(PresentationRuleSetCR ruleset) {}

    //! Called when a ruleset is about to be disposed.
    //! @param[in] ruleset The ruleset that will be disposed.
    virtual void _OnRulesetDispose(PresentationRuleSetCR ruleset) {}
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

protected:
    //! Called to find matching rulesets.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, the implementation should return all available rulesets.
    //! @return All matching rulesets.
    virtual bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const = 0;

    //! Called to get all available ruleset IDs for this locater.
    virtual bvector<Utf8String> _GetRuleSetIds() const = 0;

    //! Called to get the locater priority.
    virtual int _GetPriority() const = 0;

    //! @see InvalidateCache
    virtual void _InvalidateCache(Utf8CP rulesetId) = 0;

    //! Implementations should call this function before they dispose a ruleset.
    ECPRESENTATION_EXPORT void OnRulesetDisposed(PresentationRuleSetCR ruleset) const;

    //! Implementations should call this function when they create a new ruleset instance.
    ECPRESENTATION_EXPORT void OnRulesetCreated(PresentationRuleSetCR ruleset) const;
    
//__PUBLISH_SECTION_END__
public:
    ECPRESENTATION_EXPORT void SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler);
    
//__PUBLISH_SECTION_START__
public:
    //! Constructor.
    RuleSetLocater() : m_rulesetCallbacksHandler(nullptr) {}
    
    //! Destructor.
    virtual ~RuleSetLocater() {}

    //! Disposed cached ruleset.
    //! @param[in] rulesetId ID of the ruleset which should be disposed from cache. NULL means all rulesets.
    void InvalidateCache(Utf8CP rulesetId = nullptr) {_InvalidateCache(rulesetId);}

    //! Find all matching rulesets.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, all available rulesets are returned.
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> LocateRuleSets(Utf8CP rulesetId = nullptr) const;

    //! Get IDs of all available rulesets.
    ECPRESENTATION_EXPORT bvector<Utf8String> GetRuleSetIds() const;

    //! Get the locater priority.
    ECPRESENTATION_EXPORT int GetPriority() const;
};

typedef RefCountedPtr<RuleSetLocater> RuleSetLocaterPtr;

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
//! Ruleset locaters' manager which keeps all locaters and sends ruleset requests based
//! on ruleset locater priorities.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                03/2015
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RuleSetLocaterManager : NonCopyableClass, IRulesetCallbacksHandler
{
private:
    IRulesetCallbacksHandler* m_rulesetCallbacksHandler;
    bvector<RuleSetLocaterPtr> m_locaters;
    mutable bmap<Utf8String, bvector<PresentationRuleSetPtr>> m_rulesetsCache;
    
protected:
    ECPRESENTATION_EXPORT void _OnRulesetDispose(PresentationRuleSetCR ruleset) override;
    ECPRESENTATION_EXPORT void _OnRulesetCreated(PresentationRuleSetCR ruleset) override;

public:
    void SetRulesetCallbacksHandler(IRulesetCallbacksHandler* handler);

public:
    //! Constructor.
    RuleSetLocaterManager() : m_rulesetCallbacksHandler(nullptr) {}

    //! Tells each managed ruleset locater to dispose its cached rulesets.
    //! @param[in] rulesetId ID of the ruleset which should be disposed from cache. NULL means all rulesets.
    ECPRESENTATION_EXPORT void InvalidateCache(Utf8CP rulesetId = nullptr);

    //! Register a new ruleset locater.
    ECPRESENTATION_EXPORT void RegisterLocater(RuleSetLocater& locater);

    //! Unregister a locater.
    ECPRESENTATION_EXPORT void UnregisterLocater(RuleSetLocater const& locater);

    //! Find all rulesets that are supported by the specified connection.
    //! @param[in] connection The connection to check whether the ruleset is supported.
    //! @note A ruleset is considered supported if all its schemas are supported in the specified connection.
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> LocateSupportedRulesets(ECDbCR connection) const;

    //! Find matching rulesets.
    //! @param[in] rulesetId The ID of the ruleset to find. If nullptr, all available rulesets are returned.
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> LocateRuleSets(Utf8CP rulesetId) const;

    //! Get IDs of all available rulesets.
    ECPRESENTATION_EXPORT bvector<Utf8String> GetRuleSetIds() const;
};
typedef RuleSetLocaterManager const& RuleSetLocaterManagerCR;

//=======================================================================================
//! Ruleset locater that finds supplemental rulesets in the specified directory.
// @bsiclass                                   Aidas.Vaiksnoras                05/2017
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SupplementalRuleSetLocater : DirectoryRuleSetLocater 
{
private:
    SupplementalRuleSetLocater(BeFileNameCR directoryPath) : DirectoryRuleSetLocater(directoryPath.GetNameUtf8().c_str()) {}

protected:
    ECPRESENTATION_EXPORT bvector<PresentationRuleSetPtr> _LocateRuleSets(Utf8CP rulesetId) const override;

public:
    //! Create a new locater.
    static RefCountedPtr<SupplementalRuleSetLocater> Create(BeFileNameCR directoryPath) { return new SupplementalRuleSetLocater(directoryPath); }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
