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
// @bsiclass                                                    Keith.Bentley   10/16
//=======================================================================================
struct SessionManager
{
protected:
    DgnDbR m_dgndb;
    SessionPtr m_current;

public: 
    SessionManager(DgnDbR dgndb) : m_dgndb(dgndb) {}

    SessionCPtr GetByName(Utf8StringCR name) const {return Session::GetByName(m_dgndb, name);}

    void ClearCurrent() {m_current = nullptr;}

    SessionPtr GetCurrent() const {return m_current;}

    SessionPtr LoadCurrent(Utf8StringCR name) {auto session=GetByName(name); m_current = session.IsValid() ? session->MakeCopy<Session>() : nullptr; return m_current;}
};

END_BENTLEY_DGN_NAMESPACE
