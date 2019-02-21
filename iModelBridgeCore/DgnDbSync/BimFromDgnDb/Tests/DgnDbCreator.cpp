/*------------------------------------ConverterApp--------------------------------------------------+
|
|     $Source: BimFromDgnDb/Tests/DgnDbCreator.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DgnDbCreator.h"
#include "DgnDbCreatorImpl.h"

DgnDbCreator::DgnDbCreator(const char* fileName)
    {
    m_creator = new  BentleyG0601::Dgn::DgnDb0601ToJsonTest::DgnDbCreatorImpl(fileName);
    }

bool DgnDbCreator::CreateDgnDb()
    {
    return m_creator->CreateDgnDb();
    }

bool DgnDbCreator::AddElement(const char* schemaName, const char* instanceXml)
    {
    return m_creator->AddElement(schemaName, instanceXml);
    }

bool DgnDbCreator::ImportSchema(char const*  schemaXml)
    {
    return m_creator->ImportSchema(schemaXml);
    }
