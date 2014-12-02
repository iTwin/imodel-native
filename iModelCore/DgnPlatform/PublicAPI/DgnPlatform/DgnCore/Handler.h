/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/Handler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#if defined (_MSC_VER)
    #pragma warning (disable:4265) // Class has virtual function, but destructor is not virtual
#endif // defined (_MSC_VER)

#include "DgnDomain.h"

//__PUBLISH_SECTION_END__
DGNPLATFORM_TYPEDEFS (ISubTypeHandlerQuery)
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A transformation matrix. Identifies a few special cases.
// @bsiclass
//=======================================================================================
struct TransformInfo
{
private:
    Transform               m_trans;
    UInt32                  m_options;
    AnnotationScaleAction   m_annotationScaleAction;
    bool                    m_haveMirrorPlane;
    RotMatrix               m_mirrorPlane;
    double                  m_annotationScale;

public:
    //! Initialize to the identity transform
    DGNPLATFORM_EXPORT TransformInfo ();

    //! Initialize to the specified transform
    DGNPLATFORM_EXPORT explicit TransformInfo (TransformCR t);

    //! Get pointer to the transformation matrix
    TransformCP GetTransform () const {return &m_trans;}

    //! Get a reference to the transformation matrix
    TransformR GetTransformR () {return m_trans;}

    //! If this is a mirroring transform, get the plane across which the element is to be mirrored.
    RotMatrixCP GetMirrorPlane () const {return m_haveMirrorPlane ? &m_mirrorPlane : NULL;}

    //! If this is a mirroring transform, set the plane across which the element is to be mirrored.
    void SetMirrorPlane (RotMatrixCR mirrorPlane) {m_mirrorPlane = mirrorPlane, m_haveMirrorPlane = true;}

    //! Set element-specific transform options. @see TransformOptionValues
    void SetOptions (UInt32 options) {m_options = options;}

    //! Check element-specific transform options. @see TransformOptionValues
    UInt32 GetOptions () const {return m_options;}

    //! Set annotation scale action
    void SetAnnotationScaleAction (AnnotationScaleAction action) {m_annotationScaleAction = action;}

    //! Get annotation scale action
    AnnotationScaleAction GetAnnotationScaleAction () const {return m_annotationScaleAction;}

    //! Set annotation scale
    void SetAnnotationScale (double annotationScale) {m_annotationScale = annotationScale;}

    //! Get annotation scale
    double GetAnnotationScale () const {return m_annotationScale;}
};

struct  IGeoCoordinateReprojectionHelper;


//__PUBLISH_SECTION_END__
/*  This becomes the Published version of this macro - DO NOT CHANGE this commented out stuff!!!
__PUBLISH_SECTION_START__
#define ELEMENTHANDLER_DECLARE_MEMBERS(__classname__,__exporter__) \
    public:    __exporter__ static __classname__& GetInstance();
__PUBLISH_SECTION_END__
*/

