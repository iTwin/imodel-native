/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/ITextEdit.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <DgnPlatform/DgnHandlers/TextBlock/TextBlockAPI.h>
#include <Bentley/bvector.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! Ref-counted wrapper for ITextPartId.
typedef RefCountedPtr<ITextPartId> ITextPartIdPtr;

//! Ref-counted wrapper for ITextQueryOptions.
typedef RefCountedPtr<ITextQueryOptions> ITextQueryOptionsPtr;

//! A collection of ITextPartIds.
//! \see    ITextPartId ITextQuery ITextEdit
typedef bvector<ITextPartIdPtr> T_ITextPartIdPtrVector;

//! \see T_ITextPartIdPtrVector
typedef T_ITextPartIdPtrVector* T_ITextPartIdPtrVectorP;

//! \see T_ITextPartIdPtrVector
typedef T_ITextPartIdPtrVector& T_ITextPartIdPtrVectorR;

//! \see T_ITextPartIdPtrVector
typedef T_ITextPartIdPtrVector const * T_ITextPartIdPtrVectorCP;

//! \see T_ITextPartIdPtrVector
typedef T_ITextPartIdPtrVector const & T_ITextPartIdPtrVectorCR;

//=======================================================================================
// This class exists so that handlers can have their own private way of describing the text within them, and guarantees whatever they hand out is reference counted.
// @bsiclass                                                    Jeff.Marker     08/2009
//=======================================================================================
struct ITextPartId : public RefCountedBase
    {
//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT static ITextPartIdPtr Create ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    }; // ITextPartId

//=======================================================================================
//! Allows you to pre-filter the ITextPartId collection. This can be used to increase performance by preventing the need to create unnecessary ITextPartId or TextBlock objects.
// @bsiclass                                                    Jeff.Marker     01/2011
//=======================================================================================
struct ITextQueryOptions : public RefCountedBase, public NonCopyableClass
    {
//__PUBLISH_SECTION_END__
    private:    bool    m_includeEmptyParts;
    private:    bool    m_requireFieldSupport;

    private: ITextQueryOptions ();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! By default, empty pieces of text are not returned; this will force ITextPartId objects to be created for them (may result in NULL TextBlock objects when the text part is requested).
    public: DGNPLATFORM_EXPORT bool ShouldIncludeEmptyParts () const;

    //! Sets whether to allow empty text parts.
    //! @see    ShouldIncludeEmptyParts
    public: DGNPLATFORM_EXPORT void SetShouldIncludeEmptyParts (bool);

    //! By default, all text parts will be returned, regardless of their field support; this will restrict to ITextPartId objects that represent only parts that can technically support fields. This does NOT mean that the returned text parts actually contain fields; merely that they potential support fields.
    public: DGNPLATFORM_EXPORT bool ShouldRequireFieldSupport () const;

    //! Sets whether to restrict to text parts that can support fields.
    //! @see    ShouldRequireFieldSupport
    public: DGNPLATFORM_EXPORT void SetShouldRequireFieldSupport (bool);

    //! Creates a new instance of this object, with defaults (see the various getter methods for defaults).
    public: DGNPLATFORM_EXPORT static ITextQueryOptionsPtr CreateDefault ();

    }; // ITextQueryOptions

//=======================================================================================
// Element handlers implement this interface to expose formatted text in a read-only fashion. The handler will return one or more generic reference counted objects known as ITextPartId; these have no meaning to users of the API other than to feed them back into this interface. The general workflow is to query the handler for its ITextPartId(s), and then ask for a TextBlock representing a specific ITextPartId. Some element handlers implement ITextEdit instead (a sub-class of this class) to indicate that they also support text write operations. Some element handlers expose multiple pieces of text (e.g. dimenions); text elements (see ITextQuery::IsTextElement) will only ever expose a single piece of text. Regardless of whether the element returns a single ITextPartId or not, you must still ask for a ITextPartId, and provide one for all operations.
// @bsiclass                                                    Venkat.Kalyan   01/07
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ITextQuery
    {
private:
    DECLARE_KEY_METHOD

//__PUBLISH_SECTION_END__

    //! Note to implementors: The default implementation returns false. DO NOT override and/or return true.
    protected:  DGNPLATFORM_EXPORT virtual bool            _IsTextElement  (ElementHandleCR) const;
    protected:  DGNPLATFORM_EXPORT virtual bool            _DoesSupportFields   (ElementHandleCR) const;
    protected:  DGNPLATFORM_EXPORT virtual ITextPartIdPtr  _GetTextPartId  (ElementHandleCR, HitPathCR) const = 0;
    protected:  DGNPLATFORM_EXPORT virtual void            _GetTextPartIds (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const = 0;
    protected:  DGNPLATFORM_EXPORT virtual TextBlockPtr    _GetTextPart    (ElementHandleCR, ITextPartIdCR) const = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Determines if this element is a standard DGN text element (e.g. type 7 or 17). Many element types support ITextQuery, but it is sometimes useful to know if the element is generic text as opposed to some other element type that exposes formatted text. If a handler returns true, it is guaranteed to provide a single ITextPartId when queried.
    public: DGNPLATFORM_EXPORT bool IsTextElement (ElementHandleCR) const;

    //! Determines if this element supports fields.
    public: DGNPLATFORM_EXPORT bool DoesSupportFields (ElementHandleCR) const;
    
    //! Gets the ITextPartId for the piece of text indicated by the hitpath (or NULL). This is most useful for non generic DGN text element handlers, which can store more than just text in them.
    public: DGNPLATFORM_EXPORT ITextPartIdPtr GetTextPartId (ElementHandleCR, HitPathCR) const;

    //! Gets ITextPartIds for all pieces of text in this element. They are NOT guaranteed to be provided in any specific order.
    //! @note   ITextQueryOptions::CreateDefault will use default values suitable for the vast majority of situations.
    public: DGNPLATFORM_EXPORT void GetTextPartIds (ElementHandleCR, ITextQueryOptionsCR, T_ITextPartIdPtrVectorR) const;

    //! Generates and returns a TextBlock representing the provided ITextPartId.
    //! \note   This means that the TextBlock is in no way directly tied to actual element data; see ITextEdit::ReplaceTextPart to modify an element's text.
    public: DGNPLATFORM_EXPORT TextBlockPtr GetTextPart (ElementHandleCR, ITextPartIdCR) const;

    }; // ITextQuery

//=======================================================================================
// Element handlers implement this interface to expose formatted text in a read/write fashion. This interface cannot be used to create text, but merely to replace. Additionally, this interface is not responsible for deleting elements when asked to replace with a blank TextBlock; check the return status if this a potential case for you, and manually delete. As an extension to ITextQuery, the handler will return one or more generic reference counted objects known as ITextPartId; these have no meaning to users of the API other than to feed them back into this interface. The general workflow is to query the handler for its ITextPartId(s), ask for a TextBlock representing a specific ITextPartId, modify it, and then replace that same ITextPartId with your modified TextBlock. Regardless of whether the element returns a single ITextPartId or not, you must still ask for a ITextPartId, and provide one for all operations.
// @bsiclass                                                    Venkat.Kalyan   01/07
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ITextEdit : public ITextQuery
    {
    //=======================================================================================
    // Describes the outcome of the ITextEdit::ReplaceText method.
    // @bsiclass                                                    Venkat.Kalyan   01/07
    //=======================================================================================
    public: enum ReplaceStatus
        {
        ReplaceStatus_Success   = 0,    //!< The handler has successfully replaced the indicated text in the element provided by the handle.
        ReplaceStatus_Error     = 1,    //!< The handler could NOT replace the indicated text in the element provided by the handle for an unknown reason.
        ReplaceStatus_Delete    = 2     //!< The handler did not replace the indicated text in the element, because it is now your responsibility to delete the element.

        }; // ReplaceStatus

//__PUBLISH_SECTION_END__

    protected: DGNPLATFORM_EXPORT virtual ReplaceStatus _ReplaceTextPart (EditElementHandleR, ITextPartIdCR, TextBlockCR) = 0;

    DECLARE_KEY_METHOD

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

    //! Replaces an existing piece of text with a new one.
    //! \note   The TextBlock provided will in no way be directly tied to actual element data; if you want to modify again, you will have to call this method subsequent times.
    public: DGNPLATFORM_EXPORT ReplaceStatus ReplaceTextPart (EditElementHandleR, ITextPartIdCR, TextBlockCR);

    }; // ITextEdit

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
