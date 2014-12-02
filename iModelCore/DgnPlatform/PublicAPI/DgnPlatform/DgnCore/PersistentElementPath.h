/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PersistentElementPath.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/RefCounted.h>
#include "../DgnPlatform.h"
#include <Bentley/bvector.h>
#include <Bentley/bset.h>
#include <Bentley/WString.h>
#include <functional>
#include "DisplayPath.h"

BENTLEY_API_TYPEDEFS(DataInternalizer)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

typedef bset<ElementRefP>   T_StdElementRefSet;
typedef bvector<UInt64>     T_ElementIdInternalVector;

/*=================================================================================**//**
PersistentElementPath captures a reference to an element.

PersistentElementPath is most often used by an XAttribute that wants to store a pointer
to an element. A PersistentElementPath can be embedded in the data of an XAttribute.
PersistentElementPath implements methods to help the XAttribute to support copy (deep-copying and pointer remapping)
and dependencies. Note that PersistentElementPath is just a struct, not an XAttribute. You must embed instances
of PersistentElementPath in the data of your own XAttributes. Or, you can use PersistentElementPathXAttributeHandler for
a ready-made handler.
<p>
A PersistentElementPath captures a path from one element to another. The start of the
path is called the \em dependent element. The dependent element is contained in the <em>dependent model</em>.
The destination of the path is called the \em root element. The root element is contained in the root model.
When you evaluate a PersistentElementPath, you specify the dependent model, because that is where the path starts.

<h4>Evaluating a Path</h4>
Evaluating a PersistentElementPath means looking up the root element. When you evaluate a PersistentElementPath,
you specify the dependent model. PersistentElementPath::GetDisplayPath is a method to reconstruct a DisplayPath.
If the path to the root element is not important, you can call PersistentElementPath::EvaluateElement to get the root element and its model.
<p>
<h4>Persistence</h4>
<p>
Whenever you need to
access a PersistentElementPath in a persistent XAttribute, you must call PersistentElementPath::Load,
to allow it to restore its state from the raw data stored in the XAttribute.
<p>
This class uses a compressed format for storage that uses just 5 bytes to capture
the common case of a reference to a local element in typical files.

* @see DisplayPath
* @bsiclass                                                     Sam.Wilson   12/04
See miscdev/mdl/examples/PersElmPath for an example.
+===============+===============+===============+===============+===============+======*/
struct PersistentElementPath
{
public:
    //! DisplayPath copy options
    //! @see OnPreprocessCopy
    enum            CopyOption
    {
        //! If the root is also being copied, then remap the pointer to the copy of the root; else, maintain the reference to the original.
        COPYOPTION_RemapRootsWithinSelection = 1,

        //! Non-model root: deep-copy between files; preserve current root within a file.
        //! Non-graphic root: deep-copy between models; preserve current root within a model.
        //! Graphic root: same as COPYOPTION_RemapRootsWithinSelection.
        COPYOPTION_DeepCopyAcrossFiles = 2,

        //! Non-model and non-graphic root: deep-copy always.
        //! Non-graphic root: deep-copy between models; preserve current root within a model.
        //! Graphic root: same as COPYOPTION_RemapRootsWithinSelection.
        COPYCONTEXT_DeepCopyRootsAlways = 3,
      };

private:
    UInt8*          m_storage;

    void Clear ();
    void Copy (PersistentElementPath const&);

public:

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT
    void GetInternalIds (T_ElementIdInternalVector&) const;
//__PUBLISH_SECTION_START__
    
    //! Destructor
    DGNPLATFORM_EXPORT
    ~PersistentElementPath ();

    //! Copy constructor
    //! @param[in]  cc      Persistent path to copy.
    DGNPLATFORM_EXPORT
    PersistentElementPath (PersistentElementPath const& cc);

    //! assignment operator
    //! @param[in]  cc      Persistent path to copy.
    DGNPLATFORM_EXPORT
    PersistentElementPath& operator= (PersistentElementPath const& cc);

///@name Construction
///@{
    //! Use this constructor to create a PersistentElementPath that identifies an element
    //! @param[in]  path    Identifies an element and a topological feature on that element.
    //! @remarks PersistentElementPath saves everything up to and including the cursor element. It ignores everything after the cursor.
    DGNPLATFORM_EXPORT explicit PersistentElementPath (DisplayPathCP path);

    //! Use this constructor to create a PersistentElementPath that identifies a single element in a known model.
    //! @param[in]  rootElement     the root element.
    //! @remarks This constructor does not capture a path, but just the ElementId of the root element.
    //! @remarks This is for the special case where:
    //! \li a DisplayPath is not available
    //! \li the root element is in a known model.
    DGNPLATFORM_EXPORT explicit PersistentElementPath (ElementRefP rootElement);

