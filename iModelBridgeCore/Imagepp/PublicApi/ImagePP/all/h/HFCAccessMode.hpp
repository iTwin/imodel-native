//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCAccessMode.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/**--------------------------------------------------------------------------
 Returns true if the access mode(s) described by @r{pi_AccessMode} is
 included in the access mode(s) decribed by the object from which the
 method is called.  For example, if an access mode is READ_WRITE_CREATE,
 other access modes that are READ_ONLY or READ_WRITE are considered as
 included in the first one.

 @param Access mode to check if it is included in this one.

 @return Boolean value : true is specified access mode is included in
         this one, false otherwise.
---------------------------------------------------------------------------*/
inline bool HFCAccessMode::IsIncluded(HFCAccessMode pi_AccessMode) const
    {
    // Test if the specified access is included
    bool IsIncluded = true;

    if ((pi_AccessMode.m_HasReadAccess) && (!m_HasReadAccess))
        IsIncluded = false;
    else if ((pi_AccessMode.m_HasWriteAccess) && (!m_HasWriteAccess))
        IsIncluded = false;
    else if ((pi_AccessMode.m_HasCreateAccess) && (!m_HasCreateAccess))
        IsIncluded = false;
    else if ((pi_AccessMode.m_OpenAlways) && (!m_OpenAlways))
        IsIncluded = false;

    return IsIncluded;
    }

/**--------------------------------------------------------------------------
 Equality evaluation operator.  Two access modes are considered "equal"
 if they describe the same access capabilities.

 @param pi_rObj Object to compare to this one.

 @return Boolean value that is true if the access modes are considered
         equal, or the opposite value if they are not.
---------------------------------------------------------------------------*/
inline bool HFCAccessMode::operator==(const HFCAccessMode& pi_rObj) const
    {
    return (m_HasReadAccess == pi_rObj.m_HasReadAccess) &&
           (m_HasWriteAccess == pi_rObj.m_HasWriteAccess) &&
           (m_HasCreateAccess == pi_rObj.m_HasCreateAccess) &&
           (m_HasReadShare == pi_rObj.m_HasReadShare) &&
           (m_HasWriteShare == pi_rObj.m_HasWriteShare) &&
           (m_OpenAlways == pi_rObj.m_OpenAlways);
    }

/**--------------------------------------------------------------------------
 Non-equality evaluation operator.  Two access modes are considered "equal"
 if they describe the same access capabilities.

 @param pi_rObj Object to compare to this one.

 @return Boolean value that is false if the access modes are considered
         equal, or the opposite value if they are not.
---------------------------------------------------------------------------*/
inline bool HFCAccessMode::operator!=(const HFCAccessMode& pi_rObj) const
    {
    return !operator==(pi_rObj);
    }

/**--------------------------------------------------------------------------
 Bitwise OR operator that is overloaded to combine two access modes.
 Every access right that is set in each object remains present in the
 result.  The current object is not modified.

 @param pi_rObj Constant reference to another access mode to combine to
                the current one.

 @return A brand new access mode object that contains the combination of
         the left and right operands of this operator.
---------------------------------------------------------------------------*/
inline HFCAccessMode HFCAccessMode::operator|(const HFCAccessMode& pi_rObj) const
    {
    HFCAccessMode NewMode(*this);

    NewMode.m_HasReadAccess   |= pi_rObj.m_HasReadAccess;
    NewMode.m_HasWriteAccess  |= pi_rObj.m_HasWriteAccess;
    NewMode.m_HasCreateAccess |= pi_rObj.m_HasCreateAccess;
    NewMode.m_HasReadShare    |= pi_rObj.m_HasReadShare;
    NewMode.m_HasWriteShare   |= pi_rObj.m_HasWriteShare;
    NewMode.m_OpenAlways      |= pi_rObj.m_OpenAlways;

    return NewMode;
    }

/**--------------------------------------------------------------------------
 Bitwise OR-assignment operator that is overloaded to add new modes to
 the current object.   Every access right defined in the right operand is
 added (if not already present) to the current (left-operand) access mode
 that becomes modified.

 @param pi_rObj Constant reference to another access mode to add to the
                current one.

 @return A reference to self that can be used as an l-value. The object
         is modified and contain access rights it had before combined with those
         in the right operand.

---------------------------------------------------------------------------*/
inline HFCAccessMode& HFCAccessMode::operator|=(const HFCAccessMode& pi_rObj)
    {
    m_HasReadAccess   |= pi_rObj.m_HasReadAccess;
    m_HasWriteAccess  |= pi_rObj.m_HasWriteAccess;
    m_HasCreateAccess |= pi_rObj.m_HasCreateAccess;
    m_HasReadShare    |= pi_rObj.m_HasReadShare;
    m_HasWriteShare   |= pi_rObj.m_HasWriteShare;
    m_OpenAlways      |= pi_rObj.m_OpenAlways;

    return *this;
    }

END_IMAGEPP_NAMESPACE