/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeCriticalSection.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================
//! A critical section/mutual exclusion object.
//  @bsiclass 
//=======================================================================================
struct BeCriticalSection
    {
    friend struct BeConditionVariable;

private:
    static BeCriticalSection* s_systemCS;

    // This member is treated as an opaque OS-dependent memory buffer, and must be aligned (we're assuming 8-byte aligned), so use double.
    double  m_osCSect[8];
    bool    m_isValid;

    void* GetNativeCriticalSection()    { return m_osCSect; }

public:
    //! Constructs the critical section object
    BENTLEYDLL_EXPORT BeCriticalSection ();
    //! Destroys the critical section object
    BENTLEYDLL_EXPORT ~BeCriticalSection ();
    //! Enters the critical section
    BENTLEYDLL_EXPORT void Enter ();
    //! Leaves the critical section
    BENTLEYDLL_EXPORT void Leave ();

    //! False if creation failed.
    bool GetIsValid () const {return m_isValid;}

#if defined (_WIN32) // Windows && WinRT
    //! Query if the current thread owns this critical section
    //! @private
    BENTLEYDLL_EXPORT bool HasEntered() const;
#endif

    //! Initialize the Bentley system CS. Call this once at the start of the program.
    BENTLEYDLL_EXPORT static void StartupInitializeSystemCriticalSection ();

    //! Get the Bentley system CS. This can be used to bootstrap other CriticalSections.
    //! @remarks Program must call StartupInitializeSystemCriticalSection before calling this function.
    BENTLEYDLL_EXPORT static BeCriticalSection& GetSystemCriticalSection ();
    };

//=======================================================================================
//! A helper class that ties ownership of a BeCriticalSection object to the scope of a variable.
//  @bsiclass 
//=======================================================================================
struct BeCriticalSectionHolder
    {
private:
    BeCriticalSection&    m_csect;

public:
    //! Constructs a BeCriticalSectionHolder. This enters the critical section
    BeCriticalSectionHolder (BeCriticalSection& p) : m_csect(p) {m_csect.Enter ();}

    //! Destroys the BeCriticalSectionHolder. This leaves the critical section
    ~BeCriticalSectionHolder () {m_csect.Leave ();}
    };

END_BENTLEY_NAMESPACE