    //! Use this constructor to create a PersistentElementPath that identifies a root element.
    //! @param[in]  rootModel   The model that holds the root element.
    //! @param[in]  rootElement The root element.
    //! This is a convenience method and is equivalent to:
    //! \code
    //! DisplayPath dp (rootElement, rootModel);
    //! PersistentElementPath (&dp);
    //! \endcode
    DGNPLATFORM_EXPORT PersistentElementPath (DgnModelP rootModel, ElementRefP rootElement);

    //! @param[in]  mid              ModelId that contains element
    //! @param[in]  eid              ElementId of element
    DGNPLATFORM_EXPORT PersistentElementPath (DgnModelId mid, ElementId eid);

//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT PersistentElementPath (DgnModelP rootModel, ElementId rootElementID);

    DGNPLATFORM_EXPORT PersistentElementPath (ElementId rootElementID);
//__PUBLISH_SECTION_START__

///@name Serialization
///@{
    //! Use this constructor to just before calling Load
    DGNPLATFORM_EXPORT
    PersistentElementPath ();

//__PUBLISH_SECTION_END__
    //! Recreate a PersistentElementPath from stored state
    //! @param[in]  source  Previously stored state.
    //! @returns non-zero error status if the state is invalid
    //! @see Store
    DGNPLATFORM_EXPORT
    StatusInt Load (DataInternalizer& source);
//__PUBLISH_SECTION_START__

    //! Recreate a PersistentElementPath from stored state
    //! @param[in]  bytes   Previously stored state.
    //! @param[in]  nbytes  Number of bytes in \a bytes.
    //! @returns non-zero error status if the state is invalid
    //! @see Store
    DGNPLATFORM_EXPORT
    StatusInt Load (byte const* bytes, size_t nbytes);

    //! Recreate a PersistentElementPath from stored state
    //! @param[in]  source  Previously stored state.
    //! @returns non-zero error status if the state is invalid
    //! @see Load, ToString
    DGNPLATFORM_EXPORT
    StatusInt FromUtf8String (Utf8CP source);

    //! @deprecated FromUtf8String
    StatusInt FromWString (WCharCP source) {return FromUtf8String (Utf8String (source).c_str());}

//__PUBLISH_SECTION_END__
    //! Use this constructor to recreate a PersistentElementPath from stored state
    //! This is equivalent to using the default constructor and then calling ::Load
    //! @param[in]  source  Persistent data to which state was previously stored.
    //! @see Store Load GetDisplayPath
    DGNPLATFORM_EXPORT
explicit
    PersistentElementPath (DataInternalizer& source);

    //! Serialize the state of this PersistentElementPath
    //! @param[in]  soml    Where state is to be stored.
    //! @see Load
    DGNPLATFORM_EXPORT
    void Store (DataExternalizer* sink) const;
//__PUBLISH_SECTION_START__

    //! Serialize the state of this PersistentElementPath
    //! @param[in]  sink    Where state is to be stored.
    //! @see Load
    DGNPLATFORM_EXPORT
    void Store (bvector<byte>& sink) const;

    //! Serialize the state of this PersistentElementPath
    //! @returns string that captures the persistent state of PersistentElementPath
    //! @see Store, FromString
    DGNPLATFORM_EXPORT
    Utf8String ToUtf8String () const;

    //! @deprecated ToUtf8String
    WString ToWString () const {return WString (ToUtf8String().c_str(), BentleyCharEncoding::Utf8);}
///@}

///@name Evaluation
///@{

    //! Resolve the reference to the root element.
    //! @param[in]  dependentModel   The model that contains the dependent element.
    //! @returns NULL if the root element is not found or not available.
    DGNPLATFORM_EXPORT
    DisplayPathPtr GetDisplayPath (DgnModelP dependentModel) const;

    //! Get the root element.
    //! @param[in]  dependentModel   The model that contains the dependent element.
    //! @returns an ElementHandle that identifies the root element or an invalid handle if the path could not be evaluated.
    DGNPLATFORM_EXPORT
    ElementHandle EvaluateElement (DgnModelP dependentModel) const;

    //! Get the root element.
    //! @returns an ElementHandle that identifies the root element or an invalid handle if the path could not be evaluated.
    //! @param[in]  referenceHolder    Identifies the dependent element. This is normally the element that owns the XAttribute that contains this PeristentElementPath.
    //! @remarks The returned EditElementHandle will have a NULL DgnModel if \a referenceHolder has a NULL DgnModel.
    //! <p> This function is useful if you have an ElementHandle that identifies the dependent element element but a) you are not
    //! sure if the handle contains a model or an element ref, and b) you don't care if the returned handle has a model or not.
    //! \li If \a referenceHolder has a DgnModel, then this function calls EvaluateElement (DgnModelP).
    //! \li Else, if \a referenceHolder has an ElementRefP, then this function calls EvaluateElementRef.
    //! \li Else, this function returns an invalid EditElementHandle.
    DGNPLATFORM_EXPORT
    ElementHandle EvaluateElementFromHost (ElementHandleCR referenceHolder) const;

