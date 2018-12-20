/*--------------------------------------------------------------------------------------+
|
|     $Source: PCLWrapper/Status.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <PCLWrapper\IStatus.h>

BEGIN_PCLWRAPPER_NAMESPACE

class Status;

typedef RefCountedPtr<Status> StatusPtr;

class Status : public IStatus
    {
    private :

        IStatus::Type m_type;

    protected :

        virtual IStatus::Type _GetType() const;

    public :

        Status(IStatus::Type pi_status);

        ~Status();
    };

END_PCLWRAPPER_NAMESPACE