// This macro must be included within the class declaration of an ElementHandler.
#define ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR(__classname__,__exporter__) \
    private:   __exporter__ static __classname__*& z_PeekInstance(); \
                            static __classname__* z_CreateInstance(); \
    public:    __exporter__ static __classname__& GetInstance() {return z_Get##__classname__##Instance();}\
                            static __classname__& ReplaceInstance() {return *(z_PeekInstance()=z_CreateInstance());}\
               __exporter__ static __classname__& z_Get##__classname__##Instance();

#define ELEMENTHANDLER_DECLARE_MEMBERS(__classname__,__exporter__) \
    protected: __classname__() {}\
    ELEMENTHANDLER_DECLARE_MEMBERS_NO_CTOR(__classname__,__exporter__)

// This macro must be included within the source file that implements an ElementHandler
#define ELEMENTHANDLER_DEFINE_MEMBERS(__classname__) \
    __classname__*  __classname__::z_CreateInstance() {__classname__* instance= new __classname__(); instance->SetSuperClass(&T_Super::GetInstance()); return instance;}\
    __classname__*& __classname__::z_PeekInstance() {static __classname__* s_instance = 0; return s_instance;}\
    __classname__&  __classname__::z_Get##__classname__##Instance(){__classname__*& instance=z_PeekInstance(); if (0 == instance) instance=z_CreateInstance(); return *instance;}

#define ELEMENTHANDLER_INSTANCE(__classname__) __classname__::z_Get##__classname__##Instance()
//__PUBLISH_SECTION_START__

#if !defined (DOCUMENTATION_GENERATOR)
#define ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS(__classname__,__exporter__) \
    private: __exporter__ static Token& z_Get##__classname__##Token();\
    public: static BentleyStatus RegisterExtension  (HandlerR handler, __classname__& obj) {return obj.RegisterExt(handler,z_Get##__classname__##Token());}\
            static BentleyStatus DropExtension      (HandlerR handler) {return DropExt(handler,z_Get##__classname__##Token());}\
            static __classname__* Cast              (HandlerR handler) {return (__classname__*) CastExt (handler,z_Get##__classname__##Token());}
#endif

#define ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(__classname__) \
    Handler::Extension::Token& __classname__::z_Get##__classname__##Token(){static Handler::Extension::Token* s_token=0; if (0==s_token) s_token = NewToken(); return *s_token;}

/*=================================================================================**//**
  @addtogroup ElementHandler
An Element Handler defines the behavior of the elements that it controls, including how they
display, transform, respond to snaps, and other user interactions.

An element handler is C++ singleton object that is an instance of a class that is derived from Handler.

A Handler's methods normally take an ElementHandle or an EditElementHandle reference as an argument,
which identifies the element that the Handler should process. A Handler has no state of its own.
A Handler is expected to apply its logic to the element that it is asked to process,
based on the state of the element. The Handler design is based on the Flyweight Pattern.

  @bsiclass
*//*+===============+===============+===============+===============+===============+======*/

/// @beginGroup

struct  IGeometryQuery;

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum class SupportOperation
{
    Selection             = 0, // Can be part of selection set
    CellGroup             = 1, // Can be added to orphan cell group
    CellUnGroup           = 2, // Can be un-grouped...i.e. is orphan cell group
    TransientManipulators = 3, // Can support manipulators with transient elements
    CacheCutGraphics      = 4, // Should cache cut graphics (only called if notRenderable) Shared cell instances need to cache their own cuts.
    LineStyle             = 5, // Element type supports line styles
    CustomHilite          = 6, // Element handler wants to control it's hilited presentation, DrawContext won't setup override symbology.
};

/*=================================================================================**//**
 Handler defines the standard queries and operations available on all elements,
 whether graphical or non-graphical, internal or application defined.
 Every element in MicroStation has a Handler. A handler may have more capabilities
 than what is defined by Handler, but it will have at least these capabilities.
 @see ElementHandle::GetHandler
 @bsiclass                                                     SamWilson       11/04
+===============+===============+===============+===============+===============+======*/
class EXPORT_VTABLE_ATTRIBUTE Handler : NonCopyableClass
{
public:

    /*=================================================================================**//**
      A Handler::Extension can be used to add additional interfaces to a Handler at runtime. If a Handler is
      extended, all of its registered subclasses inherit that extension too.
      To implement a Handler::Extension, derive from that class and put the ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS macro in
      your class declaration and the ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS in your implementation. E.g.:

@verbatim
struct ExampleInterface : Handler::Extension
    {
    ELEMENTHANDLER_EXTENSION_DECLARE_MEMBERS (ExampleInterface,)
    virtual void _DoExample(ElementHandleCR) = 0;
    };
ELEMENTHANDLER_EXTENSION_DEFINE_MEMBERS(ExampleInterface)
    @endverbatim
    You can then implement your interface on many classes, e.g.:
    @verbatim
struct Example1 : ExampleInterface
    {
    virtual void _DoExample(ElementHandleCR) override {printf("Example1");}
    };
struct Example2 : ExampleInterface
    {
    virtual void _DoExample(ElementHandleCR) override {printf("Example2");}
    };
    @endverbatim
    Then, register your Handler::Extension on an existing Handler by calling the Handler::Extension's "RegisterExtension" method.
    For example, to register your extension on LineHandler, use:
    @verbatim
ExampleInterface::RegisterExtension (LineHandler::GetInstance(), *new Example1());
    @endverbatim
    A Handler can have many registered Handler::Extensions, but can only be extended by one instance of a given Handler::Extension. Therefore:
    @verbatim
status = ExampleInterface::RegisterExtension (LineHandler::GetInstance(), *new Example1()); // SUCCESS
status = ExampleInterface::RegisterExtension (LineHandler::GetInstance(), *new Example2()); // ERROR - already extended with Example1!
    @endverbatim
    Will fail. However, you can add your extension at any level in the Handler class hierachy. So:
    @verbatim
ExampleInterface::RegisterExtension (DisplayHandler::GetInstance(), *new Example1());
ExampleInterface::RegisterExtension (LineHandler::GetInstance(), *new Example2());
    @endverbatim
    Will extend all DisplayHandler classes with "Example1", but the LineHandler class (which is a subclass of DisplayHandler)
    with "Example2".<p>
    You can then look up your extension on a Handler by calling the Handler::Extension's "Cast" method. E.g.:
    @verbatim
void doExample (ElementHandleCR eh)
    {
    ExampleInterface* exampleExt = ExampleInterface::Cast(eh.GetHandler());
    if (NULL != exampleExt)
        exampleExt->_DoExample(eh);
    }
    @endverbatim
    This will print "Example2" for all LineHandler elements and "Example1" for all other types of DisplayHandler elements.<p>
    To remove a Handler::Extension, call "DropExtension". E.g.:
    @verbatim
ExampleInterface::DropExtension (LineHandler::GetInstance());
    @endverbatim
    * @bsiclass                                                     Keith.Bentley   09/09
    +===============+===============+===============+===============+===============+======*/
    struct Extension
    {
        friend class Handler;
        struct Token
        {
            friend struct Extension;
            private: Token () {}
        };

#if !defined (DOCUMENTATION_GENERATOR)
    protected:
        BentleyStatus RegisterExt (Handler& handler, Token& extensionToken) {return handler.AddExtension(extensionToken, *this);}
        static BentleyStatus DropExt (Handler& handler, Token& extensionToken) {return handler.DropExtension(extensionToken);}
        static Extension* CastExt (Handler& handler, Token& extensionToken) {return handler.FindExtension(extensionToken);}
        static Token* NewToken () {return new Token();}
#endif
    };

private:
    DGNPLATFORM_EXPORT BentleyStatus AddExtension (Extension::Token&, Extension&);
    DGNPLATFORM_EXPORT BentleyStatus DropExtension (Extension::Token&);
    DGNPLATFORM_EXPORT Extension* FindExtension (Extension::Token&);

    struct ExtensionEntry;

//__PUBLISH_SECTION_END__
    friend struct PersistentElementRef;

private:
    struct ExtensionEntry
    {
        ExtensionEntry (Extension::Token& token, Extension& extension, ExtensionEntry* next) : m_token(token), m_extension(extension), m_next(next){}
        static ExtensionEntry* Find(ExtensionEntry*, Extension::Token const&);

        Extension::Token&   m_token;
        Extension&          m_extension;
        ExtensionEntry*     m_next;
    };

//__PUBLISH_SECTION_START__
    Handler*            m_superClass;
    ExtensionEntry*     m_extensions;
    DgnDomainP          m_domain;
protected:
    ElementHandlerId    m_handlerId;
private:
//__PUBLISH_SECTION_END__

    static Handler*  z_CreateInstance ();
    static Handler*& z_PeekInstance ();

protected:
    Handler () {m_superClass = (Handler*) 0xbadf00d; m_extensions=NULL; m_domain=NULL; }
    virtual ~Handler(){}
    void SetSuperClass (Handler* super) {m_superClass = super;}

public:
//! The current version of the HandlerAPI
enum {API_VERSION = 1};
enum PreActionStatus
    {
    PRE_ACTION_Ok    = 0,
    PRE_ACTION_Block = 1,
    };

    UInt32 GetApiVersion ();
    static void locked_StaticInitializeDomains();
    static void locked_StaticInitializeRegisterHandlers();

    Handler* GetSuperClass () {return m_superClass;}

protected:
    //! To enable version-checking for your handler, override this method to report the
    //! API version that was used to compiler your handler.
    //! @remarks Version-checking is a convenience to developers during the product development
    //!          cycle. It allows MicroStation to detect when a handler needs to be recompiled
    //!          in order to catch up with API changes.
    //! @remarks If you do not override this method, MicroStation will not be able to check the
    //!          version of your handler.
    //! @remarks To override this method, simply copy the following line into your handler.
    virtual UInt32 _GetApiVersion () {return API_VERSION;}

    /// @name Transform and Fence Operations
    //@{

    //! Transform the element's data.
    //! @return SUCCESS if the element was transformed.
    //! @param[in]      element         The element to be transformed.
    //! @param[in]      transform       The transform to be applied.
    //! @remarks Called by ApplyTransform
    //! @remarks
    //!   Subclasses should override this callback to apply the operation to the
    //!   element data that is specific to the type, sub-type, or instance.
    //!   The subclass implementation should not attempt to process public linkages.
    //!   The subclass implementation should not attempt to recompute the element's range.
    //!   <p><em>The subclass implementation should begin by calling the superclass OnTransform method.</em>
    //! @see
    //!   ApplyTransform
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual StatusInt _OnTransform (EditElementHandleR element, TransformInfoCR transform);

    //! Called after all OnTransform methods have been called.
    //! @return SUCCESS if the transform could be finished
    //! @param[in]      element         The element that has been transformed.
    //! @param[in]      transform       The transform that has been applied.
    //! @return non-zero error code if the transform could not be finished.
    //! @remarks Called by ApplyTransform
    //! @remarks
    //!   Subclasses will not normally override this callback.
    //! @see
    //!   ApplyTransform
    //!   OnTransform
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual StatusInt _OnTransformFinish (EditElementHandleR element, TransformInfoCR transform);

    //! Apply fence stretch to the element's data.
    //! @param[in]      element         Element to be tested and possibly stretched.
    //! @param[in]      transform       The transform to apply to the portions of the element that meet the clip criteria.
    //! @param[in]      fp              The fence parameters to apply.
    //! @param[in]      options
    //! @remarks Called by FenceStretch
    //! @return SUCCESS if the element could be stretched.
    //! @remarks
    //!   This method is normally invoked in the case that the fence overlaps
    //!   the element. FenceStretch will handle the special cases of
    //!   the element being entirely inside our entirely outside of the fence.
    //! @remarks
    //!   This method is not called if a SYSTEM_FENCE_STRETCH asynch handled the fence stretch operation.
    //! @remarks
    //!   Subclasses should override this callback to apply the operation to the
    //!   element data that is specific to the type, sub-type, or instance.
    //!   The subclass implementation should not attempt to process public linkages.
    //!   The subclass implementation should not attempt to recompute the element's range.
    //!   <p><em>The subclass implementation should begin by calling the superclass OnFenceStretch method.</em>
    //! @see
    //!   FenceStretch
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual StatusInt _OnFenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options);

    //! Called after all OnFenceStretch methods have been called
    //! @param[in]      element         Element that has been stretched.
    //! @param[in]      transform       The transform to apply to the portions of the element that meet the clip criteria.
    //! @param[in]      fp              The fence parameters to apply.
    //! @param[in]      options
    //! @remarks Called by FenceStretch
    //! @return non-zero error code if the fence stretch could not be finished
    //! @remarks
    //!   This method is not called if a SYSTEM_FENCE_STRETCH asynch handled the fence stretch operation.
    //! @remarks
    //!   Subclasses will not normally override this callback.
    //! @see
    //!   FenceStretch
    //!   OnFenceStretch
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual StatusInt _OnFenceStretchFinish (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options);

    //! The handler is requested to clip the specified element, that is, to
    //!   return the portions of the element that are inside/outside the clip criteria.
    //!   status parameter will be set to the outcome of the operation.
    //! @param[in]      insideElm      Part of element inside fence.
    //! @param[in]      outsideElm     Part of element outside fence.
    //! @param[in]      element        Element to clip.
    //! @param[in]      fp             The fence parameters to apply.
    //! @param[in]      options
    //! @return SUCCESS if the element could be clipped.
    //! @remarks Called by FenceClip
    //! @remarks
    //!   This method is not called if a SYSTEM_FENCE_CLIP asynch handled the fence clip operation.
    //! @remarks
    //!   Subclasses should override this callback to perform the \em entire clip operation.
    //!   The subclass implementation should \em not normally call the superclass OnFenceClip method.
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual StatusInt _OnFenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options);

    //! The handler is requested to return a 3d representation of itself.
    //! @param[in]      eeh             Element to be converted.
    //! @param[in]      elevation       Z value to use for converted element.
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual void _OnConvertTo3d (EditElementHandleR eeh, double elevation);

    //! The handler is requested to return a 2d representation of itself.
    //! @param[in]      eeh             Element to be converted.
    //! @param[in]      flattenTrans    Transform that can be applied to smash element to a plane (may contain 2d rotation).
    //! @param[in]      flattenDir      Viewing direction for converted element.
    //! @remarks
    //!   Handlers that choose to implement this method without calling ApplyTransform and
    //!   also support associative patterns or custom linestyles need to make sure these
    //!   linkages get properly handled.
    //! @bsimethod
    DGNPLATFORM_EXPORT virtual void _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir);

    //@}

    /// @name Property query and mutate
    //@{
    DGNPLATFORM_EXPORT virtual void _QueryProperties (ElementHandleCR eh, PropertyContextR context);
    DGNPLATFORM_EXPORT virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context);
    //@}

    virtual bool _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) {return false;}
    virtual DisplayHandlerP _GetDisplayHandler () {return NULL;}
    DGNPLATFORM_EXPORT virtual StatusInt _ApplyTransform (EditElementHandleR element, TransformInfoCR transform);
    DGNPLATFORM_EXPORT virtual bool _IsTransformGraphics (ElementHandleCR element, TransformInfoCR transform);
    DGNPLATFORM_EXPORT virtual StatusInt _FenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options);
    DGNPLATFORM_EXPORT virtual StatusInt _FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options);
    DGNPLATFORM_EXPORT virtual void _GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength);
    virtual void _GetTypeName (WStringR string, UInt32 desiredLength) {string.clear();}
    virtual void _GetTypeDescription (WStringR string, UInt32 desiredLength) {string.clear();}
    virtual StatusInt _OnDecorate (ElementHandleCR, ViewContextP) { return ERROR; }

