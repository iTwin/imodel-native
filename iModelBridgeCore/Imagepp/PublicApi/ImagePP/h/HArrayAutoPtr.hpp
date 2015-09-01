//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HArrayAutoPtr.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for the class HArrayAutoPtr.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>::HArrayAutoPtr(P* pi_Ptr)
    : m_Ptr(pi_Ptr)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.
// IMPORTANT:  This contructor transfers the owner ship of the pointer to the
//             newly created object.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>::HArrayAutoPtr(HArrayAutoPtr<P>& pi_rObj)
    : m_Ptr(pi_rObj.release())
    {
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>::~HArrayAutoPtr()
    {
    delete[] m_Ptr;
    }

//-----------------------------------------------------------------------------
// Assignment operator.
// Delete the current pointed object and change it for the specified one. We
// release the pointer of the specified HArrayAutoPtr.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>& HArrayAutoPtr<P>::operator=(HArrayAutoPtr<P>& pi_rObj)
    {
    delete[] m_Ptr;
    reset(pi_rObj.release());

    return *this;
    }

//-----------------------------------------------------------------------------
// Assignment operator.
// Delete the current pointed object and point on the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>& HArrayAutoPtr<P>::operator=(P* pi_Ptr)
    {
    delete[] m_Ptr;
    m_Ptr = pi_Ptr;

    return *this;
    }

//-----------------------------------------------------------------------------
// Operator used to use the pointer as a reference.
//-----------------------------------------------------------------------------
template <class P>
inline P& HArrayAutoPtr<P>::operator*() const
    {
    return *m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator used to manipulate the pointed object.
//-----------------------------------------------------------------------------
template <class P>
inline P* HArrayAutoPtr<P>::operator->() const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HArrayAutoPtr<P>::operator==(const P* pi_pObj) const
    {
    return m_Ptr == pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HArrayAutoPtr<P>::operator==(const HArrayAutoPtr<P>& pi_rObj)
    {
    return m_Ptr == pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different from
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HArrayAutoPtr<P>::operator!=(const P* pi_pObj) const
    {
    return m_Ptr != pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different from
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HArrayAutoPtr<P>::operator!=(const HArrayAutoPtr<P>& pi_rObj)
    {
    return m_Ptr != pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Cast the auto ptr object to an object pointer.
//-----------------------------------------------------------------------------
template <class P>
inline HArrayAutoPtr<P>::operator P* () const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Method that returns the pointer.
//-----------------------------------------------------------------------------
template <class P>
inline P* HArrayAutoPtr<P>::get() const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Method that releases the pointer but does not delete it.
//-----------------------------------------------------------------------------
template <class P>
inline P* HArrayAutoPtr<P>::release()
    {
    return reset(0);
    }

//-----------------------------------------------------------------------------
// Method that changes the pointer for the specified one, and returns the old
// one.  There is no deletion.
//-----------------------------------------------------------------------------
template <class P>
inline P* HArrayAutoPtr<P>::reset(P* pi_Ptr)
    {
    P* tmp = m_Ptr;
    m_Ptr  = pi_Ptr;

    return tmp;
    }



//-----------------------------------------------------------------------------
// Methods for the class HMallocAutoPtr.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>::HMallocAutoPtr(size_t pi_NbItems)
    : m_Ptr(0)
    {
    if (pi_NbItems > 0)
        AllocItems(pi_NbItems);
    }

//-----------------------------------------------------------------------------
// Copy constructor.
// IMPORTANT:  This contructor transfers the owner ship of the pointer to the
//             newly created object.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>::HMallocAutoPtr(HMallocAutoPtr<P>& pi_rObj)
    : m_Ptr(pi_rObj.release())
    {
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>::~HMallocAutoPtr()
    {
    free(m_Ptr);
    }


//-----------------------------------------------------------------------------
// Alloc
//-----------------------------------------------------------------------------
template <class P>
inline P* HMallocAutoPtr<P>::AllocItems (size_t pi_NbItems)
    {
    m_Ptr = (P*)malloc(pi_NbItems*sizeof(P));
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// ReAlloc
//-----------------------------------------------------------------------------
template <class P>
inline P* HMallocAutoPtr<P>::ReallocItems (size_t pi_NewNbItems)
    {
    m_Ptr = (P*)realloc (m_Ptr, pi_NewNbItems*sizeof(P));
    return m_Ptr;
    }


//-----------------------------------------------------------------------------
// Assignment operator.
// Delete the current pointed object and change it for the specified one. We
// release the pointer of the specified HMallocAutoPtr.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>& HMallocAutoPtr<P>::operator=(HMallocAutoPtr<P>& pi_rObj)
    {
    free(m_Ptr);
    reset(pi_rObj.release());

    return *this;
    }


//-----------------------------------------------------------------------------
// Operator used to use the pointer as a reference.
//-----------------------------------------------------------------------------
template <class P>
inline P& HMallocAutoPtr<P>::operator*() const
    {
    return *m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HMallocAutoPtr<P>::operator==(const P* pi_pObj) const
    {
    return m_Ptr == pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HMallocAutoPtr<P>::operator==(const HMallocAutoPtr<P>& pi_rObj)
    {
    return m_Ptr == pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different from
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HMallocAutoPtr<P>::operator!=(const P* pi_pObj) const
    {
    return m_Ptr != pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different from
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HMallocAutoPtr<P>::operator!=(const HMallocAutoPtr<P>& pi_rObj)
    {
    return m_Ptr != pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Cast the auto ptr object to an object pointer.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>::operator P* () const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Method that returns the pointer.
//-----------------------------------------------------------------------------
template <class P>
inline P* HMallocAutoPtr<P>::get() const
    {
    return m_Ptr;
    }

// Private -----------------------------------------------------------------

//-----------------------------------------------------------------------------
// Assignment operator.
// Delete the current pointed object and point on the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline HMallocAutoPtr<P>& HMallocAutoPtr<P>::operator=(P* pi_Ptr)
    {
    free(m_Ptr);
    m_Ptr = pi_Ptr;

    return *this;
    }

//-----------------------------------------------------------------------------
// Method that releases the pointer but does not delete it.
//-----------------------------------------------------------------------------
template <class P>
inline P* HMallocAutoPtr<P>::release()
    {
    return reset(0);
    }

//-----------------------------------------------------------------------------
// Method that changes the pointer for the specified one, and returns the old
// one.  There is no deletion.
//-----------------------------------------------------------------------------
template <class P>
inline P* HMallocAutoPtr<P>::reset(P* pi_Ptr)
    {
    P* tmp = m_Ptr;
    m_Ptr  = pi_Ptr;

    return tmp;
    }

END_IMAGEPP_NAMESPACE