/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/PersistentSnapPath.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <string>
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/PersistentElementPath.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct PersistentSnapPathData;
struct PersistentSnapPathDataModifier;
struct PersistentSnapPathBlobIO;

/*=================================================================================**//**
PersistentSnapPath is a persistent reference to a point on an element or at the intersection of two elements or
otherwise based on the custom topology of an element.
A PersistentSnapPath can be
embedded in the data of an XAttribute. PersistentSnapPath implements methods to help the
XAttribute to support copy (deep-copying and reference remapping) and dependencies.
<p>
PersistentSnapPath is part of a framework to support XAttributes that contain references
to other elements. PersistentSnapPath is just a class -- it's not
an XAttribute. You embed instances of PersistentSnapPath in your own XAttributes.

<h4>Creation</h4>

You create a PersistentSnapPath initially from a SnapPath using the FromSnapPath method.
You can get a SnapPath from mdlLocate_getCurrPath.

<h4>Evaluation</h4>
<p>
Use methods EvaluatePoint, EvaluateTarget1, and EvaluateTarget2 to get the target of the snap.
<p>
<h4>Persistence</h4>
<p>
When you save your XAttribute, call PersistentSnapPath::Store on each PersistentSnapPath that is
embedded in it. That will cause each PersistentSnapPath
to stream its state out to an DataExternalizer -- a byte stream. Whenever you need to
access a PersistentSnapPath in a persistent XAttribute, you must call PersistentSnapPath::Load,
to allow it to restore its state from the XAttribute's raw data.
<p>
This class uses a compressed format for storage. For example, PersistentSnapPath uses
just 19 bytes (uncompressed) to capture a linear or arc association
in a small file. An AssocPoint, by contrast, is 40 bytes in size.

<h4>Copying</h4>

If your XAttribute embeds PersistentSnapPaths, then you must implement and register an XAttributeHandler
\em and an IXAttributePointerContainerHandler for your XAttribute. MicroStation calls methods on these interfaces
in order to manage copying of PersistentSnapPath.
<p>
When your XAttribute is copied, XAttributeHandler::OnPreprocessCopy is called. This method must call
PersistentSnapPath::OnPreprocessCopy on each embedded PersistentSnapPath.
<p>
When MicroStation finishes up the copy, it will call your IXAttributePointerContainerHandler::OnPreprocessCopyRemapIds
method. This method must call PersistentSnapPath::OnPreprocessCopyRemapIds on each embedded PersistentSnapPath.

<h4>Dependencies</h4>

In order to support Dependency back-pointers, your XAttribute handler must implement IXAttributePointerContainerHandler.
The XAttributePointerContainerHandler::_DisclosePointers will be called when your XAttribute is loaded and when it is changed.
<p>
Only element handlers can get dependency change-propagation callbacks; XAttribute handlers do not get these
callbacks. Therefore, if an XAttribute contains an element reference and you want to get dependency callbacks
when the target element changes, you must attach an element handler to the element that owns the XAttribute.
Your element handler must implement IDependencyHandler.
*
* @bsiclass                                                     Sam.Wilson      06/2005
+===============+===============+===============+===============+===============+======*/
struct PersistentSnapPath
{
    /*-----------------------------------------------------------------------------------
        static Member variables
    -----------------------------------------------------------------------------------*/
private:
    byte*       m_state;
    UInt16      m_stateLen;
    DgnModelP m_homeModel;
    friend struct      PersistentSnapPathData;
    friend struct      PersistentSnapPathDataModifier;

private:
    void        Init (DgnModelP);
    void        FreeMemory();
    void        Copy(PersistentSnapPath const&);
    StatusInt   FromAssocPointAndPaths (AssocGeom const&, DisplayPath const*, DisplayPath const*);
    StatusInt   FromBlob (PersistentSnapPathBlobIO const&);
    StatusInt   FromCustom (DisplayPath const*, byte const*, UInt32);
    StatusInt   FromCustom (DisplayPath const*, AssocGeom const&);
/*__PUBLISH_SECTION_END__*/
    void        GrabData (DataExternalizer const&);
/*__PUBLISH_SECTION_START__*/

