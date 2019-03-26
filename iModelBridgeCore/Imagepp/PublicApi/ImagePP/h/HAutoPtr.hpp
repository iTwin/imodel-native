//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HAutoPtr.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for the class HAutoPtr.
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>::HAutoPtr(P* pi_Ptr)
    : m_Ptr(pi_Ptr)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.
// IMPORTANT:  This contructor transfers the ownership of the pointer to the
//             newly created object.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>::HAutoPtr(HAutoPtr<P>& pi_rObj)
    : m_Ptr(pi_rObj.release())
    {
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>::~HAutoPtr()
    {
    delete m_Ptr;
    }

//-----------------------------------------------------------------------------
// Assigment operator.
// Deletes the current pointed object and changes it to the specified one. We
// release the pointer of the specified HAutoPtr.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>& HAutoPtr<P>::operator=(HAutoPtr<P>& pi_rObj)
    {
    delete m_Ptr;
    reset(pi_rObj.release());

    return *this;
    }

//-----------------------------------------------------------------------------
// Assignment operator.
// Delete the current pointed object and point on the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>& HAutoPtr<P>::operator=(P* pi_Ptr)
    {
    delete m_Ptr;
    m_Ptr = pi_Ptr;

    return *this;
    }

//-----------------------------------------------------------------------------
// Operator used to use the pointer as a reference.
//-----------------------------------------------------------------------------
template <class P>
inline P& HAutoPtr<P>::operator*() const
    {
    return *m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator used to manipulate the pointed object.
//-----------------------------------------------------------------------------
template <class P>
inline P* HAutoPtr<P>::operator->() const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HAutoPtr<P>::operator==(const P* pi_pObj) const
    {
    return m_Ptr == pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that compares if the pointer stored in the object is the same as
// the specified one.
//-----------------------------------------------------------------------------
template <class P>
inline bool HAutoPtr<P>::operator==(const HAutoPtr<P>& pi_rObj)
    {
    return m_Ptr == pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different from
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HAutoPtr<P>::operator!=(const P* pi_pObj) const
    {
    return m_Ptr != pi_pObj;
    }

//-----------------------------------------------------------------------------
// Operator that indicates if the specified object pointer is different
// the one contained in the current object.
//-----------------------------------------------------------------------------
template <class P>
inline bool HAutoPtr<P>::operator!=(const HAutoPtr<P>& pi_rObj)
    {
    return m_Ptr != pi_rObj.m_Ptr;
    }

//-----------------------------------------------------------------------------
// Cast the auto ptr object to an object pointer.
//-----------------------------------------------------------------------------
template <class P>
inline HAutoPtr<P>::operator P* () const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Method that returns the pointer.
//-----------------------------------------------------------------------------
template <class P>
inline P* HAutoPtr<P>::get() const
    {
    return m_Ptr;
    }

//-----------------------------------------------------------------------------
// Method that releases the pointer but does not delete it.
//-----------------------------------------------------------------------------
template <class P>
inline P* HAutoPtr<P>::release()
    {
    return reset(0);
    }

//-----------------------------------------------------------------------------
// Method that changes the pointer to the specified one, and returns the old
// one.  There is no deletion.
//-----------------------------------------------------------------------------
template <class P>
inline P* HAutoPtr<P>::reset(P* pi_Ptr)
    {
    P* tmp = m_Ptr;
    m_Ptr  = pi_Ptr;

    return tmp;
    }

END_IMAGEPP_NAMESPACE