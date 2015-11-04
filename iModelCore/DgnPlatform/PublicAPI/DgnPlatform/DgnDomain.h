/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnDomain.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    protected: virtual Dgn::DgnDomain::Handler* _CreateMissingHandler(uint64_t restrictions) override {return new Dgn::DgnDomain::MissingHandler<__classname__>(restrictions, *this);}\
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
    public: static BentleyStatus RegisterExtension(DgnDomain::Handler& handler, __classname__& obj) {return obj.RegisterExt(handler,z_Get##__classname__##Token());}\
            static BentleyStatus DropExtension(DgnDomain::Handler& handler) {return DropExt(handler,z_Get##__classname__##Token());}\
            static __classname__* Cast(DgnDomain::Handler& handler) {return (__classname__*) CastExt(handler,z_Get##__classname__##Token());}

// This macro must be included somewhere within a source file that implements a DgnDomain::Handler::Extension
#define HANDLER_EXTENSION_DEFINE_MEMBERS(__classname__) \
    DgnDomain::Handler::Extension::Token& __classname__::z_Get##__classname__##Token(){static DgnDomain::Handler::Extension::Token* s_token=0; if (0==s_token) s_token = NewToken(); return *s_token;}

#define TABLEHANDLER_DECLARE_MEMBERS(__classname__,__exporter__) \
    public:  __exporter__ static __classname__& GetHandler();

// This macro must be included somewhere within a source file that implements a DgnDomain::TableHandler
#define TABLEHANDLER_DEFINE_MEMBERS(__classname__) \
    __classname__&  __classname__::GetHandler(){static __classname__* s_instance=nullptr; if (nullptr==s_instance) s_instance=new __classname__(); return *s_instance;}

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnDomains;

/** @addtogroup DgnDomainGroup

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

The DgnDomain for the base "dgn" schema is is called DgnBaseDomain. It is always loaded and it registers all of its DgnDomain::Handlers.

*/

struct DgnDomains;

//=======================================================================================
//! A DgnDomain is a singleton C++ object that provides the runtime implementation for an ECSchema.
//! A given DgnDomain supplies set of "Handlers," each of which are singleton C++ objects derived from DgnDomain::Handler,
//! that provide the implementation for one of its ECClasses. Stated differently, a DgnDomain is a collection of DgnDomain::Handlers
//! for its ECClasses. It is not possible for one DgnDomain to supply a DgnDomain::Handler for a different DgnDomain.
//! Domains are "registered" at program startup time (via that static method DgnDomains::RegisterDomain),
//! remain for an entire session, and apply to all DgnDb's that are opened
//! or created during that session. If a DgnDomain is registered, its name is recorded in every DgnDb accessed during the session
//! and becomes required to access that DgnDb in the future.
//! @ingroup DgnDomainGroup
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnDomain : NonCopyableClass
{
    friend struct DgnDomains;

    //! The current version of the HandlerAPI
    enum {API_VERSION = 1};

    struct Handler;

    //! A template used to create a proxy handler of a superclass when the handler subclass cannot be found at run-time.
    //! The proxy handler can restrict the behavior of the superclass according to restrictions imposed by the subclass.
    template<typename T> struct MissingHandler : T
    {
    private:
        uint64_t m_restrictions;

        virtual DgnDomain::Handler* _CreateMissingHandler(uint64_t restrictions) override { return T::_CreateMissingHandler(restrictions); }
        virtual bool _IsRestrictedAction(uint64_t restrictedAction) const override { return 0 != (m_restrictions & restrictedAction); }
        virtual bool _IsMissingHandler() const override { return true; }
    public:
        explicit MissingHandler(uint64_t restrictions, T& base) : m_restrictions(restrictions)
            {
            this->m_domain = &base.GetDomain();
            this->m_superClass = &base;
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

        /**
    @verbatim
    struct ExampleInterface : DgnDomain::Handler::Extension
        {
        HANDLER_EXTENSION_DECLARE_MEMBERS (ExampleInterface,)
        virtual void _DoExample(ElementHandleCR) = 0;
        };
    HANDLER_EXTENSION_DEFINE_MEMBERS(ExampleInterface)
        @endverbatim
        You can then implement your interface on many classes, e.g.:
        @verbatim
    struct Example1 : ExampleInterface
        {
        virtual void _DoExample(DgnElementCR) override {printf("Example1");}
        };
    struct Example2 : ExampleInterface
        {
        virtual void _DoExample(DgnElementCR) override {printf("Example2");}
        };
        @endverbatim
        Then, register your Handler::Extension on an existing Handler by calling the Handler::Extension's "RegisterExtension" method.
        For example, to register your extension on ModelHandler, use:
        @verbatim
    ExampleInterface::RegisterExtension (ModelHandler::Handler(), *new Example1());
        @endverbatim
        A Handler can have many registered Handler::Extensions, but can only be extended by one instance of a given Handler::Extension. Therefore:
        @verbatim
    status = ExampleInterface::RegisterExtension(ModelHandler::Handler(), *new Example1()); // SUCCESS
    status = ExampleInterface::RegisterExtension(ModelHandler::Handler(), *new Example2()); // ERROR - already extended with Example1!
        @endverbatim
        Will fail. However, you can add your extension at any level in the Handler class hierarchy. So:
        @verbatim
    ExampleInterface::RegisterExtension(ModelHandler::Handler(), *new Example1());
    ExampleInterface::RegisterExtension(Model2dHandler::Handler(), *new Example2());
        @endverbatim
        Will extend all ModelHandler classes with "Example1", but the Model2dHandler class (which is a subclass of ModelHandler)
        with "Example2".<p>
        You can then look up your extension on a Handler by calling the Handler::Extension's "Cast" method. E.g.:
        @verbatim
    void doExample (DgnElementCR el)
        {
        ExampleInterface* exampleExt = ExampleInterface::Cast(el.GetElementHandler());
        if (NULL != exampleExt)
            exampleExt->_DoExample(el);
        }
        @endverbatim
        This will print "Example2" for all Model2ds and "Example1" for all other types of Models.<p>
        To remove a Handler::Extension, call "DropExtension". E.g.:
        @verbatim
    ExampleInterface::DropExtension (LineHandler::Handler());
        @endverbatim
        */
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
        virtual Handler* _CreateMissingHandler(uint64_t restrictions) { return new MissingHandler<Handler>(restrictions, *this); }
        virtual uint64_t _ParseRestrictedAction(Utf8CP restriction) const { return RestrictedAction::Parse(restriction); }

        Handler* GetRootClass();
    public:
        //! To enable version-checking for your handler, override this method to report the
        //! API version that was used to compiler your handler.
        //! @remarks Version-checking is a convenience to developers during the product development
        //!          cycle. It allows one to detect when a handler needs to be recompiled
        //!          in order to catch up with API changes.
        //! @remarks To override this method, simply copy the following line into your handler.
        virtual uint32_t _GetApiVersion() {return API_VERSION;}

        Handler* GetSuperClass() const {return m_superClass;}

        //! Get the name of the ECClass handled by this Handler
        Utf8StringCR GetClassName() const {return m_ecClassName;}

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

        virtual ElementHandlerP _ToElementHandler() {return nullptr;}       //!< dynamic_cast this Handler to an ElementHandler
        virtual ModelHandlerP _ToModelHandler() {return nullptr;}           //!< dynamic_cast this Handler to a ModelHandler
        virtual ViewHandlerP _ToViewHandler() {return nullptr;}             //!< dynamic_cast this Handler to a ViewHandler
        virtual AuthorityHandlerP _ToAuthorityHandler() {return nullptr;}   //!< dynamic_cast this Handler to an AuthorityHandler

        static Handler& z_GetHandlerInstance(); //!< @private
        DGNPLATFORM_EXPORT static Handler& GetHandler() {return z_GetHandlerInstance();}//!< @private
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
        virtual struct TxnTable* _Create(TxnManager&) const = 0;
    };

protected:
    int32_t         m_version;
    Utf8String      m_domainName;
    Utf8String      m_domainDescr;
    bvector<Handler*> m_handlers;
    bvector<TableHandler*> m_tableHandlers;
    virtual ~DgnDomain() {}
    DgnDbStatus VerifySuperclass(Handler& handler);

    BeSQLite::DbResult LoadHandlers(DgnDbR) const;

    //! Called after this DgnDomain's schema has been imported into the supplied DgnDb.
    //! @param[in] db The DgnDb into which the schema was imported.
    //! @note Domains are expected to override this method and use it to create required domain objects (like categories, for example).
    //! @see ImportSchema
    virtual void _OnSchemaImported(DgnDbR db) const {}

    //! Called after a DgnDb containing this schema is opened. Domains may register SQL functions in this method, for example.
    //! @param[in] db The DgnDb that was just opened.
    virtual void _OnDgnDbOpened(DgnDbR db) const {}

    //! Called when a DgnDb is about to be closed. The DgnDb is still valid, but is about to be closed.
    //! @param[in] db The DgnDb that is about to be closed.
    virtual void _OnDgnDbClose(DgnDbR db) const {}

public:
    //! Construct a new DgnDomain.
    //! @param[in] name Domain name. Must match filename of ECSchema this domain handles.
    //! @param[in] descr A description of this domain. For information purposes only, not used internally.
    //! @param[in] version The version of this DgnDomain API.
    DgnDomain(Utf8CP name, Utf8CP descr, uint32_t version) : m_domainName(name), m_domainDescr(descr) {m_version=version;}

    //! Get the name of this DgnDomain.
    Utf8CP GetDomainName() const {return m_domainName.c_str();}

    //! Get the description of this DgnDomain.
    Utf8CP GetDomainDescription() const {return m_domainDescr.c_str();}

    //! Get the version of this DgnDomain.
    int32_t GetVersion() const {return m_version;}

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

    //! Import an ECSchema for this DgnDomain.
    //! @param[in] db Import the domain schema into this DgnDb
    //! @param[in] schemaFileName The domain ECSchema file to import
    DGNPLATFORM_EXPORT DgnDbStatus ImportSchema(DgnDbR db, BeFileNameCR schemaFileName) const;

    //! Import an ECSchema for this DgnDomain.
    //! @param[in] db Import the domain schema into this DgnDb
    //! @param[in] schemaCache The ECSchemaCache containing the schema to import
    DGNPLATFORM_EXPORT DgnDbStatus ImportSchema(DgnDbR db, ECN::ECSchemaCacheR schemaCache) const;

};

//=======================================================================================
//! The set of DgnDomains used by this DgnDb. This class also caches the DgnDomain::Handler to DgnDb-specific
//! DgnClassId lookups.
//! @see DgnDb::Domains, DgnDomain
//! @ingroup DgnDomainGroup
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnDomains : DgnDbTable
{
    typedef bvector<DgnDomainCP> DomainList;
    typedef bmap<DgnClassId,DgnDomain::Handler*> Handlers;

private:
    friend struct DgnDb;
    friend struct DgnDomain;
    friend struct ComponentModel;

    DomainList    m_domains;
    Handlers      m_handlers;

    void LoadDomain(DgnDomainR);
    void AddHandler(DgnClassId id, DgnDomain::Handler* handler) {m_handlers.Insert(id, handler);}
    BeSQLite::DbResult InsertHandler(DgnDomain::Handler& handler);
    bool GetHandlerInfo(uint64_t* restrictions, DgnClassId handlerId, DgnDomain::Handler& handler);
    BeSQLite::DbResult InsertDomain(DgnDomainCR);
    ECN::ECClassCP FindBaseOfType(DgnClassId subClassId, DgnClassId baseClassId);
    BeSQLite::DbResult OnDbOpened();
    void OnDbClose();
    void SyncWithSchemas();

    explicit DgnDomains(DgnDbR db) : DgnDbTable(db) {}

public:
    //! Look up a handler for a given DgnClassId. Does not check base classes.
    //! This is for internal use only, and #FindHandler should be used by client code.
    //! @private
    DgnDomain::Handler* LookupHandler(DgnClassId handlerId);

    //! Register a domain to be used for this session. This supplies all of the handlers for classes of that domain.
    //! @param[in] domain The domain to register. Domains are singletons and cannot change during a session.
    //! @note This call must be made before any DgnDbs are created or opened.
    DGNPLATFORM_EXPORT static void RegisterDomain(DgnDomain& domain);

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
};

//=======================================================================================
//! The DgnDomain for the base "dgn" schema.
//! @see DgnDbSqlFunctions for a list of built-in functions that you can call in SQL statements.
//! @ingroup DgnDomainGroup
// @bsiclass                                                    Keith.Bentley   02/11
//=======================================================================================
struct DgnBaseDomain : DgnDomain
{
    DOMAIN_DECLARE_MEMBERS(DgnBaseDomain,DGNPLATFORM_EXPORT)

    void _OnDgnDbOpened(DgnDbR db) const override;

public:
    DgnBaseDomain();
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
