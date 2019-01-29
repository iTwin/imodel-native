/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/Status.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PCLWrapperPch.h"

#include "Status.h"

using namespace std;

BEGIN_PCLWRAPPER_NAMESPACE

//INTERFACE FUNCTION DEFINITION SECTION
IStatusPtr IStatus::CreateStatus(IStatus::Type& statusType)
    {
    return new Status(statusType);
    }

IStatus::Type IStatus::GetType() const
    {
    return _GetType();
    }
//INTERFACE FUNCTION DEFINITION SECTION - END

IStatus::Type Status::_GetType() const
    {
    return m_type;
    }

Status::Status(IStatus::Type pi_type)
    {
    m_type = pi_type;
    }

Status::~Status()
    {
    }

END_PCLWRAPPER_NAMESPACE
