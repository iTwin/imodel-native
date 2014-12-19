//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/win/h/HFCFileLog.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <ImagePP/all/h/HFCExclusiveKey.h>

class HFCFileLog
    {
public:

    HFCFileLog();
    virtual ~HFCFileLog();

    void Open(WChar* pi_FileName, bool pi_NewLog = false);
    void Close();

    virtual void Log(WChar* pi_Text);
    virtual void Log(WChar* pi_FileName, uint32_t pi_Line, WChar* pi_Text);

protected:

    virtual void WriteHeader();
    virtual void WriteFooter();

private:

    HFCFileLog(const HFCFileLog&);
    HFCFileLog& operator=(const HFCFileLog&);

    WChar* m_LogName;
    bool  m_NewLog;
    bool m_IsOpen;

    FILE* m_File;
    HFCExclusiveKey m_InUse;
    };