public:
    //! Called when a new instance of this handler is about to be added to a DgnFile.
    //! @param[in] el the instance of the element to be added. The handler may make changes to the data in the element, if necessary.
    //! @return PRE_ACTION_Ok to allow the add to proceed or PRE_ACTION_Block to stop it from happening.
    virtual PreActionStatus _OnAdd (EditElementHandleR el) {return PRE_ACTION_Ok;}

    //! Called when an instance of this handler is about to be deleted from a DgnFile.
    //! @param[in] el the ElementRef of the element to be deleted.
    //! @return PRE_ACTION_Ok to allow the delete to proceed or PRE_ACTION_Block to stop it from happening.
    virtual PreActionStatus _OnDelete (PersistentElementRefR el) {return PRE_ACTION_Ok;}

    //! Called when an instance of this handler is about to be modified with a changed version.
    //! @param[in] newEl the new instance of the element to replace the current data. The handler may make changes to the data in the element, if necessary.
    //! @param[in] oldEl the existing instance of the element that will be replaced.
    //! @return PRE_ACTION_Ok to allow the replace to proceed or PRE_ACTION_Block to stop it from happening.
    virtual PreActionStatus _OnModify (EditElementHandleR newEl, ElementHandleCR oldEl) {return PRE_ACTION_Ok;}

    //! Called after a new instance of this handler has been successfully added to a DgnFile.
    //! @param[in] el The ElementRef of the new instance of this handler.
    virtual void _OnAdded (PersistentElementRefR el) {}

    //! Called after an instance of this handler has been successfully deleted from a DgnFile.
    //! @param[in] dgnFile the DgnFile from which the instance was deleted.
    //! @param[in] el the ElementId of the now-deleted element.
    virtual void _OnDeleted (DgnProjectR dgnFile, ElementId el) {}

    //! Called after an existing instance of this handler has been successfully modified with a new instance of this handler.
    //! @param[in] newEl The ElementRef of the replaced element.
    virtual void _OnModified (PersistentElementRefR newEl) {}

    //! Called on a transaction boundary when a new instance of this handler was added in the Txn.
    //! @param[in] el the ElementRef of the element that was added during this transaction. 
    //! @note This method is called after the entire transaction has completed. All changes to persistent data (including this
    //! and all other changed elements) have been saved to the database.
    virtual void _OnTxnBoundary_Add (PersistentElementRefR el) {}

    //! Called on a transaction boundary when an instance of this handler was deleted in the Txn.
    //! @param[in] project the DgnProject from which the instance was deleted.
    //! @param[in] el the ElementId of the now-deleted element.
    //! @note This method is called after the entire transaction has completed. All changes to persistent data (including this
    //! and all other changed elements) have been saved to the database.
    virtual void _OnTxnBoundary_Delete (DgnProjectR project, ElementId el) {}

    //! Called on a transaction boundary when an instance of this handler was modified in the Txn.
    //! @param[in] el the ElementRef of the element that was modified during this transaction. 
    //! @note This method is called after the entire transaction has completed. All changes to persistent data (including this
    //! and all other changed elements) have been saved to the database.
    virtual void _OnTxnBoundary_Modify (PersistentElementRefR) {}

    //! Called when an instance of this handler was added in a transaction that is about to be reversed. That means, this instance
    //! is about to be removed from the database by the transaction manager. 
    //! @param[in] el The ElementRef of the element that was added during the about-to-be-reversed transaction. 
    //! @param[in] isUndo The direction the transaction is being reversed. If true, the transaction is about to be rolled 
    //! backwards (an undo of an add), if false it is being rolled forward (a redo of a delete).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReverseXXX methods before the transaction is reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager will reverse all modifications that were
    //! made by the original transaction leaving the database exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReverse_Add (PersistentElementRefR el, bool isUndo) {}

    //! Called when an instance of this handler was deleted in a transaction that is about to be reversed. That means, this instance
    //! is currently deleted, but is about to be re-inserted into the database by the transaction manager. 
    //! @param[in] project the DgnProject from which the instance was deleted.
    //! @param[in] el the ElementId of the now-deleted but about-to-be-revived element.
    //! @param[in] isUndo The direction the transaction is being reversed. If true, the transaction is about to be rolled 
    //! backwards (an undo of an delete), if false it is being rolled forward (a redo of an add).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReverseXXX methods before the transaction is reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager will reverse all modifications that were
    //! made by the original transaction leaving the database exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReverse_Delete (DgnProjectR project, ElementId el, bool isUndo) {}

    //! Called when an instance of this handler was modified in a transaction that is about to be reversed. That means, this instance
    //! is currently in its post-changed state, but is about to put back to its pre-changed state into the database by the transaction manager. 
    //! @param[in] el The ElementRef of the element that was modified during the about-to-be-reversed transaction. 
    //! @param[in] isUndo The direction the transaction is being reversed. If true, the transaction is about to be rolled 
    //! backwards (an undo of a modify), if false it is being rolled forward (a redo of a modify).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReverseXXX methods before the transaction is reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager will reverse all modifications that were
    //! made by the original transaction leaving the database exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReverse_Modify (PersistentElementRefR el, bool isUndo) {}

    //! Called when an instance of this handler was added in a transaction that has now been reversed. That means, the instance
    //! is currently deleted.
    //! @param[in] project the DgnProject from which the instance was deleted.
    //! @param[in] el the ElementId of the now-deleted element.
    //! @param[in] isUndo The direction the transaction was reversed. If true, the transaction was rolled 
    //! backwards (an undo of a add), if false it was being rolled forward (a redo of a delete).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReversedXXX methods after the transaction has been reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager reversed all modifications that were
    //! made by the original transaction so the database is now exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReversed_Add (DgnProjectR project, ElementId el, bool isUndo) {}

    //! Called when an instance of this handler was deleted by a transaction that has now been reversed. That means, the instance
    //! appears as it did before the original transaction that deleted it.
    //! @param[in] el The ElementRef of the element that was undeleted by reversing the transaction. 
    //! @param[in] isUndo The direction the transaction was reversed. If true, the transaction was rolled 
    //! backwards (an undo of a delete), if false it was being rolled forward (a redo of an add).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReversedXXX methods after the transaction has been reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager reversed all modifications that were
    //! made by the original transaction so the database is now exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReversed_Delete (PersistentElementRefR el, bool isUndo) {}

    //! Called when an instance of this handler was modified by a transaction that has now been reversed. That means, the instance
    //! is currently in its pre-changed (before the transaction started) state.
    //! @param[in] el The ElementRef of the element that was modified by the transaction. 
    //! @param[in] isUndo The direction the transaction was reversed. If true, the transaction was rolled 
    //! backwards (an undo of a modify), if false it was being rolled forward (a redo of the modify).
    //! @note All handlers of all elements affected by a transaction are notified with _OnReversedXXX methods after the transaction has been reversed.
    //! @note This method may NOT make any changes to persistent data. The transaction manager reversed all modifications that were
    //! made by the original transaction so the database is now exactly as it existed before the transaction started. 
    //! This event is useful to clean up any in-memory references to the element. 
    virtual void _OnTxnReversed_Modify (PersistentElementRefR el, bool isUndo) {}

