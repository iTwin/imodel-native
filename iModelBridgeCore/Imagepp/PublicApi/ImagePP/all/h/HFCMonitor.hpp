//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMonitor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCGenericMonitor
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 The default constructor for this class.  It can be used in some special
 circumstances, when the assignation to a key must be done in a stack
 level lower than the one where the monitor is needed to be released; the
 method Assign will have to be used to set the key.

 @see Assign
-----------------------------------------------------------------------------*/
template<class T>
inline HFCGenericMonitor<T>::HFCGenericMonitor()
    {
    m_pMonitor = 0;
    }


/**----------------------------------------------------------------------------
 A constructor for this class.  It assigns the monitor to an existent exclusive
 key, which will be claimed immediately if not already done.

 @param pi_rMonitor  Reference to an exclusive key.
 @param pi_IsClaimed Must be set to true if the key has already been claimed.
                     Default value is false.
-----------------------------------------------------------------------------*/
template<class T>
inline HFCGenericMonitor<T>::HFCGenericMonitor(T& pi_rMonitor, bool pi_IsClaimed)
    {
    m_pMonitor = 0;
    Assign(pi_rMonitor, pi_IsClaimed);
    }


/**----------------------------------------------------------------------------
 A constructor for this class.  It assigns the monitor to an existent exclusive
 key, which will be claimed immediately if not already done.

 @param pi_pMonitor  Pointer to an exclusive key.
 @param pi_IsClaimed Must be set to true if the key has already been claimed.
                     Default value is false.
-----------------------------------------------------------------------------*/
template<class T>
inline HFCGenericMonitor<T>::HFCGenericMonitor(T* pi_pMonitor, bool pi_IsClaimed)
    {
    m_pMonitor = 0;
    Assign(pi_pMonitor, pi_IsClaimed);
    }


/**----------------------------------------------------------------------------
 The destructor for this class.  will release the monitored key, if any.
-----------------------------------------------------------------------------*/
template<class T>
inline HFCGenericMonitor<T>::~HFCGenericMonitor()
    {
    ReleaseKey();
    }


/**----------------------------------------------------------------------------
 Releases the monitored key and disassociates it from this monitor.  This
 has the same effet than the destructor but without destruction of this
 object.  The monitor can be assigned again to another key.
-----------------------------------------------------------------------------*/
template<class T>
inline void HFCGenericMonitor<T>::ReleaseKey()
    {
    if (m_pMonitor != 0)
        m_pMonitor->ReleaseKey();
    m_pMonitor = 0;
    }


/**----------------------------------------------------------------------------
 Assigns the monitor to an existent exclusive key, which will be claimed
 immediately if not already done.  If there is already an assigned key,
 it will be released prior to assigning the new key to the monitor.

 @param pi_pMonitor  Pointer to an exclusive key.
 @param pi_IsClaimed Must be set to true if the key has already been claimed.
                     Default value is false.
-----------------------------------------------------------------------------*/
template<class T>
inline void HFCGenericMonitor<T>::Assign(T* pi_pMonitor, bool pi_IsClaimed)
    {
    HPRECONDITION(pi_pMonitor != 0);

    // Release the previous object
    ReleaseKey();

    // Assign and claim if needed
    m_pMonitor = pi_pMonitor;
    if (!pi_IsClaimed)
        m_pMonitor->ClaimKey();
    }


/**----------------------------------------------------------------------------
 Assigns the monitor to an existent exclusive key, which will be claimed
 immediately if not already done.  If there is already an assigned key,
 it will be released prior to assigning the new key to the monitor.

 @param pi_rMonitor  Reference to an exclusive key.
 @param pi_IsClaimed Must be set to true if the key has already been claimed.
                     Default value is false.
-----------------------------------------------------------------------------*/
template<class T>
inline void HFCGenericMonitor<T>::Assign(T& pi_rMonitor, bool pi_IsClaimed)
    {
    HPRECONDITION(&pi_rMonitor != 0);

    // Release the previous object
    ReleaseKey();

    // Assign and claim if needed
    m_pMonitor = &pi_rMonitor;
    if (!pi_IsClaimed)
        m_pMonitor->ClaimKey();
    }
END_IMAGEPP_NAMESPACE