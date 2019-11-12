/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DataReader.h"
BEGIN_GEOMLIBS_TESTS_NAMESPACE

void DataReader::GetBeFileName(BeFileName& beFilename, WCharCP fileName)
    {
    BeFileName tempDir;
    BeTest::GetHost().GetTempDir (tempDir);
    BeSQLiteLib::Initialize (tempDir);

    BeTest::GetHost().GetDgnPlatformAssetsDirectory (beFilename);
    beFilename.AppendToPath(L"GeomLibsTestData");
    beFilename.AppendToPath(L"Performance");
    beFilename.AppendToPath (fileName);
    }

DbDataReader::DbDataReader(WCharCP dbName)
    {
    BeFileName dbFileName;
    GetBeFileName(dbFileName, dbName);
    DbResult stat = m_db.OpenBeSQLiteDb (dbFileName.GetNameUtf8 ().c_str (), Db::OpenParams (Db::OpenMode::Readonly));
    if (BeSQLite::DbResult::BE_SQLITE_OK == stat)
        {
        m_statement = std::unique_ptr<Statement> (new Statement());
        m_statement->Prepare(m_db, "SELECT ID, DESCRIPTION, XML FROM TestSamples");
        }
    }

bool DbDataReader::_GetNextTest(Utf8String& description, Utf8String& xml, int id)
    {
    if (!m_db.IsDbOpen())
        return false;

    if (m_statement == nullptr || !m_statement->IsPrepared())
        return false;

    if (BE_SQLITE_ROW != m_statement->Step())
        return false;

    id = m_statement->GetValueInt(0);
    description = m_statement->GetValueText(1);
    xml = m_statement->GetValueText(2);

    return true;
    }

XmlFileDataReader::XmlFileDataReader(WCharCP xmlFile)
    {
    BeFileName beFileName;
    GetBeFileName(beFileName, xmlFile);

    m_vectorIndex = 0;
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, beFileName.GetName());

    if (xmlStatus != BEXML_Success)
        return;

    BeXmlDom::IterableNodeSet nodes;
    xmlDom->SelectNodes (nodes, "/Samples/Sample", NULL);

    for (BeXmlNodeP node: nodes)
        {
        Utf8String description;
        Utf8String serialization;
        node->GetAttributeStringValue(description, "Description");
        node->GetFirstChild()->GetXmlString(serialization);
        m_xmlSamples.push_back(make_bpair(description, serialization));
        }
    }


bool XmlFileDataReader::_GetNextTest(Utf8String& description, Utf8String& xml, int id)
    {
    if (m_xmlSamples.size() == 0 || m_vectorIndex == m_xmlSamples.size())
        return false;
   
    bpair<Utf8String, Utf8String> testPair = m_xmlSamples[m_vectorIndex];
    description = testPair.first;
    xml = testPair.second;
    m_vectorIndex++;
    return true;
    }
END_GEOMLIBS_TESTS_NAMESPACE

