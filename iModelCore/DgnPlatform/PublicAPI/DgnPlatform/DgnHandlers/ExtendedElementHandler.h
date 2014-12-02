/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ExtendedElementHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnCore/DisplayHandler.h>
//#include <DgnPlatform/DisplayFilter.h> removed in graphite

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
#ifdef WIP_VANCOUVER_MERGE // presentationgraphics
/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     11/2011
+===============+===============+===============+===============+===============+======*/
struct  PresentationGraphicsStroker : IStrokeForCache                             
{
ExtendedElementHandler&  m_handler;

PresentationGraphicsStroker (ExtendedElementHandler& handler) : m_handler (handler)  { }

DGNPLATFORM_EXPORT virtual void        PresentationGraphicsStroker::_StrokeForCache (ElementHandleCR elIter, ViewContextR viewContext, double pixelSize) override;
   
};  // PresentationGraphicsStroker
#endif
//__PUBLISH_SECTION_START__


/// @addtogroup DisplayHandler
/// @beginGroup

/*=================================================================================**//**
* The default type handler for the EXTENDED_ELM type that corresponds to the
* ExtendedElm structure.
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct          ExtendedElementHandler : DisplayHandler
{
    DEFINE_T_SUPER(DisplayHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ExtendedElementHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
#ifdef WIP_VANCOUVER_MERGE // presentationgraphics
    friend struct PresentationGraphicsStroker;
#endif
protected:
#ifdef WIP_VANCOUVER_MERGE // presentationgraphics
//! Generate presentation for an element when an element is created or modified.
//! Implementations should call the appropriate methods on viewContext and <code>viewContext->GetIDrawGeom()</code>
//! to generate the presentation.   
//!
//! @note     Implementations always need to call <code>context->GetIDrawGeom()</code> to get an IDrawGeom interface to output graphics.
//!           If your implementation doesn't call any methods in IDrawGeom, your element will have no presentation 
//            XGraphics and therefore is not visible.
//!
//! @note     Because this presentation will be used in many different views, it is important 
//!           to use the DisplayFilter API methods to selectively display geometry rather than testing conditions directly.
//! @param[in]      eh  The element to generate XGraphics representation for.
//! @param[in]      viewContext The context to use for generating XGraphics.
//! @return         SUCCESS if the XGraphics are generated sucessfully.
DGNPLATFORM_EXPORT virtual BentleyStatus _GeneratePresentation (ElementHandleCR eh, ViewContextR viewContext) { return ERROR; }

//! Get presentation transform for an object.   This is required for Instancing (see _InstancePresentation below) but 
//! valid for non-instanced presentations as well.
//! @param[out]     transform - the presentation transform (from the local system where the presentation is generated to the world).
//! @param[in]      eh  The element to get transform for.
//! @return         true if a valid presenttion transform is returned.  If false is returned then the presentation is displayed as generated (not tranformed).
DGNPLATFORM_EXPORT virtual bool _GetPresentationTransform (TransformR transform, ElementHandleCR eh) { return false; }

//! If the _InstancePresentation method returns true then the Presentation generated for this element are instancable. In this case, if this element handler
//! generates the same presentation (with different transforms) for more than one element then the geometry is automatically shared.  This instancing
//! is detected and tracked automatically using the same mechanism developed for IModel instancing.  This instancing is much more efficient than using
//! shared cells.
//! @param[in]      eh.  The element.
//! @return         true to generate instancable presentation.
DGNPLATFORM_EXPORT virtual bool _InstancePresentation (ElementHandleCR eh, bool& unsharedGraphicsPresent) { return true; }

//! Generate unshared presentation graphics.  This method is called only if the _InstancePresentation method returns true and 
//! sets the unsharedGraphicsPresent boolean argument to true.
//! @param[in]      eh  The element to generate XGraphics representation for.
//! @param[in]      viewContext The context to use for generating XGraphics.
//! @return         SUCCESS if the XGraphics are generated sucessfully.
DGNPLATFORM_EXPORT virtual BentleyStatus _GenerateUnsharedPresentation (ElementHandleCR eh, ViewContextR viewContext) { return ERROR; }

//! If the _AutoGeneratePresentation returns true then the presentation will be automatically generated when the element is initially created
//! or modified.  This is handled by the default transaction handler for ExtendedElements - if a different transaction handler is used or the
//! _AutoGeneratePresentation method returns false then the implementation should explicitly generate and maintain the presentation by
//! calling ValidatePresentation upon element creation or modification that requires graphics to be regenerated.
//! @param[in]      eh.  The element.
//! @param[out]      unsharedGraphicsPresent.  Set to true to indicate that unshared graphics are present.
//! @return true to indicate the presentation should be instancable.
DGNPLATFORM_EXPORT virtual bool _AutoGeneratePresentation (ElementHandleCR eh) { return true; }      

DGNPLATFORM_EXPORT virtual void _Draw (ElementHandleCR el, ViewContextR context) override;
#endif
// Handler
DGNPLATFORM_EXPORT virtual void    _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void    _GetDescription (ElementHandleCR el, WStringR string, UInt32 desiredLength) override;

DGNPLATFORM_EXPORT virtual void _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;
DGNPLATFORM_EXPORT virtual void _OnConvertTo2d (EditElementHandleR eeh, TransformCR flattenTrans, DVec3dCR flattenDir) override;

DGNPLATFORM_EXPORT virtual StatusInt _OnTransform (EditElementHandleR elHandle, TransformInfoCR tInfo) override;
DGNPLATFORM_EXPORT virtual bool      _IsRenderable (ElementHandleCR eh) override;

DGNPLATFORM_EXPORT virtual bool    _IsSupportedOperation (ElementHandleCP eh, SupportOperation stype) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public: 

//! Create a new EXTENDED_ELM with required fields for a displayable element initialized.
//! @param[out] eeh             The new element.
//! @param[in]  teh             Template element to use for symbology; if NULL defaults are used.
//! @param[in]  modelRef        Model to associate this element with.
//! @param[in]  is3d            Initialize the 2d or 3d element structure, typically modelRef->Is3d ().
//! @param[in]  isComplexHeader Whether element will be used as a complex header (normally false).
//! @note Used to create an extended displayable element by attaching an element handler XAttribute.
//! @bsimethod
DGNPLATFORM_EXPORT  static void InitializeElement (EditElementHandleR eeh, ElementHandleCP teh, DgnModelR model, bool is3d);

}; // ExtendedElementHandler
#if defined (NEEDS_WORK_DGNITEM)

/*=================================================================================**//**
* This is just an extended element where we register an extension to supply 
* a drag manipulator that has stretch handles.
* @bsiclass                                                     Sam.Wilson      09/2013
+===============+===============+===============+===============+===============+======*/
struct StretchableExtendedElementHandler : ExtendedElementHandler // added in graphite
    {
    DEFINE_T_SUPER(ExtendedElementHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (StretchableExtendedElementHandler, DGNPLATFORM_EXPORT)
    static ElementHandlerId GetElemHandlerId() {return ElementHandlerId(22296,0);} // *** WIP_NEED_HANDLERID - I am using XATTRIBUTEID_XGraphics temporarily, while I work on this.
    };

//=======================================================================================
// An "ogre" is an Optimized Graphics Replacement Element, created during the DgnDb
// publishing process to generate more performant graphics.  This handler has no special
// properties to distinguish it from the ExtendedElementHandler; it exists only to demarcate
// elements that were separated into smaller elements during publishing.  This handler marks
// the lead element of the assembly.
// @bsiclass                                                    MattGooding     05/13
//=======================================================================================
struct OgreLeadHandler : ExtendedElementHandler // added in graphite
{
    DEFINE_T_SUPER(ExtendedElementHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (OgreLeadHandler, DGNPLATFORM_EXPORT)

//__PUBLISH_SECTION_END__
protected:

// Handler
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
//__PUBLISH_SECTION_END__
DGNPLATFORM_EXPORT static void AttachToElement (EditElementHandleR eeh);

//__PUBLISH_SECTION_START__
//! @return This handler's ID.  Can be used with ElementHandle::GetHandler().GetHandlerId() to determine if an element is an optimized graphics replacement.
//! @bsimethod
DGNPLATFORM_EXPORT static ElementHandlerId GetElemHandlerId ();
};

//=======================================================================================
// An "ogre" is an Optimized Graphics Replacement Element, created during the DgnDb
// publishing process to generate more performant graphics.  This handler has no special
// properties to distinguish it from the ExtendedElementHandler; it exists only to demarcate
// elements that were separated into smaller elements during publishing.  This handler marks
// the non-lead members of the assembly.
// @bsiclass                                                    MattGooding     05/13
//=======================================================================================
struct OgreFollowerHandler : ExtendedElementHandler // added in graphite
{
    DEFINE_T_SUPER(ExtendedElementHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (OgreFollowerHandler, DGNPLATFORM_EXPORT)

    //__PUBLISH_SECTION_END__
protected:
    DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

    //__PUBLISH_SECTION_START__
    //__PUBLISH_CLASS_VIRTUAL__
public:
    //__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static void AttachToElement (EditElementHandleR eeh);

    //__PUBLISH_SECTION_START__
    //! @return This handler's ID.  Can be used with ElementHandle::GetHandler().GetHandlerId() to determine if an element is an optimized graphics replacement.
    //! @bsimethod
    DGNPLATFORM_EXPORT static ElementHandlerId GetElemHandlerId ();
};

/// @endGroup

/// @addtogroup Handler
/// @beginGroup
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  08/06
+===============+===============+===============+===============+===============+======*/
struct GroupDataHandler : Handler // graphite moved this here from type5handlers.h
{
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (GroupDataHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;
//__PUBLISH_SECTION_START__
};

/*=================================================================================**//**
* @bsiclass                                                     Sam.Wilson      10/2005
+===============+===============+===============+===============+===============+======*/
struct Type66Handler : Handler // graphite moved this here from type5handlers.h
{
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (Type66Handler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:
    DGNPLATFORM_EXPORT bool IsAppElement(DgnElementCP el, int signature);
    DGNPLATFORM_EXPORT virtual void  _GetTypeName (WStringR string, UInt32 desiredLength) override;
//__PUBLISH_SECTION_START__
};

/*=================================================================================**//**
* The default type handler for the EXTENDED_ELM type that corresponds to the
* ExtendedNonGraphicElm structure.
* @bsiclass                                                     Brien.Bastings  05/2007
+===============+===============+===============+===============+===============+======*/
struct ExtendedNonGraphicsHandler : Handler
{
//__PUBLISH_SECTION_END__
virtual UInt16 _GetV8ElementType() override {return 107;}
    DEFINE_T_SUPER(Handler)
    ELEMENTHANDLER_DECLARE_MEMBERS (ExtendedNonGraphicsHandler, DGNPLATFORM_EXPORT)
//__PUBLISH_SECTION_END__
protected:

DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;
DGNPLATFORM_EXPORT virtual void _QueryProperties (ElementHandleCR eh, PropertyContextR context) override;
DGNPLATFORM_EXPORT virtual void _EditProperties (EditElementHandleR eeh, PropertyContextR context) override;
//DGNPLATFORM_EXPORT virtual StatusInt _OnPreprocessCopy (EditElementHandleR element, ElementCopyContextP copyContext) override; removed in graphite
DGNPLATFORM_EXPORT virtual void _OnConvertTo3d (EditElementHandleR eeh, double elevation) override;

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__

public:

//! Create a new EXTENDED_ELM with required fields for a non-displayable element initialized.
//! @param[out] eeh             The new element.
//! @param[in]  teh             Template element to use for symbology; if NULL defaults are used.
//! @param[in]  modelRef        Model to associate this element with.
//! @param[in]  isComplexHeader Whether element will be used as a complex header (normally false).
//! @note Used to create an extended non-displayable element by attaching an element handler XAttribute.
//! @bsimethod
DGNPLATFORM_EXPORT static void InitializeElement (EditElementHandleR eeh, ElementHandleCP teh, DgnModelR model, bool isComplexHeader = 0);
};

struct NonGraphicsGenericGroupLeaderHandler : ExtendedNonGraphicsHandler // added in graphite
{
    DEFINE_T_SUPER(ExtendedNonGraphicsHandler)
    ELEMENTHANDLER_DECLARE_MEMBERS (NonGraphicsGenericGroupLeaderHandler, DGNPLATFORM_EXPORT)

//__PUBLISH_SECTION_END__
protected:
DGNPLATFORM_EXPORT virtual void _GetTypeName (WStringR string, UInt32 desiredLength) override;

public:
DGNPLATFORM_EXPORT static ElementHandlerId  GetElemHandlerId ();

//__PUBLISH_SECTION_START__
//__PUBLISH_CLASS_VIRTUAL__
public:
//! Create a new non-graphical generic group leader element.
//! @param[out] eeh             The new element.
//! @param[in]  teh             Template element to use for symbology; if NULL defaults are used.
//! @param[in]  modelRef        Model to associate this element with.
//! @bsimethod
DGNPLATFORM_EXPORT static void InitializeElement (EditElementHandleR eeh, ElementHandleCP teh, DgnModelR model);
};
#endif

/// @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