    /*-----------------------------------------------------------------------------------
        Public Member functions
    -----------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_END__*/
public:
DGNPLATFORM_EXPORT
    PersistentSnapPath ();
/*__PUBLISH_SECTION_START__*/

public:
    //! Construct an empty PersistentSnapPath
explicit
    DGNPLATFORM_EXPORT PersistentSnapPath (DgnModelP home); //!< The model containing the element that will hold this reference

    //! Deallocate resources held by PersistentSnapPath
    DGNPLATFORM_EXPORT ~PersistentSnapPath ();

    //! Construct a PersistentSnapPath directly from a SnapPathCP
    //! @see #FromSnapPath #IsValid
    DGNPLATFORM_EXPORT explicit PersistentSnapPath (DgnModelP home, SnapPathCP);

    //! Construct a PersistentSnapPath directly from a PersistentElementPath
    //! @see #FromPersistentElementPath #IsValid
    DGNPLATFORM_EXPORT explicit PersistentSnapPath (DgnModelP home, PersistentElementPathCR);

    DGNPLATFORM_EXPORT PersistentSnapPath (PersistentSnapPath const&);

    //! Construct a PSP that is not a link to another element but just contains static (x,y,z) coordinate data.
    //! The units of the coordinates are note stored. They are assumed to be in the units of the holder's model.
    //! @see IsStaticDPoint3d
    DGNPLATFORM_EXPORT explicit PersistentSnapPath (DPoint3d const&);

    DGNPLATFORM_EXPORT PersistentSnapPath (DgnModelP home, AssocPoint const&);

    DGNPLATFORM_EXPORT PersistentSnapPath& operator= (PersistentSnapPath const&);

    //! Query if Constructor worked
    DGNPLATFORM_EXPORT bool IsValid() const;

    //! Forget state
    DGNPLATFORM_EXPORT void Clear ();

    //! Get the home model ref
    DgnModelP GetHomeDgnModelP () const {return m_homeModel;}

    //! Set the home model ref
    void SetHomeDgnModel (DgnModelP m) {m_homeModel = m;}

    //! Capture a SnapPath
    DGNPLATFORM_EXPORT StatusInt FromSnapPath (SnapPathCP);

    //! Capture an AssocPoint
    DGNPLATFORM_EXPORT StatusInt FromAssocPoint (AssocPoint const&);

    //! Capture a PersistentElementPath with no particular point association
    //! @remarks This method makes a copy of the supplied PersistentElementPath. It does not save a reference to the actual object.
    DGNPLATFORM_EXPORT StatusInt FromPersistentElementPath (PersistentElementPath const&);

    //! Capture a PersistentElementPath with no particular point association
    //! @remarks This method makes a copy of the supplied PersistentElementPath. It does not save a reference to the actual object.
    DGNPLATFORM_EXPORT StatusInt FromPersistentElementPath (DgnModelP, PersistentElementPath const&);

    //! Compute the point identified by the snap
    //! @remarks May call the DisplayHandler::EvaluateSnap method.
    //! pt[out] the point (transformed into the coordinate system of the home model aka "world cooordinates")
    DGNPLATFORM_EXPORT StatusInt EvaluatePoint (DPoint3d& pt) const;

    //! Compute the location identified by an AssocPoint
    //! @param[out]   pt        the computed snap location
    //! @param[in]    assoc     the snap definition to evaluate
    //! @param[in]    homeModel the model that contains the dependent element
    //! @remarks Calls the DisplayHandler::EvaluateSnap method.
    DGNPLATFORM_EXPORT static StatusInt EvaluateAssocPoint (DPoint3d& pt, AssocPoint const& assoc, DgnModelP homeModel);

    //! Query if snap has two  targets
    DGNPLATFORM_EXPORT bool HasTwoPaths () const;

    //! Query if all targets can be found
    DGNPLATFORM_EXPORT bool IsTargetAvailable () const;

