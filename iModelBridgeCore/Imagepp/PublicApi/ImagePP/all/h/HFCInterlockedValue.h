//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCInterlockedValue.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCInterlockedValue
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE
/**

    This is a template class that can be used to get automatic multithread
    safety on numeric data elements that have to be shared between threads.
    For example, if a given variable is maintained by a thread but queried
    by another one, its usage without safety may cause problems. If a thread
    modifies it while the other is reading it, it is possible that the read
    operation occurs in the middle of the modification operation (even at
    processor level, because an increment operation is not atomic), leaving
    possibility of unpredictable results.  By using this class, it is
    possible to obtain almost transparent safety for that kind of situation.

    This template class creates any kind of numeric variable, making it a
    multi-thread-able variable, by wrapping it into an exclusive key that is
    re-shaped to mimic the original behavior of the data type of the
    variable. It is still possible to perform "native" operations on the
    variable, like assignment, arithmetic, comparisons, value reading.  But
    each time one of these operations is performed, the variable is locked
    in order to prevent other threads from modifying it, hoping that other
    threads also use the data through this class.

    @see HFCExclusiveKey

*/

template <class T> class HFCInterlockedValue : public HFCExclusiveKey
    {
public:

    // Constructors and destructors

    HFCInterlockedValue();
    explicit                HFCInterlockedValue(const T& pi_rValue);
    HFCInterlockedValue(const HFCInterlockedValue<T>& pi_rObj);
    virtual                 ~HFCInterlockedValue();


    // assignment operators

    HFCInterlockedValue<T>&    operator= (const HFCInterlockedValue<T>& pi_rObj);
    HFCInterlockedValue<T>&    operator= (const T& pi_rValue);


    // Comparision operator

    bool                    operator==(const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator==(const T& pi_rValue) const;
    bool                    operator!=(const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator!=(const T& pi_rValue) const;
    bool                    operator< (const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator< (const T& pi_rValue) const;
    bool                    operator<=(const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator<=(const T& pi_rValue) const;
    bool                    operator> (const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator> (const T& pi_rValue) const;
    bool                    operator>=(const HFCInterlockedValue<T>& pi_rObj) const;
    bool                    operator>=(const T& pi_rValue) const;

    // Arithmetic Operations

    T                       operator+ (const HFCInterlockedValue<T>& pi_rObj) const;
    T                       operator+ (const T& pi_rValue) const;
    HFCInterlockedValue<T>& operator+=(const HFCInterlockedValue<T>& pi_rObj);
    HFCInterlockedValue<T>& operator+=(const T& pi_rValue);
    T                       operator- (const HFCInterlockedValue<T>& pi_rObj) const;
    T                       operator- (const T& pi_rValue) const;
    HFCInterlockedValue<T>& operator-=(const HFCInterlockedValue<T>& pi_rObj);
    HFCInterlockedValue<T>& operator-=(const T& pi_rValue);
    T                       operator* (const HFCInterlockedValue<T>& pi_rObj) const;
    T                       operator* (const T& pi_rValue) const;
    HFCInterlockedValue<T>& operator*=(const HFCInterlockedValue<T>& pi_rObj);
    HFCInterlockedValue<T>& operator*=(const T& pi_rValue);
    T                       operator/ (const HFCInterlockedValue<T>& pi_rObj) const;
    T                       operator/ (const T& pi_rValue) const;
    HFCInterlockedValue<T>& operator/=(const HFCInterlockedValue<T>& pi_rObj);
    HFCInterlockedValue<T>& operator/=(const T& pi_rValue);

    // Incrementors/decrementors

    HFCInterlockedValue     operator++();         // preincrement
    HFCInterlockedValue     operator++(int32_t);    // postincrement
    HFCInterlockedValue     operator--();         // predecrement
    HFCInterlockedValue     operator--(int32_t);    // postdecrement

    // Value operators
    operator T();
    operator T() const;

private:

    // The value
    T                       m_Value;
    };

END_IMAGEPP_NAMESPACE

#include "HFCInterlockedValue.hpp"