    //! Get the root element.
    //! @param[in]  dependentModel    The model that contains the dependent element.
    //! @returns an element handle that identifies the root element or an invalid handle if the path could not be evaluated.
    //! @remarks
    //!     This evaluation function is useful when you don't need a path and you don't have a modelRef.
    //!     EvaluateElementRef will return NULL if the root element is deleted.
    DGNPLATFORM_EXPORT
    ElementRefP EvaluateElementRef (DgnModelP dependentModel) const;

    //! Processor called on the items in a PersistentElementPath
    struct  PathProcessor
        {
        //! An ElementId was encountered. ref is NULL if element is unresolved. Processing stops when NULL element is encountered.
        virtual void OnElementId (ElementId e, ElementRefP ref, DgnModelP dependentModel) = 0;
        //! An optional dependent model or optional root model id was encountered. cacheFound is NULL if the model cannot be found.
        //! Processing stops when an optional model cannot be found.
        virtual void OnModelId (DgnModelId mid, DgnModelP cacheFound, DgnProjectP homeFile) = 0;
        };

    //! Process the items in a PersistentElementPath. Use this method if you want to learn more about a path that
    //! fails to evaluate.
    DGNPLATFORM_EXPORT
    StatusInt ProcessPath (PathProcessor&, DgnModelP dependentModel);

//__PUBLISH_SECTION_END__
    //! Look up the first element identified by this path. The first element in the path is assumed to be in \a dependentModel.
    //! @remarks This method returns only the first element in this path. Normally, this only makes sense if the path
    //! contains a single element. Normally, applications should call #EvaluateElement
    //! @see EvaluateElement
    //! @param[in]  dependentModel   The model that holds the first element identified by this path.
    //! @return The element in \a dependentModel that is identified by the first ElementId stored in this path, or <code>NULL</code> if the element is not found
    DGNPLATFORM_EXPORT
    ElementRefP EvaluateFirstElementRef (DgnModelP dependentModel) const;

    //! Get the root file and model of the path.
    //! @return non-zero error status if the path could not be evaluated.
    DGNPLATFORM_EXPORT
    StatusInt       EvaluateRootModel
    (
    DgnProjectP&       file,       //!< The root file
    DgnModelId&   mid,        //!< The root model
    DgnModelP       dependentModel   //!< The model containing the dependent element
    )   const;

    //! Get the first ElementId stored in this path.
    DGNPLATFORM_EXPORT
    ElementId GetFirstElementId () const;

//__PUBLISH_SECTION_START__

//@}

///@name Queries
///@{
    //! Query if the PersistentElementPath does not contain a path.
    DGNPLATFORM_EXPORT
    bool IsEmpty () const;

    //! Test if \a root is the root element of this path
    //! @param[in]  root      The element to find
    //! @param[in]  dependentModel   The model that contains the dependent element.
    //! @returns true if \a root is the root element of this path. False if the root is a different element or if the path could not be evaluated.
    //! @remarks
    //!     This function will return true even if the root element is deleted, as long as you pass in the correct root element ref.
    //!     This is useful when handling an IDependencyHandler::OnRootsChanged callback and you find that a root has been deleted and need to determine
    //!     which of many PersistentElementPaths within the dependent XAttribute contains the reference to the deleted root.
    DGNPLATFORM_EXPORT
    bool EqualElementRef (ElementRefP root, DgnModelP dependentModel) const;

    //! Does the path depend on this ElementRefP? If dependent and root elements are in different models
    //! and/or root is a shared cell instance, then this function will detect that the path points to
    //! the shared cell definition element, etc.
    //! @param[in]  root      The element to look for.
    //! @param[in]  dependentModel   The model that contains the dependent element.
    //! @return true if \a root appears anywhere in the path, or false if it does not appear or if the path could not be evaluated.
    DGNPLATFORM_EXPORT
    bool DependsOnElementRef (ElementRefP root, DgnModelP dependentModel) const;

    //! Query if two PersistentElementPathes match.
    DGNPLATFORM_EXPORT
    bool    IsExactMatch (PersistentElementPath const& rhs)  const;
///@}

    DGNPLATFORM_EXPORT
    void DisclosePointers (T_StdElementRefSet* refs, DgnModelP dependentModel);


//__PUBLISH_SECTION_END__
    DGNPLATFORM_EXPORT Utf8String Dump () const;
//__PUBLISH_SECTION_START__


}; // PersistentElementPath

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
