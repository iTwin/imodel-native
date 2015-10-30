/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlExpectedResultImpls.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECSqlTestDataset.h"

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE
//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct PrepareECSqlExpectedResult : public ECSqlExpectedResult
    {
public:
    typedef bvector<Utf8String> SqlList;
    typedef SqlList const& SqlListCR;

private:
    SqlList m_expectedNativeSqlList;

    //! Initializes a new instance of the PrepareECSqlExpectedResult type representing a test which is expected to fail
    //! @param[in] expectedNativeSqlList List of expected native SQL string resulting from translation of ECSQL.
    //!            Pass empty list if the translation is not expected to succeed.
    //! @param[in] failingCategory Failure IECSqlBinder
    PrepareECSqlExpectedResult (SqlListCR expectedNativeSqlList, Category failingCategory, Utf8CP description);

public:
    virtual ~PrepareECSqlExpectedResult ();

    PrepareECSqlExpectedResult (PrepareECSqlExpectedResult const& rhs);
    PrepareECSqlExpectedResult& operator= (PrepareECSqlExpectedResult const& rhs);
    PrepareECSqlExpectedResult (PrepareECSqlExpectedResult&& rhs);
    PrepareECSqlExpectedResult& operator= (PrepareECSqlExpectedResult&& rhs);

    SqlListCR GetExpectedNativeSqlList () const;

    //! Initializes a new instance of the PrepareECSqlExpectedResult type.
    //! @param[in] expectedNativeSql Expected native SQL string resulting from translation of ECSQL.
    //!            Pass nullptr if the translation is not expected to succeed.
    static std::unique_ptr<ECSqlExpectedResult> Create (Utf8CP expectedNativeSql, Category failingCategory = Category::Supported, Utf8CP description = nullptr);
    //! Initializes a new instance of the PrepareECSqlExpectedResult type.
    //! @param[in] expectedNativeSqlList List of expected native SQL string resulting from translation of ECSQL.
    //!            Pass empty list if the translation is not expected to succeed.
    static std::unique_ptr<ECSqlExpectedResult> Create (SqlListCR expectedNativeSqlList, Category failingCategory = Category::Supported, Utf8CP description = nullptr);
    //! Initializes a new instance of the PrepareECSqlExpectedResult type which indicates that the test
    //! is expected to fail
    //! @param[in] failingCategory Failure IECSqlBinder
    static std::unique_ptr<ECSqlExpectedResult> CreateFailing (Category failingCategory, Utf8CP description = nullptr);

    //! Initializes a new instance of the PrepareECSqlExpectedResult type which indicates that the test
    //! is expected to fail
    //! @param[in] expectedNativeSql Expected native SQL string resulting from translation of ECSQL.
    //!            Pass nullptr if Prepare is not expected to succeed. Pass expected native SQL if prepare is expected to succeed,
    //!            but is expected to generate wrong native SQL.
    //! @param[in] failingCategory Failure IECSqlBinder
    static std::unique_ptr<ECSqlExpectedResult> CreateFailing (Utf8CP expectedNativeSql, Category failingCategory, Utf8CP description = nullptr);

    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ResultCountECSqlExpectedResult : public ECSqlExpectedResult
    {
private:
    int m_columnCount;
    int m_rowCount;

    //! Initializes a new instance of the ResultCountECSqlExpectedResult type.
    //! @param[in] expectedColumnCount Number of expected columns in result set
    //! @param[in] expectedRowCount Number of expected rows in result set. Pass -1 if row count test should be skipped
    ResultCountECSqlExpectedResult (int expectedColumnCount, int expectedRowCount);

    ResultCountECSqlExpectedResult (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount);

    //! Initializes a new instance of the ResultCountECSqlExpectedResult type representing a test which is expected to fail
    //! @param[in] failingCategory Failure IECSqlBinder
    explicit ResultCountECSqlExpectedResult (Category failingCategory, Utf8CP description);

public:
    ResultCountECSqlExpectedResult (ResultCountECSqlExpectedResult const& rhs);
    ResultCountECSqlExpectedResult& operator= (ResultCountECSqlExpectedResult const& rhs);
    ResultCountECSqlExpectedResult (ResultCountECSqlExpectedResult&& rhs);
    ResultCountECSqlExpectedResult& operator= (ResultCountECSqlExpectedResult&& rhs);

    virtual ~ResultCountECSqlExpectedResult ();

    int GetExpectedColumnCount () const;
    bool HasExpectedRowCount () const;
    int GetExpectedRowCount () const;

    //! Initializes a new instance of the ResultCountECSqlExpectedResult type.
    //! @param[in] expectedColumnCount Number of expected columns in result set
    //! @param[in] expectedRowCount Number of expected rows in result set. Pass -1 if row count test should be skipped
    static std::unique_ptr<ECSqlExpectedResult> Create (int expectedColumnCount, int expectedRowCount = -1);
    //! Initializes a new instance of the ResultCountECSqlExpectedResult type.
    //! @param[in] IECSqlBinder Expected result IECSqlBinder
    //! @param[in] expectedColumnCount Number of expected columns in result set
    //! @param[in] expectedRowCount Number of expected rows in result set. Pass -1 if row count test should be skipped
    static std::unique_ptr<ECSqlExpectedResult> Create (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount = -1);
    //! Initializes a new instance of the ResultCountECSqlExpectedResult type which indicates that the test
    //! is expected to fail
    //! @param[in] failingCategory Failure IECSqlBinder
    static std::unique_ptr<ECSqlExpectedResult> CreateFailing (Category failingCategory, Utf8CP description = nullptr);
    };

END_ECSQLTESTFRAMEWORK_NAMESPACE