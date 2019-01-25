/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/Tests/DgnDbCreatorImpl.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbCreatorImpl.h"

BEGIN_DGNDB0601_TO_JSON_NAMESPACE

DgnDbCreatorImpl::DgnDbCreatorImpl(const char* fileName) : m_dgndbFilename(fileName)
    {
    }

void DgnDbCreatorImpl::ImportSchema(const char* schemaXml)
    {

    }
END_DGNDB0601_TO_JSON_NAMESPACE
