/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMException.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include ".\Bentley.Civil.DTM.h"
using namespace System;

BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE

[System::Serializable]
public ref class DTMException : public Bentley::Exceptions::ProgrammerException
    {
    public: DTMException(long errorStatus, long errorNumber, System::String^ message) : Bentley::Exceptions::ProgrammerException(message)
                {
                m_errorStatus = errorStatus;
                m_errorNumber = errorNumber;
                }

    //protected: DTMException(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context) : Bentley::Exceptions::ProgrammerException(info, context)
    //    {
    //    m_errorStatus = info->GetInt32(L"m_errorStatus");
    //    m_errorNumber = info->GetInt32(L"m_errorNumber");
    //    }
    //public: virtual void GetObjectData(System::Runtime::Serialization::SerializationInfo^ info, System::Runtime::Serialization::StreamingContext context) override
    //    {
    //    info->AddValue(L"m_errorStatus", m_errorStatus);
    //    info->AddValue(L"m_errorNumber", m_errorNumber);
    //    this->System::Exception::GetObjectData(info, context);
    //    }
    
    public: property long ErrorStatus
                {
                long get() { return m_errorStatus; }
                }
    public: property long ErrorNumber
                {
                long get() { return m_errorNumber; }
                }

    public:
    [EditorBrowsable(EditorBrowsableState::Never) ]
    static void CheckForErrorStatus(int status)
    {
        if(status != SUCCESS)
            {
            long errorStatus;
            long errorNumber;
            char* errorMessageP = NULL;
            bcdtmUtility_getLastDtmErrorMessage(&errorStatus, &errorNumber, &errorMessageP);

            System::String^ errorMessage = gcnew System::String(errorMessageP);
            bcMem_free(errorMessageP);
            throw ThrowingPolicy::Apply(gcnew DTMException(errorStatus, errorNumber, errorMessage));
            }
    }
    private:
        long m_errorStatus;
        long m_errorNumber;
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE
