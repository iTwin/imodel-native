//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMonitor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCGenericMonitor, HFCMonitor
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/**

    This is the class to use to obtain automatic management of exclusive
    keys, by creating "monitors" on these keys, to help the synchronization
    of multi-threaded programs based on exclusive keys to manage concurrent
    access of resources.  A monitor is an object that claims a key as soon
    as it is associated to that key (which becomes a "monitored key", and
    that releases that key at destruction time.

    The HFCGenericMonitor class is a generic one that works with any
    companion class that provides the @r{ClaimKey} and @r{ReleaseKey} methods.
    However, most of the time you will use the HFCMonitor type which is
    the default implementation that uses keys of type HFCExclusiveKey.

    A monitor is useful to associate the use of an exclusive key with the
    use of the program's stack.  Instead of claiming and releasing the key
    manually, the program can simply create a monitor and let it die when it
    gets out of scope when the stack unwind, avoiding the need to deal with
    different execution paths (even C++ exceptions).

    The following example illustrates its use:

    @code
|    void MyClass::DoSomething()
|    {
|      ...
|      bool ok = false;
|      HFCMonitor m1(m_Key); // m_Key is a member of the class
|      while(!ok)
|      {
|         HFCExclusiveKey LocalKey;
|         HFCMonitor m2(LocalKey);
|         ...
|         if (...)
|            ok = true;
|      }
|      ...
|    }
    @end

    This example shows some kind of complexity in the execution paths.  For
    example, if the first "if" in the while block causes the "return" to be
    executed, both monitors will go out of scope and will release the keys.
    If the "break" statement is executed, the LocalKey will be released
    because m2 will go out of scope.  Notice that the LocalKey is
    automatically claimed and released at each iteration of the "while"
    loop.  The use of monitor makes the code much simpler to use than the
    manual triggering of the keys.

*/


template<class T> class HNOVTABLEINIT HFCGenericMonitor
    {
public:

    //:> Construction/Destruction

    HFCGenericMonitor();
    HFCGenericMonitor(T* pi_pMonitor, bool pi_IsClaimed = false);
    HFCGenericMonitor(T& pi_hMonitor, bool pi_IsClaimed = false);
    ~HFCGenericMonitor();

    //:> Methods

    //:> Release the claim on the current object
    void            ReleaseKey();

    //:> Assign a new monitored object
    void            Assign(T* pi_pMonitor, bool pi_IsClaimed = false);
    void            Assign(T& pi_rMonitor, bool pi_IsClaimed = false);


private:

    T*              m_pMonitor;
    };

END_IMAGEPP_NAMESPACE

#include "HFCMonitor.hpp"

// *** Moved these default monitor to their respective class header.
// Define the default HFCMonitor that works on HFCExclusiveKeys
// #include <Imagepp/all/h/HFCExclusiveKey.h>
// typedef HFCGenericMonitor<HFCExclusiveKey> HFCMonitor;

// #include "HFCBinStreamLockManager.h"
// typedef HFCGenericMonitor<HFCBinStreamLockManager>
// HFCLockMonitor;