protected:
    DGNPLATFORM_EXPORT virtual ScanTestResult _DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP);
    virtual bool _HandlerDummy3(void*) {return false;}
    virtual bool _HandlerDummy2(void*) {return false;}
    virtual bool _HandlerDummy1(void*) {return false;}

public:
    DGNPLATFORM_EXPORT ElementHandlerId GetHandlerId() const;
    DGNPLATFORM_EXPORT DgnDomainP GetDgnDomain() const;
    void SetDgnDomain(DgnDomainR, ElementHandlerId);

    //! Query if handler is suitable for specified purpose.
    //! @param[in]      stype           Purpose handler is being considered for.
    //! @param[in]      eh              Element data (May be NULL).
    //! @return true if handler acceptable for purpose, false otherwise.
    //! @bsimethod
    DGNPLATFORM_EXPORT bool IsSupportedOperation (ElementHandleCP eh, SupportOperation stype);

    DGNPLATFORM_EXPORT ScanTestResult DoScannerTests (ElementHandleCR eh, BitMaskCP levelsOn, UInt32 const* classMask, ViewContextP);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

#if !defined (DOCUMENTATION_GENERATOR)
    static Handler& z_GetHandlerInstance ();
    DGNPLATFORM_EXPORT static Handler& GetInstance () {return z_GetHandlerInstance();}
