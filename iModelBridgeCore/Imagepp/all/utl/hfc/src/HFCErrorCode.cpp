//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCErrorCode.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <Imagepp/all/h/HFCErrorCode.h>
#include <Imagepp/all/h/HFCStringTokenizer.h>


//-----------------------------------------------------------------------------
// Public
// Default Constructor
//-----------------------------------------------------------------------------
HFCErrorCode::HFCErrorCode()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Copy Constructor
//-----------------------------------------------------------------------------
HFCErrorCode::HFCErrorCode(const HFCErrorCode& pi_rObj)
    : m_ID(pi_rObj.m_ID),
    m_Flags(pi_rObj.m_Flags),
    m_ModuleID(pi_rObj.m_ModuleID),
    m_Code(pi_rObj.m_Code),
    m_Message(pi_rObj.m_Message)
    {
    for (HFCErrorCodeParameters::const_iterator Itr = pi_rObj.m_Parameters.begin();
        Itr != pi_rObj.m_Parameters.end();
        Itr++)
        m_Parameters.push_back(*Itr);
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HFCErrorCode::~HFCErrorCode()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Assignment operator
//-----------------------------------------------------------------------------
HFCErrorCode& HFCErrorCode::operator=(const HFCErrorCode& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_ID         = pi_rObj.m_ID;
        m_Flags      = pi_rObj.m_Flags;
        m_ModuleID   = pi_rObj.m_ModuleID;
        m_Code       = pi_rObj.m_Code;
        m_Message    = pi_rObj.m_Message;

        m_Parameters.clear();
        for (HFCErrorCodeParameters::const_iterator Itr = pi_rObj.m_Parameters.begin();
            Itr != pi_rObj.m_Parameters.end();
            Itr++)
            m_Parameters.push_back(*Itr);
        }

    return (*this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeID& HFCErrorCode::GetID() const
    {
    return (m_ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeFlags& HFCErrorCode::GetFlags() const
    {
    return (m_Flags);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeID& HFCErrorCode::GetModuleID() const
    {
    return (m_ModuleID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeID& HFCErrorCode::GetSpecificCode() const
    {
    return (m_Code);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const HFCErrorCodeParameters& HFCErrorCode::GetParameters() const
    {
    return (m_Parameters);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const WString& HFCErrorCode::GetMessageText() const
    {
    return (m_Message);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::SetID(const HFCErrorCodeID& pi_rID)
    {
    HPRECONDITION(pi_rID != 0);

    m_ID = pi_rID;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::SetFlags(const HFCErrorCodeFlags& pi_rFlags)
    {
    m_Flags = pi_rFlags;

    // Re-generate the ID.
    uint32_t ID = ((uint32_t)m_Flags) << 28;
    ID += ((uint32_t)m_ModuleID) << 16;
    ID += (uint32_t)m_Code;
    m_ID = HFCErrorCodeID(ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::SetModuleID(const HFCErrorCodeID& pi_rID)
    {
    m_ModuleID = pi_rID;

    // Re-generate the ID.
    uint32_t ID = ((uint32_t)m_Flags) << 28;
    ID += ((uint32_t)m_ModuleID) << 16;
    ID += (uint32_t)m_Code;
    m_ID = HFCErrorCodeID(ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::SetSpecificCode(const HFCErrorCodeID& pi_rCode)
    {
    m_Code = pi_rCode;

    // Re-generate the ID.
    uint32_t ID = ((uint32_t)m_Flags) << 28;
    ID += ((uint32_t)m_ModuleID) << 16;
    ID += (uint32_t)m_Code;
    m_ID = HFCErrorCodeID(ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::AddParameter(const WString& pi_rParam)
    {
    HPRECONDITION(!pi_rParam.empty());

    WString::size_type Pos = 0;
    if ((Pos = pi_rParam.find(L' ')) == WString::npos)
        // No spaces were found so add the string directly.
        m_Parameters.push_back(pi_rParam);

    else
        {
        // at least a space was found, so loop on all the spaces and replace
        // them with underscores ('_')
        WString Param(pi_rParam);
        while ((Pos = Param.find(L' ', Pos)) != WString::npos)
            Param[Pos] = L'_';

        m_Parameters.push_back(Param);
        }
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::SetMessageText(const WString& pi_rMessage)
    {
    HPRECONDITION(!pi_rMessage.empty());

    m_Message = pi_rMessage;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor from an error code string
//-----------------------------------------------------------------------------
HFCErrorCode::HFCErrorCode(const WString& pi_rCode)
    {
    HPRECONDITION(!pi_rCode.empty());
    HPRECONDITION(IsValidErrorCodeString(pi_rCode));
    WString::size_type Pos = 1;  // start after the opening bracket

    // Set the flags, Module ID and the code
    m_Flags    = HFCErrorCodeFlags(pi_rCode.substr(Pos, 1));
    Pos += 1;
    m_ModuleID = HFCErrorCodeID(pi_rCode.substr(Pos, 3));
    Pos += 3;
    m_Code     = HFCErrorCodeID(pi_rCode.substr(Pos, 4));
    Pos += 4;

    // if there is space character now, we must verify the parameters
    if (pi_rCode[Pos] == L' ')
        {
        Pos++;

        // find the last bracket
        WString::size_type BracketPos = pi_rCode.find(L']', Pos);
        HASSERT(BracketPos != string::npos);

        // Tokenize the parameters separated by spaces
        WString ParamString(pi_rCode, Pos, BracketPos - Pos);
        HFCStringTokenizer ParamTokenizer(ParamString, L" ");
        WString Token;
        bool FirstToken = true;
        while (ParamTokenizer.Tokenize(Token))
            {
            // if the current token is empty accept because it simply is
            // two or more spaces between 2 parameters.
            if (!Token.empty())
                {
                if (FirstToken)
                    FirstToken = false;
                else
                    m_Parameters.push_back(Token);
                }
            }

        // position ourself at the bracket
        Pos = BracketPos;
        }

    // if there is a space after the bracket, then everything after
    // that space is the message
    Pos++; //skip the bracket
    if (Pos++ < pi_rCode.size())
        m_Message = pi_rCode.substr(Pos, pi_rCode.size() - Pos);

    // Generate the ID.
    uint32_t ID = ((uint32_t)m_Flags) << 28;
    ID += ((uint32_t)m_ModuleID) << 16;
    ID += (uint32_t)m_Code;
    m_ID = HFCErrorCodeID(ID);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HFCErrorCode::GenerateString(wostringstream& po_rStream) const
    {
    // Add the starting bracket
    po_rStream << L"[";

    // add the 8 bytes of the flags, module id and code
    m_Flags.GenerateString(po_rStream);
    m_ModuleID.GenerateString(po_rStream, 3);
    m_Code.GenerateString(po_rStream, 4);

    // add the parameters, if any
    if (m_Parameters.size() > 0)
        {
        po_rStream << L" ";
        po_rStream << m_Parameters.size();

        for (HFCErrorCodeParameters::const_iterator Itr = m_Parameters.begin();
             Itr != m_Parameters.end();
             Itr++)
            po_rStream << L" " << *Itr;
        }

    // Add the closing bracket
    po_rStream << L"]";

    // Add the message if any
    if (!m_Message.empty())
        po_rStream << L" " << m_Message;
    }


//-----------------------------------------------------------------------------
// Static
// verifies that the string is indeed an error code string
//-----------------------------------------------------------------------------
bool HFCErrorCode::IsValidErrorCodeString(const WString& pi_rCode)
    {
    HPRECONDITION(!pi_rCode.empty());
    WString::size_type Pos = 0;

    // 1 . string must have at least 10 characters to even bother with it.
    //     The opening and closing brackets (2) + the flags, module and code (8)
    // 2.  must start with an opening bracket.
    bool Result = ((pi_rCode.size() >= 10) &&
                    (pi_rCode[Pos] == L'['));

    // the next eigth characters must be hexadecimal characters
    for (Pos = 1; (Result) && (Pos < 9); Pos++)
        Result = (iswxdigit(pi_rCode[Pos]) != 0);

    // if there is space character now, we must verify the parameters
    if (pi_rCode[Pos] == L' ')
        {
        Pos++;

        // find the last bracket
        WString::size_type BracketPos = pi_rCode.find(L']', Pos);
        Result = (BracketPos != WString::npos);
        if (Result)
            {
            // Tokenize the parameters separated by spaces
            WString ParamString(pi_rCode, Pos, BracketPos - Pos);
            HFCStringTokenizer ParamTokenizer(ParamString, L" ");
            size_t TokenCount = 0;
            size_t ParamCount = 0;

            WString Token;
            while (Result && ParamTokenizer.Tokenize(Token))
                {
                // if the current token is empty accept because it simply is
                // two or more spaces between 2 parameters.
                if (!Token.empty())
                    {
                    // if token count is zero, then the current token must be a digit
                    if (TokenCount == 0)
                        {
                        for (WString::size_type TokenPos = 0;
                             (Result) && (TokenPos < Token.size());
                             TokenPos++)
                            Result = iswdigit(Token[TokenPos]) != 0;

                        ParamCount = BeStringUtilities::Wtoi(Token.c_str());
                        }
//                    else nothing, can be anything
//                    {
//                    }

                    // Increment the number of token
                    TokenCount++;
                    }
                }

            // verify that the token count corresponds to the param count
            if (Result)
                Result = (ParamCount == (TokenCount - 1));
            }

        // position ourself at the bracket
        Pos = BracketPos;
        }

    // The current character must be a bracket. And if there is more characters
    // in the string, the one after must be a space.  The rest is the message.
    if (Result)
        {
        Result = (((Pos < pi_rCode.size()) && (pi_rCode[Pos++] == L']')) && // current char must exist and be '[
                  ((Pos >= pi_rCode.size()) || (pi_rCode[Pos] == L' ')) );
        }

    return (Result);
    }