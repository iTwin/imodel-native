/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XAttributeHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "../DgnPlatform.h"
#include "XAttributeIter.h"
#include "ElementHandle.h"
#include "ITxnManager.h"
#include "XAttributeChange.h"
#include "PersistentElementPath.h"
#include <set>
#include <map>
#include <Bentley/bvector.h>
#include <functional>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__
//=======================================================================================
//! Interface to the helper object passed to XAttributeHandler::OnPreprocessCopy
//! @bsiclass
//=======================================================================================
struct     IReplaceXAttribute
{
virtual ~IReplaceXAttribute () {}

//! Call this to replace the specified XAttribute in a way that is compatible
//! with XAttributeHandler::OnPreprocessCopy
//! @param[in]      xahandle    Identifies the XAttribute on the source element.
//! @param[in]      size        # bytes of data to add.
//! @param[in]      data        Data to add.
//! @remarks \a xahandle is used only to get the XAttributeHandlerID and the XAttribute's secondary ID.
virtual void Add (XAttributeHandleCR xahandle, size_t size, void const* data) = 0;

//! Call this to replace the specified XAttribute in a way that is compatible
//! with XAttributeHandler::OnPreprocessCopy
//! @param[in]      xid         The XAttribute handler ID.
//! @param[in]      id          The XAttribute's secondary ID.
//! @param[in]      size        # bytes of data to add.
//! @param[in]      data        Data to add.
virtual void Add (XAttributeHandlerId xid, UInt32 id, size_t size, void const* data) = 0;
};


//=======================================================================================
//! Interface adopted by a class to interpret the data in an XAttribute with a given XAttributeHandlerId.
//! @remarks
//! An XAttributeHandler implementation should always be a singleton.
//! Handlers are registered by calling XAttributeHandlerManager::RegisterHandler
//! @see XAttributeHandlerManager
//! @bsiclass
//=======================================================================================
struct XAttributeHandler
{
    friend struct ITxn;

protected:
    DgnDomainP          m_domain;
    XAttributeHandlerId m_handlerId;

    XAttributeHandler() {m_domain = 0;}

    // default implementation: {return (size1==size2) && 0==memcmp(data1,data2,size1);}
    DGNPLATFORM_EXPORT virtual bool   _AreEqual (void const* data1, int size1, void const* data2, int  size2, UInt32 comparisonFlags,
                                        double distanceTolerance, double  directionTolerance);

public:
    DGNPLATFORM_EXPORT XAttributeHandlerId GetHandlerId() const;
    DGNPLATFORM_EXPORT DgnDomainP GetDgnDomain() const;
    void SetDgnDomain(DgnDomainR, XAttributeHandlerId);

    //! Called before my XAttribute has been newly added to an existing persistent element and has therefore become persistent.
    //! @param[in]      elRef       The element which will host the new XAttribute.
    //! @param[in]      handlerID   The XAttribute handler's ID.
    //! @param[in]      xAttrID     The XAttribute's ID.
    //! @param[in]      type        Transaction type.
    //! @bsimethod
    virtual void _OnPreAdd (ElementRefP elRef, XAttributeHandlerId handlerID, UInt32 xAttrID) {}

    //! @param[in]      xAttr       The XAttribute to be removed.
    //! @param[in]      type        Transaction type.
    //! @bsimethod
    virtual void _OnPreDelete (XAttributeHandleCR xAttr) {}

    //! Called before (a subset of) my XAttribute data on a persistent element is to be modified.
    //! @param[in]      xAttr       The XAttribute to be modified.
    //! @param[in]      newData     The new data that will be overwritten in the XAttribute.
    //! @param[in]      start       The offset of the first byte of newData in the XAttribute data.
    //! @param[in]      length      The number of bytes in newData to overwrite existing data.
    //! @param[in]      type        Transaction type.
    //! @remarks note that the modification has not yet happened, so this is your (only) opportunity to compare the pre-changed state of the
    //! XAttribute data with the about-to-become new state.
    //! @bsimethod
    virtual void _OnPreModifyData (XAttributeHandleCR xAttr, void const* newData, UInt32 start, UInt32 length) {}

    //! Called before my XAttribute data on a persistent element is to be replaced with new data.
    //! @param[in]      xAttr       The XAttribute to be replaced.
    //! @param[in]      newData     The new data that will be put into the XAttribute.
    //! @param[in]      newSize     The number of bytes in newData (will become the new size of my XAttribute).
    //! @param[in]      type        Transaction type.
    //! @remarks note that the modification has not yet happened, so this is your (only) opportunity to compare the pre-changed state of the
    //! XAttribute data with the about-to-become new state.
    //! @bsimethod
    virtual void _OnPreReplaceData (XAttributeHandleCR xAttr, void const* newData, UInt32 newSize) {}

