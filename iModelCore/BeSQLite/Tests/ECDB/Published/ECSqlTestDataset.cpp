/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ECDB/Published/ECSqlTestDataset.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECSqlTestDataset.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE
//********************* IECSqlExpectedResult ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::IECSqlExpectedResult (IECSqlExpectedResult::Type type)
: m_type (type), m_category (Category::Supported)
    {}

IECSqlExpectedResult::IECSqlExpectedResult (IECSqlExpectedResult::Type type, IECSqlExpectedResult::Category category, Utf8CP description)
: m_type (type), m_category (category), m_description (description)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::IECSqlExpectedResult (IECSqlExpectedResult const& rhs)
: m_type (rhs.m_type), m_category (rhs.m_category), m_description (rhs.m_description)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult& IECSqlExpectedResult::operator= (IECSqlExpectedResult const& rhs)
    {
    if (this != &rhs)
        {
        m_type = rhs.m_type;
        m_category = rhs.m_category;
        m_description = rhs.m_description;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::IECSqlExpectedResult (IECSqlExpectedResult&& rhs)
: m_type (move (rhs.m_type)), m_category (move (rhs.m_category)), m_description (move (rhs.m_description))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult& IECSqlExpectedResult::operator= (IECSqlExpectedResult&& rhs)
    {
    if (this != &rhs)
        {
        m_type = move (rhs.m_type);
        m_category = move (rhs.m_category);
        m_description = move (rhs.m_description);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::Type IECSqlExpectedResult::GetType () const
    {
    return m_type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::Category IECSqlExpectedResult::GetCategory () const
    {
    return m_category;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP IECSqlExpectedResult::GetDescription () const
    {
    return m_description.c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
bool IECSqlExpectedResult::IsExpectedToSucceed () const
    {
    return m_category == Category::Supported || m_category == Category::SupportedButMightBecomeInvalid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String IECSqlExpectedResult::CategoryToString (IECSqlExpectedResult::Category category)
    {
    switch (category)
        {
        default:
        case Category::Supported:
            return "Supported ECSQL";
        case Category::SupportedButMightBecomeInvalid:
            return "Supported ECSQL (Might become invalid though)";
        case Category::NotYetSupported:
            return "Not yet supported ECSQL";
        case Category::Invalid:
            return "Invalid ECSQL";
        }
    }

//********************* ECSqlExpectedResultsDictionary ************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlExpectedResultsDictionary::ECSqlExpectedResultsDictionary (ECSqlExpectedResultsDictionary&& rhs)
    : m_innerDictionary (move (rhs.m_innerDictionary)), m_lastAdded (move (rhs.m_lastAdded))
    {
    //raw pointers are not nulled out by std::move
    //m_lastAdded is not owned
    rhs.m_lastAdded = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlExpectedResultsDictionary& ECSqlExpectedResultsDictionary::operator= (ECSqlExpectedResultsDictionary&& rhs)
    {
    if (this != &rhs)
        {
        m_innerDictionary = move (rhs.m_innerDictionary);
        m_lastAdded = move (rhs.m_lastAdded);

        //raw pointers are not nulled out by std::move
        rhs.m_lastAdded = nullptr;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlExpectedResultsDictionary::Add (unique_ptr<IECSqlExpectedResult> expectedResult)
    {
    m_lastAdded = expectedResult.get ();
    m_innerDictionary[expectedResult->GetType ()] = move (expectedResult);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult const* ECSqlExpectedResultsDictionary::GetLastAdded () const
    {
    return m_lastAdded;
    }


//********************* ECSqlTestItem ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestItem::ECSqlTestItem (Utf8CP ecsql)
    : m_ecsql (ecsql), m_rollbackAfterwards (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestItem::ECSqlTestItem (Utf8CP ecsql, bool rollbackAfterwards)
    : m_ecsql (ecsql), m_rollbackAfterwards (rollbackAfterwards)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestItem::ECSqlTestItem (ECSqlTestItem&& rhs)
    : m_ecsql (move (rhs.m_ecsql)), m_parameterValueList (move (rhs.m_parameterValueList)), m_expectedResults (move(rhs.m_expectedResults)), m_rollbackAfterwards (move (rhs.m_rollbackAfterwards))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestItem& ECSqlTestItem::operator= (ECSqlTestItem&& rhs)
    {
    if (this != &rhs)
        {
        m_ecsql = move (rhs.m_ecsql);
        m_parameterValueList = move (rhs.m_parameterValueList);
        m_expectedResults = move (rhs.m_expectedResults);
        m_rollbackAfterwards = move (rhs.m_rollbackAfterwards);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTestItem::AddParameterValue (ParameterValue&& parameterValue)
    {
    m_parameterValueList.push_back (move (parameterValue));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlTestItem::AddExpectedResult (std::unique_ptr<IECSqlExpectedResult> expectedResult)
    {
    m_expectedResults.Add (move (expectedResult));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ECSqlTestItem::GetECSql () const
    {
    return m_ecsql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  08/13
//+---------------+---------------+---------------+---------------+---------------+------
vector<ECSqlTestItem::ParameterValue> const& ECSqlTestItem::GetParameterValues() const
    {
    return m_parameterValueList;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlExpectedResultsDictionary const& ECSqlTestItem::GetExpectedResults () const
    {
    return m_expectedResults;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlTestItem::GetRollbackAfterwards () const
    {
    return m_rollbackAfterwards;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlExpectedResult::Category ECSqlTestItem::GetExpectedResultCategory () const
    {
    auto lastAddedExpectedResult = m_expectedResults.GetLastAdded ();
    if (lastAddedExpectedResult == nullptr)
        return IECSqlExpectedResult::Category::Supported;

    return lastAddedExpectedResult->GetCategory ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  07/13
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlTestItem::GetExpectedResultDescription () const
    {
    auto lastAddedExpectedResult = m_expectedResults.GetLastAdded ();
    if (lastAddedExpectedResult == nullptr)
        return nullptr;

    return lastAddedExpectedResult->GetDescription ();
    }



//********************* ECSqlTestDataset ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset::ECSqlTestDataset (ECSqlTestDataset&& rhs)
    : m_innerList (move (rhs.m_innerList))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestDataset& ECSqlTestDataset::operator= (ECSqlTestDataset&& rhs)
    {
    if (this != &rhs)
        {
        m_innerList = move (rhs.m_innerList);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlTestItem& ECSqlTestDataset::AddTestItem (ECSqlTestItem& testItem)
    {
    m_innerList.push_back (std::move (testItem));

    return m_innerList[m_innerList.size () - 1];
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle                  04/13
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<ECSqlTestItem> const& ECSqlTestDataset::GetTestItems () const
    {
    return m_innerList;
    }

END_ECDBUNITTESTS_NAMESPACE