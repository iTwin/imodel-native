/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/SessionManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Every DgnDb has one SessionManager associated with it.
// @bsiclass                                                    Keith.Bentley   10/16
//=======================================================================================
struct SessionManager
{
protected:
    DgnDbR m_dgndb;
    mutable SessionPtr m_current;

public: 
    SessionManager(DgnDbR dgndb) : m_dgndb(dgndb) {}

    SessionCPtr GetByName(Utf8StringCR name) const {return Session::GetByName(m_dgndb, name);}

    //! Clear the current Session.
    void ClearCurrent() {m_current = nullptr;}

    //! Set the current Session from a supplied Session element.
    void SetCurrent(SessionCPtr session) {m_current = session.IsValid() ? session->MakeCopy<Session>() : nullptr;}

    //! Set the current Session by name.
    void SetCurrent(Utf8StringCR name) {SetCurrent(GetByName(name));}

    DGNPLATFORM_EXPORT SessionR GetCurrent() const;
};

END_BENTLEY_DGN_NAMESPACE
