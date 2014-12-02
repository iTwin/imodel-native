/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/XAttributeChangeSet.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <Bentley/RefCounted.h>
#include <DgnPlatform/DgnCore/XAttributeChange.h>
#include <Bentley/bset.h>

DGNPLATFORM_TYPEDEFS (XAttributeChangeSet)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A set of scheduled XAttribute changes
//! @bsiclass                                                     Sam.Wilson      11/2004
//=======================================================================================
struct  XAttributeChangeSet
{
//__PUBLISH_SECTION_END__

public:
    //! Alias for the XAttributeChangeSet's std::set type
    typedef bset<XAttributeChange>  T_Set;
    //! Alias for the XAttributeChangeSet's std::set::iterator type
    typedef T_Set::iterator         T_Iterator;
    //! Alias for the XAttributeChangeSet's std::set::const_iterator type
    typedef T_Set::const_iterator   T_ConstIterator;

    /*-----------------------------------------------------------------------------------
        static Member variables
    -----------------------------------------------------------------------------------*/
private:
    bool            m_purgeBefore;
    T_Set           m_changed;

    #if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
        static UInt32 s_count;
        static UInt32 s_ndcount;
    #endif

    XAttributeChangeSet (XAttributeChangeSetCR);   // Nope.

    //!  Add a change to the schedule for this attribute.
    //!  @remarks This will store a deep copy of the input XAttributeChange data.
    //! @return non-zero error status if the schedule already contains a change to this xattribute
    StatusInt   Schedule (XAttributeChangeCR, XAttributeChange::ChangeType);

    /*-----------------------------------------------------------------------------------
        Non-Published public Member functions
    -----------------------------------------------------------------------------------*/
public:
    //  ---------- Lifecycle ----------------

    //! Initialize an attribute set.
    DGNPLATFORM_EXPORT XAttributeChangeSet ();

    //! Free the attribute set, releasing its memory to the heapzone.
    DGNPLATFORM_EXPORT ~XAttributeChangeSet ();

    //! Copy a set. This is a convenience method. It simply iterates the input set and schedules each item in this set.
    //! The XAttribute data is deep-copied.
    DGNPLATFORM_EXPORT void CopySchedule (XAttributeChangeSetCR);

    //! Clear the set
    DGNPLATFORM_EXPORT void Clear ();

    void SetPurge () {m_purgeBefore = true;}
    bool IsPurge () const {return m_purgeBefore;}

    /// @name Scheduling
    /// @{

    //!  Add a change to the schedule for this attribute.
    //!  @remarks This will store a copy of the XA data
    //! @return non-zero error status if the schedule already contains a change to this xattribute
    DGNPLATFORM_EXPORT StatusInt Schedule (XAttributeHandlerId const&, UInt32 xAttrId, size_t sz, void const* d, XAttributeChange::ChangeType);

    //!  Add a change to the schedule for this attribute. If replaceExistin is true, replaces any existing scheduled change with matching handlerId and xAttrId.
    //!  @remarks This will store a copy of the XA data
    //! @return non-zero error status if the schedule already contains a change to this xattribute
    DGNPLATFORM_EXPORT StatusInt Schedule (XAttributeHandlerId const&, UInt32 xAttrId, size_t sz, void const* d, XAttributeChange::ChangeType, bool replaceExisting);


    //!  Cancel the change scheduled for this attribute.
    //! @return non-zero error status if the schedule does not contain a change to this xattribute
    DGNPLATFORM_EXPORT StatusInt Cancel (T_Iterator);

    //! Read all of the persistent XAttributes and schedule them as Writes.
    //! \em NB: This assumes that ed->GetElementRef() identifies the persistent element
    DGNPLATFORM_EXPORT static StatusInt LoadXasAsWrites (MSElementDescrP ed);

    //! Read all of the persistent XAttributes from 'source' element and schedule
    //! then as writes on 'sink' element.
    DGNPLATFORM_EXPORT static StatusInt CopyPersistentXas (MSElementDescrP sink, MSElementDescrCP source, XAttributeChange::ChangeType xct = XAttributeChange::CHANGETYPE_Write);

    /// @}
    /// @name Queries
    /// @{

    //!  Get an iterator that points to the first changed attribute in the set
    DGNPLATFORM_EXPORT T_Iterator Begin ();

    //!  Get the special iterator that represents a position "beyond the end" of the set
    DGNPLATFORM_EXPORT T_Iterator End ();

    //!  Get an iterator that points to the first changed attribute in the set
    DGNPLATFORM_EXPORT T_ConstIterator Begin () const;

    //!  Get the special iterator that represents a position "beyond the end" of the set
    DGNPLATFORM_EXPORT T_ConstIterator End () const;

    //!  Look up the specified attribute in the change set. Returns End() if not found.
    DGNPLATFORM_EXPORT T_Iterator Find (XAttributeChangeCR);

    //!  Look up the specified attribute in the change set. Returns End() if not found.
    DGNPLATFORM_EXPORT T_Iterator Find (XAttributeHandlerId const&, UInt32 xAttrId);

    //! Signature of XAttributeChange compare callback
    typedef bool T_IsEqual (XAttributeChangeCR, XAttributeChangeCR, UInt32 comparisonFlags, double distanceTol, double directionTol);

    //!  Test if this set is equal to \a s2.
    //!  This function will return false if:
    //! \li either set has a different number of XAttributeChanges,
    //! \li a change to a given XAttribute has been scheduled to one set and not the other
    //! \li the two sets contain have different types of changes for a given XAttribute
    //! \li each set has a Write change for a given XAttribute, but the scheduled data differs.
    //! @param[in]      s2                    The other set.
    //! @param[in]      f                     Optional: the compare function.
    //! @param[in]      comparisonFlags       Flags controlling parameter to ignore during the
    //!  comparison.  See COMPAREOPT_ definitions in mselmdsc.fdf.
    //! @param[in]      distanceTolerance     Tolerance in UORs for comparing distance values.
    //! @param[in]      directionTolerance    Tolerance in radians for comparing angular.
    //! values.  A value of 1.0E-8 is appropriate for this comparison.
    //! @return true if both sets are empty or if both contain the same changes to the same XAttributes
    //! @remarks The XAttributeHandler::IsEqual method is called
    DGNPLATFORM_EXPORT bool IsEqual (XAttributeChangeSetCR s2, T_IsEqual* f, UInt32 comparisonFlags, double distanceTolerance, double directionTolerance) const;

    //!  Query how many changes are scheduled.
    DGNPLATFORM_EXPORT UInt32 Size () const;

    /// @}

    /*-----------------------------------------------------------------------------------
        Published Member functions
    -----------------------------------------------------------------------------------*/
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (XATTRIBUTE_CHANGE_TRACK_LEAKS)
DGNPLATFORM_EXPORT void xAttributeChangeSet_checkLeaks();
#endif

/** @endcond */
