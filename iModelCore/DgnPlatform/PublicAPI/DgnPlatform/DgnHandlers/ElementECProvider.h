/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ElementECProvider.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

//! @cond DONTINCLUDEINDOC
#if defined (_MSC_VER)
    #pragma managed(push, off)

    // Disable warning messages 4266 - no override available for virtual member function from base 'type'; function is hidden
    #pragma warning( disable : 4266 )
#endif // defined (_MSC_VER)

#define PROVIDERID_Element 0xECDB

#include <ECObjects/ECObjectsAPI.h>

DGNPLATFORM_TYPEDEFS(ElementECProvider);
DGNPLATFORM_TYPEDEFS(IElementECInstanceGenerator);
DGNPLATFORM_TYPEDEFS(ElementECInstance);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct ElementECDelegate;
#ifdef DGN_IMPORTER_REORG_WIP

typedef RefCountedPtr<ElementECEnabler>  ElementECEnablerPtr;

typedef bvector<ElementECEnablerCP>     ElementECEnablersVector;
typedef bvector<ElementECEnablerCP>     ElementECEnablerCPVector;

/*---------------------------------------------------------------------------------**//**
* An object capable of generating instances from and persisting instances to an element.
* Must be registered with ElementECProvider::RegisterInstanceGenerator in order to be
* used.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct IElementECInstanceGenerator
    {
protected:
    virtual void                        _GetSchemaKey (ECN::SchemaKeyR schemaKey) = 0;
    virtual void                        _GetECClasses (T_ECClassCPVector& classes) = 0;
    virtual ElementECEnablerPtr         _CreateEnabler (ECN::ECClassCR ecClass) = 0;
    virtual ElementECEnablersVector*    _ObtainInstanceEnablers() = 0;
    virtual bool                        _ExposeChildrenForEdit (ElementHandleCR rootElem) const = 0;
    virtual bool                        _SupportsEditLockedElement () const = 0;
    virtual bool                        _SupportsElement (ElementHandleCR eh) const = 0;
    virtual void                        _GenerateInstances (DgnElementECInstanceVector& instance, ElementHandleCR eh, ElementECEnablerCPVector const& enablers) const = 0;
    virtual bool                        _ReplaceElement (DgnElementECInstanceR instance) const = 0;
    virtual void                        _Dispose() = 0;

public:
    DGNPLATFORM_EXPORT static WCharCP   BASE_ELEMENT_SCHEMA;
    DGNPLATFORM_EXPORT static WCharCP   EXTENDED_ELEMENT_SCHEMA;
                       static int       VERSION_MAJOR;
                       static int       VERSION_MINOR;

    //! Get the key for the primary schema used by this instance generator
    DGNPLATFORM_EXPORT void                         GetSchemaKey (ECN::SchemaKeyR schemaKey);
    //! Get the list of EC classes exposed by this instance generator
    DGNPLATFORM_EXPORT void                         GetECClasses (T_ECClassCPVector& classes);
    //! Create an enabler for the specified EC class
    DGNPLATFORM_EXPORT ElementECEnablerPtr          CreateEnabler (ECN::ECClassCR ecClass);
    //! Get a list of enablers, one for each supported EC class
    DGNPLATFORM_EXPORT ElementECEnablersVector*     ObtainInstanceEnablers();
    //! Return true if a complex element generates non-readonly instances for its child elements
                       bool                         ExposeChildrenForEdit (ElementHandleCR rootElem) const;
   //! Return true if element header's locked flag should be ignored when determining if instance is read-only
                       bool                         SupportsEditLockedElement () const;
   //! Return true if this instance generator can generate instances for the passed element
                       bool                         SupportsElement (ElementHandleCR eh) const;
   //! Generate instances for the specified element using the supplied list of enablers
                       void                         GenerateInstances (DgnElementECInstanceVector& instances, ElementHandleCR eh, ElementECEnablerCPVector const& enablers) const;
   //! Apply modifications made to the instance to the underlying element
                       bool                         ReplaceElement (DgnElementECInstanceR instance) const;
   //! Called when this instance generator is unregistered, including when the ElementECProvider itself is unregistered
                       void                         Dispose();

    static ECN::SchemaKey CreateSchemaKey (WCharCP schemaName);
    };

struct DelegatedElementECEnabler;
/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementECEnabler : DgnElementECInstanceEnabler
{
private:
    // Only subclasses that support CreateInstanceAsElement will use this WipInstance stuff.
    mutable ECN::ClassLayoutCP           m_sharedWipClassLayout;
    mutable ECN::StandaloneECEnablerPtr  m_sharedWipEnabler;
    mutable ECN::StandaloneECInstancePtr m_sharedWipInstance;

    void    InitializeSharedWipInstance (ECN::IStandaloneEnablerLocaterP standaloneInstanceEnablerLocater) const;

protected:
    DGNPLATFORM_EXPORT ElementECEnabler(ECN::ECClassCR ecClass);
    DGNPLATFORM_EXPORT ~ElementECEnabler();

    virtual ECN::StandaloneECInstanceP           _GetSharedWipInstance () const override;
    virtual ECN::StandaloneECEnablerR            _GetStandaloneECInstanceEnabler () const override;
    virtual IElementECInstanceGeneratorR        _GetInstanceGenerator (ElementHandleCR eh) const = 0;
public:
    ECN::StandaloneECEnablerR            GetStandaloneInstanceEnabler () const;
    IElementECInstanceGeneratorR        GetInstanceGenerator (ElementHandleCR eh) const;
    virtual DelegatedElementECEnabler const*  AsDelegatedEnabler() const {return NULL;}
};

/*---------------------------------------------------------------------------------**//**
* An instance created by an IElementECInstanceGenerator.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementECInstance : DgnElementECInstance
    {
protected:
    DGNPLATFORM_EXPORT ElementECInstance (DgnModelR modelRef, DgnElementP elementRef, uint32_t localId);

    DGNPLATFORM_EXPORT virtual bool         _IsReadOnly() const override;
    //! If you override this method you must invoke this base implementation and append any extra data to the returned string.
    DGNPLATFORM_EXPORT virtual bool         _GetInstanceIdExtension (WStringR extension) const override;
public:
    IElementECInstanceGeneratorR            GetInstanceGenerator() const;
    ElementECEnablerCR                      GetElementECEnabler() const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementECProvider : IDgnElementECProvider, ECN::IStandaloneEnablerLocater, DgnHost::HostObjectBase
{
    typedef bvector <IElementECInstanceGeneratorP>          GeneratorList;
    typedef bpair <IElementECInstanceGeneratorP, uint8_t>     GeneratorAndId;
    typedef bvector <GeneratorAndId>                        GeneratorAndIdList;
    typedef bvector <void const*>                           DelegateIdList;
private:
    static DgnHost::Key&    GetHostKey ();
    // per ECClass, a list of generators supporting that class and an enabler. guaranteed enabler non-null and handler list not empty
    struct CachedECClassData
        {
        ElementECEnablerCP                      m_enabler;
        bvector <IElementECInstanceGeneratorP>  m_extensions;
        };

    typedef bmap <ECN::SchemaNameClassNamePair, CachedECClassData>   ECGeneratorListsAndEnablersByClass;
    typedef bmap <ECN::SchemaNameClassNamePair, ElementECEnablerPtr> EnablerPtrsByName;
    typedef bmap <ECN::ECClassCP, DgnECInstanceEnablerPtr>           ExternalEnablersByClass;

    mutable ECN::ECSchemaReferenceList           m_schemaList;
    mutable EnablerPtrsByName                   m_instanceEnablersByName;                         // holds refcounted enablers to keep them alive
    mutable ECGeneratorListsAndEnablersByClass  m_ecGeneratorListsAndEnablersByClass;             // used by FilteredFinders
    mutable GeneratorAndIdList                  m_auxGenerators;                                  // non-extension-based instance generators
    DelegateIdList                              m_delegateIds;
    uint8_t                                     m_maxAuxGeneratorId;                              // incremented each time a new instance generator is registered; serves as ID

    ElementECProvider() : m_maxAuxGeneratorId (0) { }
    static ElementECProviderP               CreateProvider()  { return new ElementECProvider(); }

    // IDgnECProvider methods
    virtual ECN::ECSchemaPtr                 _LocateSchemaInDgnFile (SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) override;
    virtual BentleyStatus                   _LocateSchemaXmlInDgnFile (WStringR schemaXml, SchemaInfoR schemaInfo, ECN::SchemaMatchType matchType) override;
    virtual void                            _GetSchemaInfos (bvector<SchemaInfo>& infos, DgnDbR dgnFile, ECSchemaPersistence persistence) override;
    virtual uint16_t                        _GetProviderId () const { return PROVIDERID_Element; }
    virtual WCharCP                         _GetProviderName () const { return L"ElementECProvider"; }
    virtual DgnECInstanceEnablerP           _ObtainInstanceEnablerByName (WCharCP schemaName, WCharCP className, DgnDbR dgnFile, void* perFileCache) override;
#if WIP_DEAD_DGNEC_CODE
    virtual void                            _FindRelatedInstances (IDgnECRelationshipInstanceVector* relationships, DgnElementECInstanceVector* relatedInstances, DgnElementECInstanceCR sourceInstance, QueryRelatedClassSpecifier const& relationshipClassSpecifier, bool useRecursion) override;
    virtual IDgnECInstanceFinderPtr         _CreateFinder (DgnDbR dgnFile, ECQueryCR query, void* perFileCache) const override;
    virtual IDgnECInstanceFinderPtr         _CreateRelatedFinder (DgnECInstanceCR source, QueryRelatedClassSpecifier const& relationshipClassSpecifier) const override;
    virtual IDgnECRelationshipFinderPtr     _CreateRelationshipFinder (DgnECInstanceCR source, QueryRelatedClassSpecifier const& relationshipClassSpecifier) const override;
#endif
    virtual DgnECRelationshipEnablerP       _ObtainDgnECRelationshipEnabler (WCharCP schemaName, WCharCP className, DgnDbR dgnFile) override;
    virtual DgnElementECInstancePtr         _LoadInstance (DgnModelR modelRef, DgnElementP elementRef, uint32_t localId, bool loadProperties) const override;
#if WIP_DEAD_DGNEC_CODE
    virtual DgnECInstancePtr                _LocateInstance (IDgnECInstanceLocatorCR locator, bool loadProperties) const override;

#endif
  // IStandaloneEnablerLocater
    virtual    ECN::StandaloneECEnablerPtr   _LocateStandaloneEnabler (ECN::SchemaKeyCR schemaKey, const wchar_t* className) override;
    virtual void *                          _OnInitializeForFile (DgnDbR dgnFile) override;
    virtual void                            _OnDisconnectFromFile (DgnDbR dgnFile, void* providerPerFileCache) override;

    void                                    AddInstanceEnabler (ElementECEnablerR instanceEnabler);
    void                                    RegisterSchema (IElementECInstanceGenerator& extension, T_ECClassCPVector& ecClasses);
    void                                    RegisterGeneratorAndEnablersByClass (IElementECInstanceGenerator& extension, T_ECClassCPVector& ecClasses);
    void                                    UnRegisterGeneratorAndEnablersByClass (IElementECInstanceGenerator& generator);

    ElementECEnablerP                       GetElementECEnablerByName (WCharCP schemaName, WCharCP className);
public:
    static void                             InitProvider (DgnECManagerR mgr);

    DGNPLATFORM_EXPORT static ElementECProviderR  GetProvider();


    // convenience method, optimizes cache lookup
    void                                    GetInstanceGeneratorsForElement (GeneratorList& generators, ElementHandleCR eh) const;

#if WIP_DEAD_DGNEC_CODE
    DGNPLATFORM_EXPORT void                 FindRelatedInstancesOnElement (DgnElementECInstanceVector& relatedInstances, QueryRelatedClassSpecifier const& classSpec, DgnElementP element, DgnModelR modelRef, WCharCP sourceInstanceId) const;
#endif
    DGNPLATFORM_EXPORT ElementECEnablerP    ObtainElementECEnabler (ECN::ECClassCR ecClass, IElementECInstanceGeneratorR extension);

    static void                             OnSchemaUnloaded (ECN::ECSchemaR ecSchema);
    void                                    RegisterInstanceGenerator (IElementECInstanceGeneratorR generator);
    void                                    UnRegisterInstanceGenerator (IElementECInstanceGeneratorR generator);
    void                                    GetInstanceIdExtension (WStringR idExtension, ElementECInstanceCR instance) const;
    uint16_t                                GetDelegateId (ElementECDelegate const& del);

    // Creates a 'standalone' DgnECInstance for an instance obtained from an external DgnFile. e.g., the target of a link pointing to a model or view in another file; in which case 'hostFile' is the file containing the link, not the link target.
    DgnECInstancePtr                        CreateExternalInstance (DgnECInstanceCR source, DgnDbR hostFile) const;

    DGNPLATFORM_EXPORT static void          BackDoor_UnregisterProviderForUnitTest();
    DGNPLATFORM_EXPORT static uint32_t      BackDoor_GetDimstylePropertyType (uint32_t& dimProp, uint32_t propertyIndex);
};
#endif

//__PUBLISH_SECTION_START__
END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma managed(pop)
#endif // defined (_MSC_VER)

//! @endcond
