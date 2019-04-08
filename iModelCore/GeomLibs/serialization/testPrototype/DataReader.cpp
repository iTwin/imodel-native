
/*--------------------------------------------------------------------------------------+
|
|     $Source: serialization/testPrototype/DataReader.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DataReader.h"
BEGIN_GEOMLIBS_TESTS_NAMESPACE

//DbDataReader::DbDataReader(WCharCP dbName)
//    {
//    BeFileName dbFileName;
//    GetBeFileName(dbFileName, dbName);
//    DbResult stat = m_db.OpenBeSQLiteDb (dbFileName.GetNameUtf8 ().c_str (), Db::OpenParams (Db::OPEN_Readonly, DefaultTxn_Yes));
//    if (BeSQLite::DbResult::BE_SQLITE_OK == stat)
//        {
//        m_statement = std::unique_ptr<Statement> (new Statement());
//        m_statement->Prepare(m_db, "SELECT ID, DESCRIPTION, XML FROM TestSamples");
//        }
//    }
//
//bool DbDataReader::_GetNextTest(Utf8String& description, Utf8String& xml, int id)
//    {
//    if (!m_db.IsDbOpen())
//        return false;
//
//    if (m_statement == nullptr || !m_statement->IsPrepared())
//        return false;
//
//    if (BE_SQLITE_ROW != m_statement->Step())
//        return false;
//
//    id = m_statement->GetValueInt(0);
//    description = m_statement->GetValueText(1);
//    xml = m_statement->GetValueText(2);
//
//    return true;
//    }
//
XmlFileDataReader::XmlFileDataReader(BeFileName beFileName)
    {
    m_vectorIndex = 0;
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, beFileName.GetName());

    if (xmlStatus != BEXML_Success)
        return;

    BeXmlDom::IterableNodeSet nodes;
    xmlDom->SelectNodes (nodes, "/Samples/Sample", NULL);

    for (BeXmlNodeP node: nodes)
        {
        WString description;
        WString serialization;
        node->GetAttributeStringValue(description, "Description");
        node->GetFirstChild()->GetXmlString(serialization);
        m_xmlSamples.push_back(make_bpair(description, serialization));
        }
    }


bool XmlFileDataReader::_GetNextTest(Utf8String& description, Utf8String& xml, int id)
    {
    if (m_xmlSamples.size() == 0 || m_vectorIndex == m_xmlSamples.size())
        return false;
   
    bpair<WString, WString> testPair = m_xmlSamples[m_vectorIndex];
    description.Assign(testPair.first.c_str());
    xml.Assign(testPair.second.c_str());
    m_vectorIndex++;
    return true;
    }
END_GEOMLIBS_TESTS_NAMESPACE