    virtual void _OnTxnBoundary_Add (PersistentElementRefR el, UInt32 xAttrId) {}
    virtual void _OnTxnBoundary_Delete (DgnProjectR project, ElementId el, UInt32 xAttrId) {}
    virtual void _OnTxnBoundary_Modify (PersistentElementRefR, UInt32 xAttrId) {}

    virtual void _OnTxnReverse_Add (PersistentElementRefR, UInt32 xAttrId, bool isUndo) {}
    virtual void _OnTxnReverse_Delete (DgnProjectR, ElementId, UInt32 xAttrId, bool isUndo) {}
    virtual void _OnTxnReverse_Modify (PersistentElementRefR, UInt32 xAttrId, bool isUndo) {}

    virtual void _OnTxnReversed_Add (DgnProjectR, ElementId, UInt32 xAttrId, bool isUndo) {}
    virtual void _OnTxnReversed_Delete (PersistentElementRefR, UInt32 xAttrId, bool isUndo) {}
    virtual void _OnTxnReversed_Modify (PersistentElementRefR, UInt32 xAttrId, bool isUndo) {}

//! The handler is asked to compare the data from two versions of the same XAttribute
//!   and return true if they should be considered to be equivalent.
//! @remarks This method is called by mdlElmdscr_areIdentical
//! @remarks TBD: This method should be called by change-merging logic.
//! @remarks The default implementation is a bit-wise compare
//! @param[in]      data1           Data of XAttribute.
//! @param[in]      size1           Number of bytes of data of XAttribute.
//! @param[in]      data2           Data of XAttribute.
//! @param[in]      size2           Number of bytes of data of XAttribute.
//! @param[in]      comparisonFlags Flags controlling parameter to ignore during the
//!                                     comparison.  See COMPAREOPT_ definitions in mselmdsc.fdf.
//! @param[in]      distanceTolerance   Tolerance in UORs for comparing distance values.
//! @param[in]      directionTolerance  Tolerance in radians for comparing angular
//!                                     values.  A value of 1.0E-8 is appropriate for this comparison.
//! @return true if both sets are empty or if both contain the same changes to the same XAttributes
//! @bsimethod
DGNPLATFORM_EXPORT bool AreEqual (void const* data1, int size1, void const* data2, int size2, UInt32 comparisonFlags,
                                double distanceTolerance, double directionTolerance);

//! Compare the scheduled XAttributeChanges on the the two descriptors. Note: This does NOT recurse over components or process siblings
DGNPLATFORM_EXPORT static bool AreXAttributeChangeSetsEqual (MSElementDescrCP ed1, MSElementDescrCP ed2, UInt32 comparisonFlags,
                                                                double distanceTolerance, double directionTolerance);

//! Compare the scheduled XAttributeChangeSets
DGNPLATFORM_EXPORT static bool AreXAttributeChangeSetsEqual (XAttributeChangeSetCP xAttrCS1, XAttributeChangeSetCP xAttrCS2, UInt32 comparisonFlags,
                                                                double distanceTolerance, double directionTolerance);

//! Compare all XAttributes on the the two elements. Note: This does NOT recurse over components
DGNPLATFORM_EXPORT static bool AreXAttributesEqual (ElementHandleCR eh1, ElementHandleCR eh2, UInt32 comparisonFlags, double distanceTolerance, double directionTolerance);

DGNPLATFORM_EXPORT static StatusInt WriteXAttributeChangeSet (ElementRefP ref, XAttributeChangeSet* attrs, bool isAdd);

DGNPLATFORM_EXPORT static StatusInt CopyPersistentXAttributes (ElementRefP dst, ElementRefP src);
DGNPLATFORM_EXPORT static StatusInt CopyPersistentXAttributes (ElementHandleCR dst, ElementHandleCR src);
};

//__PUBLISH_SECTION_END__

#if defined ELEMENT_LOADING_REWORK
//=======================================================================================
//! The XAttribute handler registry.
//! @see XAttributeHandler
//! @bsiclass
//=======================================================================================
struct          XAttributeHandlerManager
{
//! Look up the XAttribute handler for the specified permanent handler ID.
//! @return the handler, if registered, or NULL if none.
//! @param[in]      xid         The permanent handler ID to look up.
//! @bsimethod
DGNPLATFORM_EXPORT static XAttributeHandlerP FindHandler (XAttributeHandlerId xid);
};
#endif

//__PUBLISH_SECTION_START__

//@}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
