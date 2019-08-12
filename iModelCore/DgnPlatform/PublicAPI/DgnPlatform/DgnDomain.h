/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnDbTables.h>

// This macro declares the required members for a DgnDomain
#define DOMAIN_DECLARE_MEMBERS(__classname__,__exporter__) \
    private:   __exporter__ static __classname__*& z_PeekDomain(); \
                            static __classname__* z_CreateDomain(); \
    public:    __exporter__ static __classname__& GetDomain() {return z_Get##__classname__##Domain();}\
               __exporter__ static __classname__& z_Get##__classname__##Domain();

// This macro must be included somewhere within a source file that implements a DgnDomain
#define DOMAIN_DEFINE_MEMBERS(__classname__) \
    __classname__*  __classname__::z_CreateDomain() {__classname__* instance= new __classname__(); return instance;}\
    __classname__*& __classname__::z_PeekDomain() {static __classname__* s_instance = 0; return s_instance;}\
    __classname__&  __classname__::z_Get##__classname__##Domain(){__classname__*& instance=z_PeekDomain(); if (nullptr==instance) instance=z_CreateDomain(); return *instance;}

// This macro must be included within the class declaration of a DgnDomain::Handler.
#define DOMAINHANDLER_DECLARE_MEMBERS_NO_CTOR(__classname__,__exporter__) \
    private:   __exporter__ static __classname__*& z_PeekInstance(); \
                            static __classname__* z_CreateInstance(); \
    protected: Dgn::DgnDomain::Handler* _CreateMissingHandler(uint64_t restrictions, Utf8StringCR domainName, Utf8StringCR className) override {return new Dgn::DgnDomain::MissingHandler<__classname__>(restrictions, domainName, className, *this);}\
    public:    __exporter__ static __classname__& GetHandler() {return z_Get##__classname__##Instance();}\
               __exporter__ static __classname__& z_Get##__classname__##Instance();

// This macro declares the required members for an DgnDomain::Handler.
#define DOMAINHANDLER_DECLARE_MEMBERS(__ECClassName__,__classname__,__superclass__,__exporter__) \
    private:   typedef __superclass__ T_Super; \
    protected: __classname__()  {m_ecClassName =  __ECClassName__ ;} \
    DOMAINHANDLER_DECLARE_MEMBERS_NO_CTOR(__classname__,__exporter__)

// This macro must be included somewhere within a source file that implements a DgnDomain::Handler
#define HANDLER_DEFINE_MEMBERS(__classname__) \
    __classname__*  __classname__::z_CreateInstance() {__classname__* instance= new __classname__(); instance->SetSuperClass(&T_Super::GetHandler()); return instance;}\
    __classname__*& __classname__::z_PeekInstance() {static __classname__* s_instance = 0; return s_instance;}\
    __classname__&  __classname__::z_Get##__classname__##Instance(){__classname__*& instance=z_PeekInstance(); if (nullptr==instance) instance=z_CreateInstance(); return *instance;}

// This macro declares the required members for an DgnDomain::Handler::Extension.
#define HANDLER_EXTENSION_DECLARE_MEMBERS(__classname__,__exporter__) \
    private: __exporter__ static Token& z_Get##__classname__##Token();\
    public: static BentleyStatus RegisterExtension(Dgn::DgnDomain::Handler& handler, __classname__& obj) {return obj.RegisterExt(handler,z_Get##__classname__##Token());}\
            static BentleyStatus DropExtension(Dgn::DgnDomain::Handler& handler) {return DropExt(handler,z_Get##__classname__##Token());}\
            static __classname__* Cast(Dgn::DgnDomain::Handler& handler) {return (__classname__*) CastExt(handler,z_Get##__classname__##Token());}

// This macro must be included somewhere within a source file that implements a DgnDomain::Handler::Extension
#define HANDLER_EXTENSION_DEFINE_MEMBERS(__classname__) \
    Dgn::DgnDomain::Handler::Extension::Token& __classname__::z_Get##__classname__##Token(){static Dgn::DgnDomain::Handler::Extension::Token* s_token=0; if (0==s_token) s_token = NewToken(); return *s_token;}

#define TABLEHANDLER_DECLARE_MEMBERS(__classname__,__exporter__) \
    public:  __exporter__ static __classname__& GetHandler();

// This macro must be included somewhere within a source file that implements a DgnDomain::TableHandler
#define TABLEHANDLER_DEFINE_MEMBERS(__classname__) \
    __classname__&  __classname__::GetHandler(){static __classname__* s_instance=nullptr; if (nullptr==s_instance) s_instance=new __classname__(); return *s_instance;}

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Option to control the processing of the revisions
// @bsiclass                                                 Ramanujam.Raman   10/15
//=======================================================================================
enum class RevisionProcessOption : int
{
    None = 0,
    Merge, //!< Revisions will be merged in
    Reverse, //!< Revisions will be reversed
    Reinstate //!< Revisions will be reinstated
};

//=======================================================================================
//! Options to upgrade schemas when the DgnDb is opened
//! @note We upgrade the schemas only when the DgnDb is opened to eliminate the impact 
//! of the changes to existing caches. 
//=======================================================================================
struct SchemaUpgradeOptions
{
//! Option to control the validation and upgrade of domain schemas in the DgnDb.
enum class DomainUpgradeOptions : int
    {
    CheckRequiredUpgrades, //!< Domain schemas will be validated for any required upgrades. Any errors will be reported back, and cause the application to fail opening the DgnDb. 
    CheckRecommendedUpgrades, //!< Domain schemas will be validated for any required or optional upgrades. Any errors will be reported back, and cause the application to fail opening the DgnDb. 
    SkipCheck, //!< Domain schemas will neither be validated nor be upgraded. Used only internally. 
    Upgrade //!< Domain schemas will be upgraded if necessary. However, only compatible schema upgrades will be allowed - these are typically additions of classes, properties, and changes to custom attributes. 
    };

private:
    DomainUpgradeOptions m_domainUpgradeOptions = DomainUpgradeOptions::CheckRequiredUpgrades;
    bvector<DgnRevisionCP> m_revisions;
    RevisionProcessOption m_revisionProcessOption = RevisionProcessOption::None;

public:
    //! Default constructor
    SchemaUpgradeOptions() {}

    //! Constructor to setup schema upgrades from the registered domains
    SchemaUpgradeOptions(DomainUpgradeOptions domainOptions) { SetUpgradeFromDomains(domainOptions); }

    //! Constructor to setup schema upgrades by merging/reversing/reinstating a revision (that may contain schema changes).
    SchemaUpgradeOptions(DgnRevisionCR revision, RevisionProcessOption revisionOptions = RevisionProcessOption::Merge) { SetUpgradeFromRevision(revision, revisionOptions); }

    //! Constructor to setup schema upgrades by merging revisions (that may contain schema changes).
    SchemaUpgradeOptions(bvector<DgnRevisionCP> const& revisions, RevisionProcessOption revisionOptions = RevisionProcessOption::Merge) { SetUpgradeFromRevisions(revisions, revisionOptions); }

    //! Setup to upgrade schemas from the registered domains
    void SetUpgradeFromDomains(DomainUpgradeOptions domainOptions = DomainUpgradeOptions::CheckRequiredUpgrades)
        {
        m_domainUpgradeOptions = domainOptions;
        }

    //! Setup Schema upgrades by merging a revision (that may contain schema changes)
    void SetUpgradeFromRevision(DgnRevisionCR upgradeRevision, RevisionProcessOption revisionOptions = RevisionProcessOption::Merge)
        {
        m_revisions.clear();
        m_revisions.push_back(&upgradeRevision);
        m_revisionProcessOption = revisionOptions;
        }

    //! Setup Schema upgrades by merging a revision (that contains schema changes)
    void SetUpgradeFromRevisions(bvector<DgnRevisionCP> const& upgradeRevisions, RevisionProcessOption revisionOptions = RevisionProcessOption::Merge)
        {
        m_revisions = upgradeRevisions;
        m_revisionProcessOption = revisionOptions;
        }

    //! Get the option that controls upgrade of schemas in the DgnDb from the domains.
    DomainUpgradeOptions GetDomainUpgradeOptions() const { return m_domainUpgradeOptions; }

    //! Gets the revisions that are to be processed
    bvector<DgnRevisionCP> const& GetRevisions() const { return m_revisions; }

    //! Get the option that controls the processing of revisions 
    RevisionProcessOption GetRevisionProcessOption() const { return m_revisionProcessOption;  }

    //! Returns true if schemas are to be upgraded from the domains.
    bool AreDomainUpgradesAllowed() const { return m_domainUpgradeOptions == DomainUpgradeOptions::Upgrade; }

    //! Resets the options
    void Reset()
        {
        m_domainUpgradeOptions = DomainUpgradeOptions::CheckRequiredUpgrades;
        m_revisions.clear();
        m_revisionProcessOption = RevisionProcessOption::None;
        }
};

//=======================================================================================
//! Status returned by schema validation, import or upgrade routines
//=======================================================================================
enum class SchemaStatus
    {
    Success = 0,
    SchemaNotFound,
    SchemaReadFailed,
    SchemaTooNew,
    SchemaTooOld,
    SchemaUpgradeRequired,
    SchemaLockFailed,
    SchemaImportFailed,
    SchemaDomainNamesMismatched,
    SchemaInvalid,
    MergeSchemaRevisionFailed,
    DbHasLocalChanges,
    DbIsReadonly,
    CouldNotAcquireLocksOrCodes,
    SchemaUpgradeRecommended,
    };

struct DgnDomains;

/** @addtogroup GROUP_DgnDomain DgnDomain Module

A "Domain" is a combination of an ECSchema, plus a set of C++ classes that implement its runtime behavior.

To connect your Domain's ECSChema with your C++ classes, create a subclass of DgnDomain. A DgnDomain is a singleton - that is,
there is only one instance of a DgnDomain subclass that applies to all DgnDbs for a session. You tell the system
about your DgnDomain by calling the static method DgnDomains::RegisterDomain.  The constructor of DgnDomain takes the "domain name",
which must match the ECShema file name. That is how a DgnDomain is paired with its ECSchema.

A DgnDomain holds an array of C++ singleton objects, each of which each derive from DgnDomain::Handler. A DgnDomain::Handler
holds the name of the ECClass it "handles". DgnDomain::Handlers are added to a DgnDomain by calling DgnDomain::RegisterHandler.
Note that DgnDomain::Handlers are singletons - they apply to all DgnDbs, and have no instance data.

You can create a DgnDomain::Handler for any ECClass, and the connection between them is via schema name/class name.
A DgnDomain::Handler handles an ECClass, *and all of its subclasses* unless they have their own DgnDomain::Handler

Within a given DgnDb, instances of an ECClass are known by their local DgnClassId. The same ECClass may have two different
DgnClassIds in two different DgnDbs. Whenever a DgnDb is created or opened, the list of loaded DgnDomains is stored in a map of
local DgnClassId to DgnDomain::Handler (it will report an error if any expected ones are missing.) That map is stored in a class
called DgnDomains, which is accessed through the method DgnDb::Domains().

The DgnDomain for the base "dgn" schema is called BisCoreDomain. It is always loaded and it registers all of its DgnDomain::Handlers.

*/

struct DgnDomains;
struct TxnTable;

//=======================================================================================
//! A DgnDomain is a singleton C++ object that provides the runtime implementation for an ECSchema.
//! A given DgnDomain supplies set of "Handlers," each of which are singleton C++ objects derived from DgnDomain::Handler,
//! that provide the implementation for one of its ECClasses. Stated differently, a DgnDomain is a collection of DgnDomain::Handlers
//! for its ECClasses. It is not possible for one DgnDomain to supply a DgnDomain::Handler for a different DgnDomain.
//! Domains are "registered" at program startup time (via that static method DgnDomains::RegisterDomain),
//! remain for an entire session, and apply to all DgnDb's that are opened
//! or created during that session. 
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDomain : NonCopyableClass
{
    friend struct DgnDomains;

    //! The current version of the HandlerAPI
    enum {API_VERSION = 1};

    //! Flag to indicate if the domain API can be used for inserts, updates or deletes. 
    enum class Readonly { Yes = 1, No = 0 };

    //! Flag to indicate if the domain is considered to be required for all DgnDb-s in the session. 
    enum class Required { Yes = 1, No = 0 };

    struct Handler;

    //! A template used to create a proxy handler of a superclass when the handler subclass cannot be found at run-time.
    //! The proxy handler can restrict the behavior of the superclass according to restrictions imposed by the subclass.
    template<typename T> struct MissingHandler : T
    {
    private:
        uint64_t    m_restrictions;
        Utf8String  m_domainName;

        DgnDomain::Handler* _CreateMissingHandler(uint64_t restrictions, Utf8StringCR domainName, Utf8StringCR className) override { return T::_CreateMissingHandler(restrictions, domainName, className); }
        bool _IsRestrictedAction(uint64_t restrictedAction) const override { return 0 != (m_restrictions & restrictedAction); }
        bool _IsMissingHandler() const override { return true; }
        Utf8CP _GetDomainName() const override { return m_domainName.c_str(); }
    public:
        explicit MissingHandler(uint64_t restrictions, Utf8StringCR domainName, Utf8StringCR className, T& base) : m_restrictions(restrictions), m_domainName(domainName)
            {
            this->m_domain = &base.GetDomain();
            this->m_superClass = &base;
            this->m_ecClassName = className;
            }
    };

    //! A DgnDomain::Handler is a C++ singleton object that provides an implementation for an ECClass and all of its subclasses.
    //! A DgnDomain::Handler must be registered with its DgnDomain via DgnDomain::RegisterHandler before any DgnDbs are created or opened.
    struct Handler : NonCopyableClass
        {
        friend struct DgnDomains;
        //! A DgnDomain::Handler::Extension can be used to add additional interfaces to a Handler at runtime. If a Handler is
        //! extended, all of its registered subclasses inherit that extension too.
        //! To implement a DgnDomain::Handler::Extension, derive from that class and put the HANDLER_EXTENSION_DECLARE_MEMBERS macro in
        //! your class declaration and the HANDLER_EXTENSION_DEFINE_MEMBERS in your implementation. E.g.:

        struct Extension
        {
            friend struct Handler;
            struct Token
            {
                friend struct Extension;
            private:
                Token() {}
            };

        protected:
            BentleyStatus RegisterExt(Handler& handler, Token& extensionToken) {return handler.AddExtension(extensionToken, *this);}//!< @private
            static BentleyStatus DropExt(Handler& handler, Token& extensionToken) {return handler.DropExtension(extensionToken);}   //!< @private
            static Extension* CastExt(Handler& handler, Token& extensionToken) {return handler.FindExtension(extensionToken);}      //!< @private
            static Token* NewToken() {return new Token();}                                                                           //!< @private
        };

        //! Identifies actions which may be restricted for objects created by a missing subclass of Handler
        //! Specified as an array of action names in a ClassHasHandler custom attribute attached to the Handler's ECClass.
        //! The action name to be used in the custom attribute's array is specified in quotes for each action below. (Parsing is case-insensitive)
        //! Subclasses of Handler::RestrictedAction can add their own actions.
        struct RestrictedAction
        {
            static const uint64_t None = 0; //!< No restrictions
            static const uint64_t All = 0xffffffffffffffff; //!< All modifications are prohibited. "All"

            static const uint64_t Delete = 1; //!< Delete the object. "Delete"
            static const uint64_t Insert = Delete << 1; //!< Insert a new instance of this EClass into the database. "Insert"
            static const uint64_t Update = Insert << 1; //!< Update an existing instance of this ECClass in the database. "Update"

            static const uint64_t NextAvailable = Update << 1; //!< Subclasses can add actions beginning with this value.

            DGNPLATFORM_EXPORT static uint64_t Parse(Utf8CP name); //!< Parse action name from ClassHasHandler custom attribute. Subclasses must call this base class method.
        };

        private:
            DGNPLATFORM_EXPORT BentleyStatus AddExtension(Extension::Token&, Extension&);
            DGNPLATFORM_EXPORT BentleyStatus DropExtension(Extension::Token&);
            DGNPLATFORM_EXPORT Extension* FindExtension(Extension::Token&);

        struct ExtensionEntry
        {
            ExtensionEntry(Extension::Token& token, Extension& extension, ExtensionEntry* next) : m_token(token), m_extension(extension), m_next(next){}
            static ExtensionEntry* Find(ExtensionEntry*, Extension::Token const&);

            Extension::Token&   m_token;
            Extension&          m_extension;
            ExtensionEntry*     m_next;
        };

        friend struct DgnDomain;
        static Handler*  z_CreateInstance();
        static Handler*& z_PeekInstance();

    protected:
        Handler*        m_superClass;
        Utf8String      m_ecClassName;
        ExtensionEntry* m_extensions;
        DgnDomainCP     m_domain;
        Handler() : m_domain(nullptr), m_ecClassName("Handler") {m_superClass = (Handler*) 0xbadf00d; m_extensions=nullptr;  }
        virtual ~Handler(){}
        void SetSuperClass(Handler* super) {m_superClass = super;}
        void SetDomain(DgnDomain& domain) {m_domain = &domain;}
        DGNPLATFORM_EXPORT virtual DgnDbStatus _VerifySchema(DgnDomains&);
        virtual Handler* _CreateMissingHandler(uint64_t restrictions, Utf8StringCR domainName, Utf8StringCR className) { return new MissingHandler<Handler>(restrictions, domainName, className, *this); }
        virtual uint64_t _ParseRestrictedAction(Utf8CP restriction) const { return RestrictedAction::Parse(restriction); }

    public:
        //! To enable version-checking for your handler, override this method to report the
        //! API version that was used to compiler your handler.
        //! @remarks Version-checking is a convenience to developers during the product development
        //!          cycle. It allows one to detect when a handler needs to be recompiled
        //!          in order to catch up with API changes.
        //! @remarks To override this method, simply copy the following line into your handler.
        virtual uint32_t _GetApiVersion() {return API_VERSION;}

        Handler* GetSuperClass() const {return m_superClass;}
        Handler* GetRootClass();

        //! Get the name of the ECClass handled by this Handler
        Utf8StringCR GetClassName() const {return m_ecClassName;}

        //! Get the name of the DgnDomain from which this handler's ECClass originated.
        //! Note in the case of a missing domain, this may differ from GetDomain().GetDomainName()
        virtual Utf8CP _GetDomainName() const { return GetDomain().GetDomainName(); }

        //! Get DgnDomain of Handler
        DgnDomainCR GetDomain() const {return *m_domain;}

        //! get a localized version of the class name for this handler
        virtual void _GetLocalizedName(Utf8StringR name, uint32_t desiredLength) {name = GetClassName();}

        //! get a localized description of this handler's class
        virtual void _GetLocalizedDescription(Utf8StringR descr, uint32_t desiredLength) {descr = "";}

        //! Query whether the specified action is disallowed. This function will only ever return true for a missing handler.
        //! The meaning of the input is defined by specific handler subclasses.
        virtual bool _IsRestrictedAction(uint64_t restrictedAction) const { return false; }

        //! Query whether this handler is substituting for one of its missing subclasses.
        virtual bool _IsMissingHandler() const { return false; }

        virtual ElementHandlerP _ToElementHandler() {return nullptr;}   //!< dynamic_cast this Handler to an ElementHandler
        virtual ModelHandlerP _ToModelHandler() {return nullptr;}       //!< dynamic_cast this Handler to a ModelHandler
        virtual CodeSpecHandlerP _ToCodeSpecHandler() {return nullptr;} //!< dynamic_cast this Handler to an CodeSpecHandler

        DGNPLATFORM_EXPORT static Handler& z_GetHandlerInstance(); //!< @private
        static Handler& GetHandler() {return z_GetHandlerInstance();}//!< @private
    }; // Handler

    //=======================================================================================
    //! A handler for a specfic SQLite table. Needed for processing ChangeSets.
    // @bsiclass                                                    Keith.Bentley   07/11
    //=======================================================================================
    struct TableHandler : NonCopyableClass
    {
    private:
        DgnDomainCP  m_domain;

    public:
        TableHandler() : m_domain(nullptr) {}
        void SetDomain(DgnDomain& domain) {m_domain = &domain;}
        virtual TxnTable* _Create(TxnManager&) const = 0;
    };

private:
    Readonly m_isReadonly;
    Required m_isRequired;
    BeFileName m_schemaRootDir;

    void SetSchemaRootDir(BeFileNameCR schemaRootDir) { m_schemaRootDir = schemaRootDir; }
    BeFileName GetSchemaPathname() const;
    bool ValidateSchemaPathname() const;

    ECN::ECSchemaPtr ReadSchema(ECN::ECSchemaReadContextR schemaContext) const;
    SchemaStatus ValidateSchema(ECN::ECSchemaCR schema, DgnDbCR dgndb) const;

protected:
    int32_t         m_version;
    Utf8String      m_domainName;
    Utf8String      m_domainDescr;
    bvector<Handler*> m_handlers;
    bvector<TableHandler*> m_tableHandlers;

    virtual ~DgnDomain() {}
    DgnDbStatus VerifySuperclass(Handler& handler);

    BeSQLite::DbResult LoadHandlers(DgnDbR) const;

    //! Called after this DgnDomain's schema has been imported into the supplied DgnDb for the first time
    //! @param[in] db The DgnDb into which the schema was imported.
    //! @note Domains are expected to override this method and use it to create required domain objects (like categories, for example).
    //! The method is only called the first time the domain schema is imported into the DgnDb.
    virtual void _OnSchemaImported(DgnDbR db) const {}

    //! Called after a DgnDb containing this schema is opened. Domains may register SQL functions in this method, for example.
    //! @param[in] db The DgnDb that was just opened.
    virtual void _OnDgnDbOpened(DgnDbR db) const {}

    //! Called when a DgnDb is about to be closed. The DgnDb is still valid, but is about to be closed.
    //! @param[in] db The DgnDb that is about to be closed.
    virtual void _OnDgnDbClose(DgnDbR db) const {}

    //! Implemented by the domain to provide the path of the schema on disk relative to the assets directory for the domain. 
    //! Note that the assets directory for the domain can be specified when the domain is registered, but typically defaults to 
    //! the location specified by the host application (@see DgnDomains::RegisterDomain()).
    virtual WCharCP _GetSchemaRelativePath() const = 0;

public:
    //! Construct a new DgnDomain.
    //! @param[in] name Domain name. Must match filename of ECSchema this domain handles.
    //! @param[in] descr A description of this domain. For information purposes only, not used internally.
    //! @param[in] version The version of this DgnDomain API.
    DgnDomain(Utf8CP name, Utf8CP descr, uint32_t version) : m_domainName(name), m_domainDescr(descr), m_isRequired(DgnDomain::Required::No), m_isReadonly(DgnDomain::Readonly::No) {m_version = version; }

    //! Get the name of this DgnDomain.
    Utf8CP GetDomainName() const {return m_domainName.c_str();}

    //! Get the description of this DgnDomain.
    Utf8CP GetDomainDescription() const {return m_domainDescr.c_str();}

    //! Get the version of this DgnDomain.
    int32_t GetVersion() const {return m_version;}

    //! Returns true if the domain is setup to be Readonly in this session. 
    bool IsReadonly() const { return m_isReadonly == Readonly::Yes; }

    //! Setup this domain to be read-only/read-write in this session. 
    void SetReadonly(Readonly isReadonly) { m_isReadonly = isReadonly; }

    //! Returns true if the domain is setup to be required in this session
    bool IsRequired() const { return m_isRequired == Required::Yes; }

    //! Setup this domain to be required/optional in this session
    void SetRequired(Required isRequired) { m_isRequired = isRequired; }

    //! Imports (or upgrades) the schema of this domain into the supplied DgnDb.
    //! @remarks 
    //! <ul>
    //! <li> Only used for cases where the schemas of an optional domain are to be imported in. In all other cases 
    //! domain schemas are imported or upgraded when the DgnDb is created or opened. 
    //! <li> It's the caller's responsibility to start a new transaction before this call and commit it after a successful 
    //! import. If an error happens during the import, the new transaction is abandoned within the call. 
    //! <li> Errors out if there are local changes (uncommitted or committed). These need to be flushed by committing 
    //! the changes if necessary, and then creating a revision. See @ref RevisionManager. 
    //! </ul>
    DGNPLATFORM_EXPORT SchemaStatus ImportSchema(DgnDbR dgndb);

    //! Returns true of the schema for this domain has been imported into the supplied DgnDb. 
    //! @remarks Only checks if the schema has been imported, and does not do any validation of 
    //! version. @see DgnDomains::ValidateSchemas(), DgnDomain::ImportSchema().
    DGNPLATFORM_EXPORT bool IsSchemaImported(DgnDbCR dgndb) const;

    DGNPLATFORM_EXPORT Handler* FindHandler(Utf8CP className) const;

    //! Register a Handler for an ECClass within this DgnDomain.
    //! @param[in] handler the Handler to register.
    //! @param[in] replace if true, handler should replace the current Handler and will fail if a Handler for this class is NOT already registered.
    //! If false, fails if a Handler for this class IS already registered.
    //! @note Before a Handler is registered, all of its superclass Handlers must also be registered (that is, Handlers must be
    //! registered in class hierarchy order: base classes before subclasses.)
    DGNPLATFORM_EXPORT DgnDbStatus RegisterHandler(Handler& handler, bool replace=false);

    //! Register a table handler with this DgnDomain.
    void RegisterTableHandler(TableHandler& handler) {handler.SetDomain(*this); m_tableHandlers.push_back(&handler);}

};

//=======================================================================================
//! The set of DgnDomains used by this DgnDb. This class also caches the DgnDomain::Handler to DgnDb-specific
//! DgnClassId lookups.
//! @see DgnDb::Domains, DgnDomain
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnDomains : DgnDbTable
{
    typedef bvector<DgnDomainCP> DomainList;
    typedef bmap<DgnClassId,DgnDomain::Handler*> Handlers;

private:
    friend struct DgnDb;
    friend struct DgnDomain;
    friend struct TxnManager;

    DomainList    m_domains;
    Handlers      m_handlers;
    SchemaUpgradeOptions m_schemaUpgradeOptions;
    bool          m_allowSchemaImport;
 
    void LoadDomain(DgnDomainR);
    void AddHandler(DgnClassId id, DgnDomain::Handler* handler) {m_handlers.Insert(id, handler);}
    BeSQLite::DbResult InsertHandler(DgnDomain::Handler& handler);
    bool GetHandlerInfo(uint64_t* restrictions, DgnClassId handlerId, DgnDomain::Handler& handler);
    BeSQLite::DbResult InsertDomain(DgnDomainCR);
    ECN::ECClassCP FindBaseOfType(DgnClassId subClassId, DgnClassId baseClassId);

    void OnDbOpened();
    void OnDbClose();
    void SyncWithSchemas();
    void DeleteHandlers();

    // Imports schemas of all required domains into the DgnDb. 
    SchemaStatus ImportSchemas();
    // Validates (and upgrades if necessary) domain schemas - used when the DgnDb is first opened up.
    SchemaStatus InitializeSchemas(SchemaUpgradeOptions const& schemaUpgradeOptions);
    // Upgrades schemas of all domains already imported into the DgnDb, and of any newly registered required domains. 
    SchemaStatus UpgradeSchemas();
    SchemaStatus ValidateSchemas();
    SchemaStatus DoValidateSchemas(bvector<ECN::ECSchemaPtr>* schemasToImport, bvector<DgnDomainP>* domainsToImport);
    SchemaStatus ValidateSchemaReferences(SchemaStatus& status, bvector<ECN::ECSchemaPtr>* schemasToImport, ECN::ECSchemaReadContextR schemaContext, bset<ECN::ECSchemaP>& validatedSchemas);
    static SchemaStatus DoValidateSchema(ECN::ECSchemaCR appSchema, bool isSchemaReadonly, DgnDbCR db);
    SchemaStatus DoImportSchemas(bvector<ECN::ECSchemaCP> const& schemasToImport, BeSQLite::EC::SchemaManager::SchemaImportOptions importOptions);
    SchemaStatus DoImportSchemas(bvector<ECN::ECSchemaPtr> const& schemasToImport, bvector<DgnDomainP> const& domainsToImport);
    ECN::ECSchemaReadContextPtr PrepareSchemaReadContext() const;

    explicit DgnDomains(DgnDbR db) : DgnDbTable(db), m_allowSchemaImport(true) {}

public:
    //! Look up a handler for a given DgnClassId. Does not check base classes.
    //! This is for internal use only, and #FindHandler should be used by client code.
    //! @private
    DgnDomain::Handler* LookupHandler(DgnClassId handlerId);

    //! Register a domain to be used for this session. This supplies all of the handlers for classes of that domain.
    //! @param[in] domain The domain to register. Domains are singletons and cannot change during a session.
    //! @param[in] isRequired Pass true to ensure/validate that all DgnDb-s that are created/opened in this 
    //! session have this domain enabled. Pass false if the domain is optional. 
    //! @param[in] isReadonly Pass true to use the domain only for reading instances, or false to allow
    //! all CRUD operations (assuming the DgnDb itself is writable)
    //! @param[in] schemaRootDir Optional root directory of Directory in which the assets for the domain are delivered. If not specified, 
    //! the directory provided by the DgnPlatform host is used (@see DgnPlatformLib::Host::IKnownLocationsAdmin::_GetDgnPlatformAssetsDirectory()). 
    //! Note that ECSchema-s for the domain may actually reside in some path relative to this assets directory, and this
    //! relative path is specified by the author of the domain (@see DgnDomain::_GetSchemaRelativePath()). 
    //! @remarks
    //! <ul>
    //! <li> If isRequired=Required::Yes, newly created DgnDb-s will have the ECSchema of the domain (and any dependencies)
    //! imported in, and enables use of the domain's CRUD API. Opening previously created DgnDb-s will trigger 
    //! validation of the ECSchema of the domain against the corresponding version in the DgnDb. Any subsequent 
    //! validation errors will cause the open to fail, but the issue may be resolvable by call to 
    //! DgnDomains::ImportSchemas() - the process however requires locking of the schemas, and is typically done 
    //! by a Project Administrator. 
    //! <li> If isRequired=Required::No, an explicit call to @ref DgnDomain::ImportSchema() is required to import the domain's 
    //! ECSchema into newly created DgnDbs. Like before, the call will require locking of the schemas, and 
    //! is typically done by a Project Administrator. 
    //! <li> If a domain is registered to be required, it's name is recorded in every DgnDb accessed during 
    //! the session, and becomes required to access that DgnDb in the future.
    //! <li> If a domain is registered as optional (not required), it's name is recorded in the DgnDb only if 
    //! the schema for the domain has been explicitly imported. Once that's done, the domain must be registered
    //! to access that DgnDb.
    //! </ul>
    DGNPLATFORM_EXPORT static BentleyStatus RegisterDomain(DgnDomain& domain, DgnDomain::Required isRequired = DgnDomain::Required::No, DgnDomain::Readonly isReadonly = DgnDomain::Readonly::No, BeFileNameCP schemaRootDir = nullptr);

    //! Look up a domain by name.
    //! @param[in] name The name of the domain to find.
    DGNPLATFORM_EXPORT DgnDomainCP FindDomain(Utf8CP name) const;

    //! Get the local (within this DgnDb) DgnClassId for the specified handler.
    DGNPLATFORM_EXPORT DgnClassId GetClassId(DgnDomain::Handler& handler);

    //! Find the DgnDomain::Handler for a DgnClassId within this DgnDb.
    //! If there is no handler registered for the supplied DgnClassId, recursively look for one on any of its base classes
    //! derived from the supplied baseClassId.
    //! @param[in] handlerId The DgnClassId for which the handler is desired.
    //! @param[in] baseClassId The root DgnClassId of the handler of interest. This method will walk through the base class hierarchy
    //! of handlerId towards baseClassId until it finds a registered handler.
    //! @note The DgnClassId @b is a ECClassId.
    DGNPLATFORM_EXPORT DgnDomain::Handler* FindHandler(DgnClassId handlerId, DgnClassId baseClassId);

    //! @private
    void DisableSchemaImport() {m_allowSchemaImport = false;}
    //! @private
    void EnableSchemaImport() {m_allowSchemaImport = true;}

};

struct CreateDgnDbParams;

//=======================================================================================
//! The DgnDomain for the base "dgn" schema.
//! @see DgnDbSqlFunctions for a list of built-in functions that you can call in SQL statements.
//! @ingroup GROUP_DgnDomain
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct BisCoreDomain : DgnDomain
{
    friend struct DgnDb;
    DOMAIN_DECLARE_MEMBERS(BisCoreDomain,DGNPLATFORM_EXPORT)

private:
    CreateDgnDbParams const* m_createParams = nullptr;

    void SetCreateParams(CreateDgnDbParams const& createParams) { m_createParams = &createParams; }

    WCharCP _GetSchemaRelativePath() const override { return BISCORE_ECSCHEMA_PATH; }
    void _OnDgnDbOpened(DgnDbR db) const override;
    void _OnSchemaImported(DgnDbR) const override;

public:
    BisCoreDomain();
};

END_BENTLEY_DGN_NAMESPACE
