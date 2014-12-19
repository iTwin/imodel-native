//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCErrorCodeMacros.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Starts the declaration for a single inheritance class
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_START_DECLARATION_CLASS(pi_Name, pi_Ancestor)\
    class pi_Name : public pi_Ancestor\
    {\
        public:\
                            pi_Name();\
            virtual         ~pi_Name();\
            static uint32_t GetCode();


//-----------------------------------------------------------------------------
// Starts the declaration for a double inheritance class
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_START_DECLARATION_CLASS2(pi_Name, pi_Ancestor1, pi_Ancestor2)\
    class pi_Name : public pi_Ancestor1,\
                    public pi_Ancestor2\
    {\
        public:\
                            pi_Name();\
            virtual         ~pi_Name();\
            static uint32_t GetCode();


//-----------------------------------------------------------------------------
// End a HFCErrorCode declaration
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_END_DECLARATION\
    };


//-----------------------------------------------------------------------------
// Overrides the method for building error code from HFCExceptions
// can be used with Builders and BuilderHandlers
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_DECLARE_BUILDER_EXCEPTION\
    virtual HFCErrorCode* BuildFromException  (const HFCException& pi_rException) const;


//-----------------------------------------------------------------------------
// Overrides the method for building error code from OS exceptions
// can be used with Builders and BuilderHandlers
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_DECLARE_BUILDER_OS\
    virtual HFCErrorCode* BuildFromOSException(uint32_t pi_Exception) const;


//-----------------------------------------------------------------------------
// Overrides the method for building error code from HSTATUS code
// can be used with Builders and BuilderHandlers
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_DECLARE_BUILDER_HSTATUS\
    virtual HFCErrorCode* BuildFromStatus(HSTATUS pi_Status) const;


//-----------------------------------------------------------------------------
// Overrides the method for handling error code
// can be used with Builders and BuilderHandlers
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_DECLARE_HANDLER\
    virtual HSTATUS Handle(const HFCErrorCode& pi_rCode) const;


//-----------------------------------------------------------------------------
// Implements the constructor and destructor for a single inheritance class
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_IMPLEMENT_CLASS(pi_Name, pi_Ancestor, pi_Code)\
    pi_Name::pi_Name()\
        : pi_Ancestor()\
    {\
    }\
    \
    pi_Name::~pi_Name()\
    {\
    }\
    uint32_t pi_Name::GetCode()\
    {\
        return pi_Code;\
    }


//-----------------------------------------------------------------------------
// Implements the constructor and destructor for a double inheritance class
//-----------------------------------------------------------------------------
#define HFC_ERRORCODE_IMPLEMENT_CLASS2(pi_Name, pi_Ancestor1, pi_Ancestor2, pi_Code)\
    pi_Name::pi_Name()\
        : pi_Ancestor1(),\
          pi_Ancestor2()\
    {\
    }\
    \
    pi_Name::~pi_Name()\
    {\
    }\
    uint32_t pi_Name::GetCode()\
    {\
        return pi_Code;\
    }
