/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley/BeTest.h>
#include <Bentley/BeFileName.h>
#include <BeSQLite/BeSQLite.h>
#include <BeXml/BeXml.h>
#include "GeomLibsTests.h"

USING_NAMESPACE_BENTLEY_SQLITE 

BEGIN_GEOMLIBS_TESTS_NAMESPACE

struct DataReader
    {
    protected:
        virtual bool _GetNextTest(Utf8String& description, Utf8String& xml, int id) { return false; };
        void GetBeFileName(BeFileName& beFilename, WCharCP fileName);
        DataReader() {} ; 

    public:
        bool GetNextTest(Utf8String& description, Utf8String& xml, int id) { return _GetNextTest(description, xml, id); }

    };

struct DbDataReader : DataReader
    {
    private:
        BeSQLite::Db m_db;
        std::unique_ptr<Statement> m_statement;

    protected:
        virtual bool _GetNextTest(Utf8String& description, Utf8String& xml, int id) override;

    public:
        DbDataReader(WCharCP fileName);
    };

struct XmlFileDataReader : DataReader
    {
    private:
        bvector<bpair<Utf8String, Utf8String>> m_xmlSamples;
        int m_vectorIndex;
    protected:
        virtual bool _GetNextTest(Utf8String& description, Utf8String& xml, int id) override;

    public:
        XmlFileDataReader(WCharCP fileName);

    };

END_GEOMLIBS_TESTS_NAMESPACE
