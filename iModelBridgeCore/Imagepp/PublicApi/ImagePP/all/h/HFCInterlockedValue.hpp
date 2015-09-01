//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCInterlockedValue.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCInterlockedValue
//-----------------------------------------------------------------------------

#include "HFCMonitor.h"

BEGIN_IMAGEPP_NAMESPACE

/**----------------------------------------------------------------------------
 Default constructor for this class.  The value is set to zero by this
 constructor.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::HFCInterlockedValue()
    {
    m_Value = (T)0;
    }


/**----------------------------------------------------------------------------
 Constructor that takes an assignment value.  Beware that the conversion
 constructor is an explicit one, so it is not possible to get an
 automatic transformation of a variable caused by implicit typecast
 during a call to a function or method.

 @param pi_rValue Reference to initial value to assign to the internal variable.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::HFCInterlockedValue(const T& pi_rValue)
    {
    m_Value = pi_rValue;
    }


/**----------------------------------------------------------------------------
 Copy constructor for this class.  The value is copied from another interlocked
 value.

 @param pi_rObj Reference to another multi-thread variable (of same type) to
                duplicate.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::HFCInterlockedValue(const HFCInterlockedValue<T>& pi_rObj)
    {
    m_Value = (T)pi_rObj;
    }


/**----------------------------------------------------------------------------
 Destructor for this class.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::~HFCInterlockedValue()
    {
    }


/**----------------------------------------------------------------------------
 Assignment operator that duplicates another interlocked value.

 @param pi_rObj Constant reference to variable to duplicate.

 @return A reference to self, to be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator= (const HFCInterlockedValue<T>& pi_rObj)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value = (T)pi_rObj;
    return (*this);
    }


/**----------------------------------------------------------------------------
 Assignment operator that reset the value being managed.

 @param Constant reference to value to assign to internal variable.

 @return A reference to self, to be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator= (const T& pi_rValue)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value = pi_rValue;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator==(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value == (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator==(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value == pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator!=(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value != (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator!=(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value != pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator<(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value < (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator<(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value < pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator<=(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value <= (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator<=(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value <= pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator>(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value > (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator>(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value > pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator>=(const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value >= (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one the comparison operators that have been overloaded for this
 class.  Their purpose is to mimic to behavior of a native "unprotected"
 variable, making possible the use of this object into comparison as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @param pi_rValue Constant reference to the value to compare to.

 @return A Boolean value that indicates the result of the comparison.
-----------------------------------------------------------------------------*/
template<class T> inline
bool HFCInterlockedValue<T>::operator>=(const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value >= pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator+ (const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value + (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator+ (const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value + pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator+=(const HFCInterlockedValue<T>& pi_rObj)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value += (T)pi_rObj;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator+=(const T& pi_rValue)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value += pi_rValue;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator- (const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value - (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator- (const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value - pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator-=(const HFCInterlockedValue<T>& pi_rObj)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value -= (T)pi_rObj;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator-=(const T& pi_rValue)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value -= pi_rValue;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator* (const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value * (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator* (const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value * pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator*=(const HFCInterlockedValue<T>& pi_rObj)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value *= (T)pi_rObj;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator*=(const T& pi_rValue)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value *= pi_rValue;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator/ (const HFCInterlockedValue<T>& pi_rObj) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value / (T)pi_rObj);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return The resulting numeric value.
-----------------------------------------------------------------------------*/
template<class T> inline T
HFCInterlockedValue<T>::operator/ (const T& pi_rValue) const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value / pi_rValue);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rObj Constant reference to another protected variable (of same type)
                that holds the value to be used as right operand in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator/=(const HFCInterlockedValue<T>& pi_rObj)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value /= (T)pi_rObj;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the arithmetic operators that have been overloaded for
 this class.  Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the arithmetic use of this
 object, as it was its internal variable, while adding implicit
 protection for multi-thread environments.

 @param pi_rValue Constant reference to the value that is the right operand
                  in the operation.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>&
HFCInterlockedValue<T>::operator/=(const T& pi_rValue)
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value /= pi_rValue;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the increment/decrement operators that have been overloaded
 for this class. Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the use of this object, as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>
HFCInterlockedValue<T>::operator++()
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value++;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the increment/decrement operators that have been overloaded
 for this class. Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the use of this object, as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>
HFCInterlockedValue<T>::operator++(int32_t)
    {
    // Keep a copy of the current value
    HFCInterlockedValue Result(*this);

    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value++;
    return (Result);
    }


/**----------------------------------------------------------------------------
 This is one of the increment/decrement operators that have been overloaded
 for this class. Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the use of this object, as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>
HFCInterlockedValue<T>::operator--()
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value--;
    return (*this);
    }


/**----------------------------------------------------------------------------
 This is one of the increment/decrement operators that have been overloaded
 for this class. Their purpose is to mimic to behavior of a native
 "unprotected" variable, making possible the use of this object, as it
 was its internal variable, while adding implicit protection for
 multi-thread environments.

 @return A reference to self that can be used as an l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCInterlockedValue<T>
HFCInterlockedValue<T>::operator--(int32_t)
    {
    // Keep a copy of the current value
    HFCInterlockedValue Result(*this);

    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    m_Value--;
    return (Result);
    }


/**----------------------------------------------------------------------------
 Type cast operator that returns the value held in the internal variable
 protected by this object.  This operator make possible the use of the
 object in place where a straight value or an unprotected variable is
 required.

 @return The value held by the internal variable.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::operator T()
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value);
    }


/**----------------------------------------------------------------------------
 Type cast operator that returns the value held in the internal variable
 protected by this object.  This operator make possible the use of the
 object in place where a straight value or an unprotected variable is
 required.

 @return The value held by the internal variable.
-----------------------------------------------------------------------------*/
template<class T> inline
HFCInterlockedValue<T>::operator T() const
    {
    HFCMonitor Monitor(const_cast<HFCInterlockedValue*>(this));

    return (m_Value);
    }

END_IMAGEPP_NAMESPACE