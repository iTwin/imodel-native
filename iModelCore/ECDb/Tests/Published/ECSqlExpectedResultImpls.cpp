/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlExpectedResultImpls.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlExpectedResultImpls.h"

using namespace std;

BEGIN_ECDBUNITTESTS_NAMESPACE
//********************* ECSqlPrepareExpectedResult ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> PrepareECSqlExpectedResult::Create (SqlListCR expectedNativeSqlList, Category failingCategory, Utf8CP description)
    {
    return unique_ptr<IECSqlExpectedResult> (new PrepareECSqlExpectedResult (expectedNativeSqlList, failingCategory, description));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> PrepareECSqlExpectedResult::Create (Utf8CP expectedNativeSql, Category failingCategory, Utf8CP description)
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
unique_ptr<IECSqlExpectedResult> PrepareECSqlExpectedResult::CreateFailing (Category failingCategory, Utf8CP description)
    {
    SqlList expectedNativeSqlList;
    return Create (expectedNativeSqlList, failingCategory, description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> PrepareECSqlExpectedResult::CreateFailing (Utf8CP expectedNativeSql, Category failingCategory, Utf8CP description)
    {
    return Create (expectedNativeSql, failingCategory, description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::PrepareECSqlExpectedResult (SqlListCR expectedNativeSqlList, Category failingCategory, Utf8CP description)
    : IECSqlExpectedResult (Type::Prepare, failingCategory, description), m_expectedNativeSqlList (expectedNativeSqlList)
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
    : IECSqlExpectedResult (rhs), m_expectedNativeSqlList (rhs.m_expectedNativeSqlList)
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
        IECSqlExpectedResult::operator= (rhs);
        m_expectedNativeSqlList = rhs.m_expectedNativeSqlList;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
PrepareECSqlExpectedResult::PrepareECSqlExpectedResult (PrepareECSqlExpectedResult&& rhs)
    : IECSqlExpectedResult (move (rhs)), m_expectedNativeSqlList (move (rhs.m_expectedNativeSqlList))
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
        IECSqlExpectedResult::operator= (move (rhs));

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
unique_ptr<IECSqlExpectedResult> ResultCountECSqlExpectedResult::Create (int expectedColumnCount, int expectedRowCount)
    {
    return unique_ptr<IECSqlExpectedResult> (new ResultCountECSqlExpectedResult (expectedColumnCount, expectedRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> ResultCountECSqlExpectedResult::Create (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount /*= -1*/)
    {
    return unique_ptr<IECSqlExpectedResult> (new ResultCountECSqlExpectedResult (IECSqlBinder, description, expectedColumnCount, expectedRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> ResultCountECSqlExpectedResult::CreateFailing (Category failingCategory, Utf8CP description)
    {
    return unique_ptr<IECSqlExpectedResult> (new ResultCountECSqlExpectedResult (failingCategory, description));
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (int expectedColumnCount, int expectedRowCount)
    : IECSqlExpectedResult (Type::ResultCount), m_columnCount (expectedColumnCount), m_rowCount (expectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (Category IECSqlBinder, Utf8CP description, int expectedColumnCount, int expectedRowCount)
    : IECSqlExpectedResult (Type::ResultCount, IECSqlBinder, description), m_columnCount (expectedColumnCount), m_rowCount (expectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (Category failingCategory, Utf8CP description)
    : IECSqlExpectedResult (Type::ResultCount, failingCategory, description), m_columnCount (-1), m_rowCount (-1)
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
    : IECSqlExpectedResult (rhs), m_columnCount (rhs.m_columnCount), m_rowCount (rhs.m_rowCount)
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
        IECSqlExpectedResult::operator= (rhs);

        m_columnCount = rhs.m_columnCount;
        m_rowCount = rhs.m_rowCount;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ResultCountECSqlExpectedResult::ResultCountECSqlExpectedResult (ResultCountECSqlExpectedResult&& rhs)
    : IECSqlExpectedResult (move (rhs)), m_columnCount (move (rhs.m_columnCount)), m_rowCount (move (rhs.m_rowCount))
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
        IECSqlExpectedResult::operator= (move (rhs));

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


//********************* AffectedRowsECSqlExpectedResult ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> AffectedRowsECSqlExpectedResult::Create (int expectedAffectedRowCount)
    {
    return unique_ptr<IECSqlExpectedResult> (new AffectedRowsECSqlExpectedResult (expectedAffectedRowCount));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
unique_ptr<IECSqlExpectedResult> AffectedRowsECSqlExpectedResult::CreateFailing (Category failingCategory, Utf8CP description)
    {
    return unique_ptr<IECSqlExpectedResult> (new AffectedRowsECSqlExpectedResult (failingCategory, description));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult::AffectedRowsECSqlExpectedResult (int expectedAffectedRowCount)
    : IECSqlExpectedResult (Type::AffectedRowCount), m_affectedRowCount (expectedAffectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult::AffectedRowsECSqlExpectedResult (Category failingCategory, Utf8CP description)
    : IECSqlExpectedResult (Type::AffectedRowCount, failingCategory, description), m_affectedRowCount (-1)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult::~AffectedRowsECSqlExpectedResult ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult::AffectedRowsECSqlExpectedResult (AffectedRowsECSqlExpectedResult const& rhs)
    : IECSqlExpectedResult (rhs), m_affectedRowCount (rhs.m_affectedRowCount)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult& AffectedRowsECSqlExpectedResult::operator= (AffectedRowsECSqlExpectedResult const& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        IECSqlExpectedResult::operator= (rhs);

        m_affectedRowCount = rhs.m_affectedRowCount;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult::AffectedRowsECSqlExpectedResult (AffectedRowsECSqlExpectedResult&& rhs)
    : IECSqlExpectedResult (move (rhs)), m_affectedRowCount (move (rhs.m_affectedRowCount))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
AffectedRowsECSqlExpectedResult& AffectedRowsECSqlExpectedResult::operator= (AffectedRowsECSqlExpectedResult&& rhs)
    {
    if (this != &rhs)
        {
        //call base class operator
        IECSqlExpectedResult::operator= (move (rhs));

        m_affectedRowCount = move (rhs.m_affectedRowCount);
        }

    return *this;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
int AffectedRowsECSqlExpectedResult::GetExpectedAffectedRowCount () const
    {
    return m_affectedRowCount;
    }

END_ECDBUNITTESTS_NAMESPACE
