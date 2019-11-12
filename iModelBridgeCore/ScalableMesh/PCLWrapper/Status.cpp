/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
