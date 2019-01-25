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
    m_creator = new  BentleyG0601::Dgn::BimFromDgnDbTest::DgnDbCreatorImpl(fileName);
    }

void DgnDbCreator::AddElement()
    {

    }

void DgnDbCreator::ImportSchema(char const*  schemaXml)
    {
    m_creator->ImportSchema(schemaXml);
    }
