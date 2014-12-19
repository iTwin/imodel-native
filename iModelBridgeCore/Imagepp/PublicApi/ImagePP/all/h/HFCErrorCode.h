//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCode.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCErrorCodeID.h"
#include "HFCErrorCodeFlags.h"

//--------------------------------------
// parameters type
//--------------------------------------
typedef vector<WString>
HFCErrorCodeParameters;

class HFCErrorCode
    {
public:
    //--------------------------------------
    // Construction/Destruction
    //--------------------------------------

    _HDLLu                 HFCErrorCode();
    _HDLLu                 HFCErrorCode(const WString& pi_rCode);
    _HDLLu                 HFCErrorCode(const HFCErrorCode& pi_rObj);
    _HDLLu                 ~HFCErrorCode();


    //--------------------------------------
    // Operators
    //--------------------------------------

    // Assignment operator
    HFCErrorCode&   operator=(const HFCErrorCode& pi_rObj);


    //--------------------------------------
    // Get Methods
    //--------------------------------------

    const HFCErrorCodeID&
    GetID() const;
    const HFCErrorCodeFlags&
    GetFlags() const;
    const HFCErrorCodeID&
    GetModuleID() const;
    const HFCErrorCodeID&
    GetSpecificCode() const;
    const HFCErrorCodeParameters&
    GetParameters() const;
    const WString&   GetMessageText() const;


    //--------------------------------------
    // Set Methods
    //--------------------------------------

    void            SetID(const HFCErrorCodeID& pi_rID);
    void            SetFlags(const HFCErrorCodeFlags& pi_rFlags);
    void            SetModuleID(const HFCErrorCodeID& pi_rID);
    void            SetSpecificCode(const HFCErrorCodeID& pi_rCode);
    void            AddParameter(const WString& pi_rParam);
    void            SetMessageText(const WString& pi_rMessage);


    //--------------------------------------
    // Utility Methods
    //--------------------------------------

    // Output the value to a stream
//chck UNICODE
    void            GenerateString(wostringstream& po_rStream) const;

    // Indicates is a string is a valid error code string
    _HDLLu static bool    IsValidErrorCodeString(const WString& pi_rCode);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    HFCErrorCodeID          m_ID;
    HFCErrorCodeFlags       m_Flags;
    HFCErrorCodeID          m_ModuleID;
    HFCErrorCodeID          m_Code;
    HFCErrorCodeParameters  m_Parameters;
    WString                 m_Message;
    };
