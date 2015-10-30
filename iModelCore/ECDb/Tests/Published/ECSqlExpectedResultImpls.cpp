/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlExpectedResultImpls.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlExpectedResultImpls.h"

using namespace std;

BEGIN_ECSQLTESTFRAMEWORK_NAMESPACE

//********************* ECSqlPrepareExpectedResult ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> PrepareECSqlExpectedResult::Create (SqlListCR expectedNativeSqlList, Category failingCategory, Utf8CP description)
    {
    return unique_ptr<ECSqlExpectedResult> (new PrepareECSqlExpectedResult (expectedNativeSqlList, failingCategory, description));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> PrepareECSqlExpectedResult::Create (Utf8CP expectedNativeSql, Category failingCategory, Utf8CP description)
    {
    SqlList expectedNativeSqlList;
    if (expectedNativeSql != nullptr)
        {
        Utf8String expectedNativeSqlStr (expectedNativeSql);
        expectedNativeSqlStr.ToLower ();
        expectedNativeSqlList.push_back (expectedNativeSqlStr.c_str ());
        }

    return Create (expectedNativeSqlList, failingCategory, description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> PrepareECSqlExpectedResult::CreateFailing (Category failingCategory, Utf8CP description)
    {
    SqlList expectedNativeSqlList;
    return Create (expectedNativeSqlList, failingCategory, description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> PrepareECSqlExpectedResult::CreateFailing (Utf8CP expectedNativeSql, Category failingCategory, Utf8CP description)
    {
    return Create (expectedNativeSql, failingCategory, description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::PrepareECSqlExpectedResult (SqlListCR expectedNativeSqlList, Category failingCategory, Utf8CP description)
    : ECSqlExpectedResult (Type::Prepare, failingCategory, description), m_expectedNativeSqlList (expectedNativeSqlList)
    {
    for (auto& expectedNativeSql : m_expectedNativeSqlList)
        {
        expectedNativeSql.ToLower ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::~PrepareECSqlExpectedResult ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::PrepareECSqlExpectedResult (PrepareECSqlExpectedResult const& rhs)
    : ECSqlExpectedResult (rhs), m_expectedNativeSqlList (rhs.m_expectedNativeSqlList)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult& PrepareECSqlExpectedResult::operator= (PrepareECSqlExpectedResult const& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        ECSqlExpectedResult::operator= (rhs);
        m_expectedNativeSqlList = rhs.m_expectedNativeSqlList;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::PrepareECSqlExpectedResult (PrepareECSqlExpectedResult&& rhs)
    : ECSqlExpectedResult (move (rhs)), m_expectedNativeSqlList (move (rhs.m_expectedNativeSqlList))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult& PrepareECSqlExpectedResult::operator= (PrepareECSqlExpectedResult&& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        ECSqlExpectedResult::operator= (move (rhs));

        m_expectedNativeSqlList = move (rhs.m_expectedNativeSqlList);
        }

    return *this;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::SqlListCR PrepareECSqlExpectedResult::GetExpectedNativeSqlList () const
    {
    return m_expectedNativeSqlList;
    }


//********************* ResultCountECSqlExpectedResult ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> ResultCountECSqlExpectedResult::Create (int expectedColumnCount, int expectedRowCount)
    {
    return unique_ptr<ECSqlExpectedResult> (new ResultCountECSqlExpectedResult (expectedColumnCount, expectedRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> ResultCountECSqlExpectedResult::Create (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount /*= -1*/)
    {
    return unique_ptr<ECSqlExpectedResult> (new ResultCountECSqlExpectedResult (IECSqlBinder, description, expectedColumnCount, expectedRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<ECSqlExpectedResult> ResultCountECSqlExpectedResult::CreateFailing (Category failingCategory, Utf8CP description)
    {
    return unique_ptr<ECSqlExpectedResult> (new ResultCountECSqlExpectedResult (failingCategory, description));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (int expectedColumnCount, int expectedRowCount)
    : ECSqlExpectedResult (Type::ResultCount), m_columnCount (expectedColumnCount), m_rowCount (expectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount)
    : ECSqlExpectedResult (Type::ResultCount, IECSqlBinder, description), m_columnCount (expectedColumnCount), m_rowCount (expectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (Category failingCategory, Utf8CP description)
    : ECSqlExpectedResult (Type::ResultCount, failingCategory, description), m_columnCount (-1), m_rowCount (-1)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::~ResultCountECSqlExpectedResult ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (ResultCountECSqlExpectedResult const& rhs)
    : ECSqlExpectedResult (rhs), m_columnCount (rhs.m_columnCount), m_rowCount (rhs.m_rowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult& ResultCountECSqlExpectedResult::operator= (ResultCountECSqlExpectedResult const& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        ECSqlExpectedResult::operator= (rhs);

        m_columnCount = rhs.m_columnCount;
        m_rowCount = rhs.m_rowCount;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (ResultCountECSqlExpectedResult&& rhs)
    : ECSqlExpectedResult (move (rhs)), m_columnCount (move (rhs.m_columnCount)), m_rowCount (move (rhs.m_rowCount))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult& ResultCountECSqlExpectedResult::operator= (ResultCountECSqlExpectedResult&& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        ECSqlExpectedResult::operator= (move (rhs));

        m_columnCount = move (rhs.m_columnCount);
        m_rowCount = move (rhs.m_rowCount);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
int ResultCountECSqlExpectedResult::GetExpectedColumnCount () const
    {
    return m_columnCount;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
bool ResultCountECSqlExpectedResult::HasExpectedRowCount () const
    {
    return m_rowCount >= 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
int ResultCountECSqlExpectedResult::GetExpectedRowCount () const
    {
    return m_rowCount;
    }


END_ECSQLTESTFRAMEWORK_NAMESPACE