#endif

    /// @name Dynamic Cast Functions
    //@{

    //! Use this method instead of dynamic_cast<DisplayHandlerP> (handler)
    //! @return \c this if the class is derived from DisplayHandler; NULL otherwise.
    DGNPLATFORM_EXPORT DisplayHandlerP GetDisplayHandler ();
    //@}

    //__PUBLISH_SECTION_END__
    /// @name Lifecycle
    DGNPLATFORM_EXPORT StatusInt CallOnDecorate (ElementHandleCR, ViewContextP);


    //__PUBLISH_SECTION_START__
    /// @name Transform and Fence Operations
    //@{

    //! Transform the element.
    //! @return SUCCESS if the element could be transformed.
    //! @param[in]      element         The element to be transformed.
    //! @param[in]      transform       The transform to be applied.
    //! @remarks
    //!   Even though \em element may have an associated DgnModelP, this method should \em not
    //!   base its logic in any way on that or any other particular model.
    //!   This method applies to the element's intrinsic geometry, which can be viewed through
    //!   many model refs.
    //! @remarks
    //!   Invokes the OnTransform method. Subclasses should override OnTransform to apply
    //!   the transform to the element data that they control.
    //! @see
    //!   OnTransform
    //!   OnTransformFinish
    //! @bsimethod
    DGNPLATFORM_EXPORT StatusInt ApplyTransform (EditElementHandleR element, TransformInfoCR transform);

    //__PUBLISH_SECTION_END__

    //! This method queries if transforming the element itself and then calling Draw would
    //!   yield the same graphics as transforming the results of Draw.
    //!
    //! That is, this queries if the following operations generate the same graphics:
    //! \li Transform the element and then draw it.
    //! \li Draw the element and then transform the drawn graphics.
    //!
    //! @param[in]      element         The element to be transformed or drawn with the specified transform.
    //! @param[in]      transform       The transform to be applied to the element or to its graphics.
    //! @return true if transforming the element yields the same graphics as transforming the results of Draw.
    //! @bsimethod
    DGNPLATFORM_EXPORT bool IsTransformGraphics (ElementHandleCR element, TransformInfoCR transform);

    //__PUBLISH_SECTION_START__

    //! The handler is requested to "stretch" the specified element, that is, to
    //!   transform the portions of the element that meet the clip criteria. If entire
    //!   element is inside fence it should be transformed.
    //!   status parameter will be set to the outcome of the operation.
    //! @param[in]      element         The element to stretch.
    //! @param[in]      transform       The transform to apply to the portions of the element that meet the clip criteria
    //! @param[in]      fp              The fence parameters to apply.
    //! @param[in]      options
    //! @return SUCCESS if the element could be stretched.
    //! @remarks
    //!   Normally, subclasses should \em not override this method, but should override OnFenceStretch instead.
    //! @remarks
    //!   This method handles special cases, as follows:
    //!@verbatim
    //!  Call mdlClip_isElemInsideExt
    //!
    //!  if element is entirely outside the fence
    //!      return SUCCESS;
    //!
    //!  if element is entirely inside the fence
    //!      return ApplyTransform (element, transform)
    //!
    //!  Otherwise, the element overlaps the fence.
    //!
    //!  return OnFenceStretch
    //! @endverbatim
    //! @see
    //!   OnFenceStretch
    //!   OnFenceStretchFinish
    //! @bsimethod
    DGNPLATFORM_EXPORT StatusInt FenceStretch (EditElementHandleR element, TransformInfoCR transform, FenceParamsP fp, FenceStretchFlags options);

    //! The handler is requested to clip the specified element, that is, to
    //!   return the portions of the element that are inside/outside the clip criteria.
    //!   status parameter will be set to the outcome of the operation.
    //! @param[out]     inside           Part(s) of element inside fence.
    //! @param[out]     outside          Part(s) of element outside fence.
    //! @param[in]      element          The element to clip
    //! @param[in]      fp               The fence parameters to apply
    //! @param[in]      options
    //! @return SUCCESS if the element could be clipped.
    //! @remarks
    //!   Normally, subclasses should \em not override this method, but should override OnFenceClip instead.
    //! @remarks
    //!   This method checks for system asynchs and handles special cases, as follows:
    //!@verbatim
    //!  if (callAsynchs == true && some SYSTEM_FENCE_CLIP asynch returns SUCCESS)
    //!      return SUCCESS
    //!
    //!  If optimized clipping is enabled
    //!      if optimized clipping succeeds
    //!          return SUCCESS
    //!  otherwise
    //!      return OnFenceClip

    //! @endverbatim
    //! @see
    //!   OnFenceClip
    //! @bsimethod
    DGNPLATFORM_EXPORT StatusInt FenceClip (ElementAgendaP inside, ElementAgendaP outside, ElementHandleCR element, FenceParamsP fp, FenceClipFlags options);

    //! Convert a 2d element to its 3d form.
    //! @param[out]     eeh             The element to convert.
    //! @param[in]      elevation       The z component to apply to 2d point data.
    //! @bsimethod
    DGNPLATFORM_EXPORT void ConvertTo3d (EditElementHandleR eeh, double elevation);

    //! Convert a 3d element to its 2d form.
    //! @param[out]     eeh             The element to convert.
    //! @param[in]      flattenTrans    Transform to apply to flatten the element to a plane.
    //! @param[in]      flattenDir      Direction used to compute flatten transform.
    //! @note A 3d only element like CONE_ELM will be converted into a cell containing curve geometry.
    //! @bsimethod
    DGNPLATFORM_EXPORT void ConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir);

    //@}

    //! Get a brief string describing the element.
    //! @param    el        IN the element for this handler.
    //! @param    string    OUT description string to be filled in.
    //! @param    desiredLength IN the largest number of characters the caller wishes to see in the description. Implementers should
    //!   endeavor to honor this if possible, and should return the most elaborate description possible within
    //!   the desired length. However, implementers should never truncate the description to nonsense, even if the minimum sensible length exceeds
    //!   desiredLength. Callers must be prepared for strings that exceed desiredLength.
    //! @remarks
    //!   el may point to a changed or history state of the element's data.
    //!   This happens when GetDescription is called by design history.
    //! @bsimethod
    DGNPLATFORM_EXPORT void GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength);

    //! Get the type name for this handler. Currently used to populate the select tool's
    //! element type category. Handlers that want their elements selected by type need to
    //! implement this method. A sub-type handler should return a unique name and not the
    //! base class name.
    //! @param[out]     string          Description string to be filled in.
    //! @param[in]      desiredLength   The largest number of characters the caller wishes to see in the description.
    //! @bsimethod
    DGNPLATFORM_EXPORT void GetTypeName (WStringR string, UInt32 desiredLength);
    DGNPLATFORM_EXPORT void GetTypeDescription (WStringR string, UInt32 desiredLength);

    /// @name Property query and mutate
    //@{

    //! Method for enummerating the common properties of elements such as color and level.
    //! The supplied PropertyContext holds an object that supports the IQueryProperties
    //! interface. The query object tells the handler what properties it is interested in and
    //! the handler then announces them to the query object through callback methods.
    //! @param[in]      eh          The element to extract the properties of.
    //! @param[in]      context     The property context that holds the IQueryProperties object.
    //! @see PropertyContext IQueryProperties
    //! @bsimethod
    DGNPLATFORM_EXPORT void QueryProperties (ElementHandleCR eh, PropertyContextR context);

    //! Method for changing the common properties of elements such as color and level.
    //! The supplied PropertyContext holds an object that supports the IEditProperties
    //! interface. The edit object tells the handler what properties it is interested in and
    //! the handler then announces them to the edit object through callback methods. The
    //! edit object can decided to replace the property value with a new value which the
    //! handler will then use to update itself.
    //! @param[out]     eeh         The element to update the properties of.
    //! @param[in]      context     The property context that holds the IEditProperties object.
    //! @see PropertyContext IEditProperties
    //! @bsimethod
    DGNPLATFORM_EXPORT void EditProperties (EditElementHandleR eeh, PropertyContextR context);

//@}
}; // Handler


/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