    //! Get target element.
    //! @see GetPaths to retrieve both targets of an intersection snap
    //! @remarks
    //!     \em Note: See PersistentElementPath::EvaluateElement for the lifetime of the modelref contained in the returned handle.
    DGNPLATFORM_EXPORT ElementHandle EvaluateElement (bool wantFirst = true) const;

    //! Does the moniker depend on this ElementRefP? If host and target are in same model
    //! and if target is not a shared cell instance,
    //! then this is the same as EvaluateElementRef() == target.
    //! For non-simple paths, then this will recognize the moniker's dependence on
    //! parts of the shared cell instance path, etc.
    DGNPLATFORM_EXPORT bool DependsOnElementRef (ElementRefP target) const;

    //! Get the target element(s) identified by the snap
    //! Optionaly, get the assoc point data
    DGNPLATFORM_EXPORT StatusInt GetPaths (DisplayPathPtr&, DisplayPathPtr&, AssocPoint* assoc = NULL) const;

    //! Get the custom keypoint data associated with this snap
    //! @return ERROR if there is no custom data
    //! @param[out] data pointer to saved custom data
    //! @param[out] nbytes number of bytes of custom data
    //! @remarks The caller must \em not free the returned pointer.
    //! @remarks The caller must \em not modify the custom data using the returned pointer.
    DGNPLATFORM_EXPORT StatusInt GetCustomKeypointData (byte const*& data, UInt32& nbytes) const;

    //! Set or update the custom keyoint data associated with this snap
    //! @param[in] data custom data
    //! @param[in] nbytes number of bytes of custom data
    //! @remarks The function converts the snap to a "custom" snap. See SetPayloadData to
    //!             add additional data to a snap.
    DGNPLATFORM_EXPORT void SetCustomKeypointData (byte const* data, UInt32 nbytes);

    //! Query if this PSP contains only static (x,y,z) data. To get the point, call EvaluatePoint
    DGNPLATFORM_EXPORT bool IsStaticDPoint3d () const;

    //! Get the payload data
    //! @param[in] bytes payload data
    //! @param[in] nbytes number of bytes of payload data
    //! @remarks The caller must \em not free the returned pointer.
    //! @remarks The caller must \em not modify the custom data using the returned pointer.
    //! @remarks You can add payload data to any kind of snap, including custom data.
    DGNPLATFORM_EXPORT StatusInt GetPayloadData (void const*& bytes, UInt32& nbytes) const;

    //! Add, replace, or delete payload data
    DGNPLATFORM_EXPORT void SetPayloadData (void const* data, UInt32 nbytes);

    //! Store a note that captures root deep-copying preference.
    //! @remarks OnPreprocessCopy does \em not check this flag. Instead, the owner of the PSP must check it and decide what to do with it.
    DGNPLATFORM_EXPORT void SetDoDeepCopy (bool);

    DGNPLATFORM_EXPORT bool GetDoDeepCopy () const;

/*__PUBLISH_SECTION_END__*/
    //! Tell the PersistentSnapPath to recreate its state from store data
    DGNPLATFORM_EXPORT StatusInt Load (DataInternalizer&);
    //! Tell the PersistentSnapPath to store its state
    DGNPLATFORM_EXPORT void Store (DataExternalizer&) const;
/*__PUBLISH_SECTION_START__*/

    //! Phase I of copying a persistent display path: deep-copy target elements and/or replace stored IDs with remap keys
    //! @param[in] opt  How to handle the references contained in the display path
    //! @param[in] cc   The copy context
    //! @param[in] hostElement   The element that contains the XAttribute
    //! @returns non-zero error status if the display path cannot be copied and should be dropped
    //! @remarks
    //!    After this function returns, this persistent display path should be stored until OnPreprocessCopyRemapIds is called.
    //! @see OnPreprocessCopyRemapIds
    // DGNPLATFORM_EXPORT StatusInt OnPreprocessCopy (ElementHandle const& hostElement, CopyContextP cc, PersistentElementPath::CopyOption opt); removed in graphite

    //! Get description for debugging purposes
    DGNPLATFORM_EXPORT WString GetDescription () const;

}; // PersistentSnapPath

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
