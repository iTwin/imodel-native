/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/ECSqlTestDataset.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE
//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlExpectedResult
    {
public:
    enum class Type
        {
        Generic,
        Prepare,
        ResultCount
        };

    enum class Category
        {
        Supported,
        SupportedButMightBecomeInvalid,
        NotYetSupported,
        Invalid
        };

private:
    Type m_type;
    Category m_category;
    Utf8String m_description;

protected:
    explicit ECSqlExpectedResult(Type type);
    ECSqlExpectedResult(Type type, Category category, Utf8CP description);

public:
    virtual ~ECSqlExpectedResult() {}
    ECSqlExpectedResult(ECSqlExpectedResult const& rhs);
    ECSqlExpectedResult& operator= (ECSqlExpectedResult const& rhs);
    ECSqlExpectedResult(ECSqlExpectedResult&& rhs);
    ECSqlExpectedResult& operator= (ECSqlExpectedResult&& rhs);

    //! Initializes a new instance of the ECSqlExpectedResult type indicating the test being expected to succeed.
    static std::unique_ptr<ECSqlExpectedResult> Create();
    //! Initializes a new instance of the ECSqlExpectedResult type indicating the test being expected to fail.
    static std::unique_ptr<ECSqlExpectedResult> CreateFailing(Category failingCategory, Utf8CP description = nullptr);

    Type GetType() const;
    Category GetCategory() const;
    Utf8CP GetDescription() const;
    bool IsExpectedToSucceed() const;

    static Utf8String CategoryToString(Category IECSqlBinder);
    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlExpectedResultsDictionary : NonCopyableClass
    {
private:
    std::map<ECSqlExpectedResult::Type, std::unique_ptr<ECSqlExpectedResult>> m_innerDictionary;
    ECSqlExpectedResult const* m_lastAdded;

public:
    ECSqlExpectedResultsDictionary () : m_lastAdded (nullptr) {}
    ~ECSqlExpectedResultsDictionary () {}
    //only movable, not copyable
    ECSqlExpectedResultsDictionary (ECSqlExpectedResultsDictionary&& rhs);
    ECSqlExpectedResultsDictionary& operator= (ECSqlExpectedResultsDictionary&& rhs);

    //! Adds an expected result item for a given result type into the dictionary
    //! @param[in] expectedResult Expected result object. The dictionary will take over ownership of the object.
    void Add (std::unique_ptr<ECSqlExpectedResult> expectedResult);

    template<class TECSqlExpectedResult>
    bool TryGet (TECSqlExpectedResult const*& expectedResult, ECSqlExpectedResult::Type type) const
        {
        expectedResult = nullptr;

        auto it = m_innerDictionary.find (type);
        if (it == m_innerDictionary.end ())
            {
            return false;
            }

        expectedResult = dynamic_cast<TECSqlExpectedResult const*> (it->second.get ());
        return expectedResult != nullptr;
        }

    ECSqlExpectedResult const* GetLastAdded () const;
    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlTestItem : NonCopyableClass
    {
public:
    //=======================================================================================    
    // @bsiclass                                                 Krischan.Eberle     04/2013
    //=======================================================================================    
    struct ParameterValue
        {
    private:
        enum class Kind
            {
            ECValue,
            DateTime,
            IGeometry,
            ECInstanceId
            };

        Utf8String m_parameterName;
        //Using ECValue only for the test framework so that we don't have to invent another variant like type.
        //ECValue is not used by ECDb.
        //As ECValue cannot store ECInstanceIds, local DateTimes or IGeometries (yet), those are held in the separate field. We need local DateTimes in the tests
        //to test that ECDb doesn't support them.
        Kind m_kind;
        ECN::ECValue m_value;
        DateTime m_dateTimeValue;
        IGeometryPtr m_geomValue;
        ECInstanceId m_ecinstanceId;

    public:
        //! @param[in] parameterName Name of the parameter if the parameter is a named one
        //! @param[in] parameterValue parameter value to be bound to the test statement.
        ParameterValue (Utf8CP parameterName, ECN::ECValueCR parameterValue)
            : m_parameterName (parameterName), m_kind (Kind::ECValue), m_value (parameterValue), m_geomValue (nullptr)
            {}

        //! @param[in] parameterName Name of the parameter if the parameter is a named one
        //! @param[in] dateTimeParameterValue date time parameter value to be bound to the test statement.
        ParameterValue (Utf8CP parameterName, DateTimeCR dateTimeParameterValue)
            : m_parameterName (parameterName), m_kind (Kind::DateTime), m_dateTimeValue (dateTimeParameterValue), m_geomValue (nullptr)
            {}

        //! @param[in] parameterName Name of the parameter if the parameter is a named one
        //! @param[in] geomParameterValue IGeometry parameter value to be bound to the test statement.
        ParameterValue (Utf8CP parameterName, IGeometryPtr geomParameterValue)
            : m_parameterName (parameterName), m_kind (Kind::IGeometry), m_geomValue (geomParameterValue)
            {}

        //! @param[in] parameterName Name of the parameter if the parameter is a named one
        //! @param[in] ecinstanceIdValue ECInstanceId parameter value to be bound to the test statement.
        ParameterValue (Utf8CP parameterName, ECInstanceId ecinstanceIdValue)
            : m_parameterName (parameterName), m_kind (Kind::ECInstanceId), m_ecinstanceId (ecinstanceIdValue), m_geomValue (nullptr)
            {}

        explicit ParameterValue (ECN::ECValueCR parameterValue)
            : m_kind (Kind::ECValue), m_value (parameterValue), m_geomValue (nullptr)
            {}

        explicit ParameterValue (DateTimeCR parameterValue)
            : m_kind (Kind::DateTime), m_dateTimeValue (parameterValue), m_geomValue (nullptr)
            {}

        explicit ParameterValue (IGeometryPtr geomParameterValue)
            : m_kind (Kind::IGeometry), m_geomValue (geomParameterValue)
            {}

        explicit ParameterValue (ECInstanceId parameterValue)
            : m_kind (Kind::ECInstanceId), m_ecinstanceId (parameterValue), m_geomValue (nullptr)
            {}

        ParameterValue (ParameterValue&& rhs)
            : m_parameterName (std::move (rhs.m_parameterName)), m_kind (std::move (rhs.m_kind)), 
            m_value (std::move (rhs.m_value)), m_dateTimeValue (std::move (rhs.m_dateTimeValue)),
            m_geomValue (std::move (rhs.m_geomValue)),
            m_ecinstanceId (std::move (rhs.m_ecinstanceId))
            {}

        ParameterValue& operator= (ParameterValue&& rhs)
            {
            if (this != &rhs)
                {
                m_parameterName = std::move (rhs.m_parameterName);
                m_kind = std::move (rhs.m_kind);
                m_value = std::move (rhs.m_value);
                m_dateTimeValue = std::move (rhs.m_dateTimeValue);
                m_geomValue = std::move (rhs.m_geomValue);
                m_ecinstanceId = std::move (rhs.m_ecinstanceId);
                }

            return *this;
            }

        bool IsNamedParameter () const {return !m_parameterName.empty ();}
        Utf8CP GetName () const {return m_parameterName.c_str ();}

        bool IsDateTime () const 
            {
            return m_kind == Kind::DateTime || (m_kind == Kind::ECValue && m_value.IsDateTime ()); 
            }

        bool IsIGeometry () const
            {
            return m_kind == Kind::IGeometry;
            }

        bool IsECInstanceId () const
            {
            return m_kind == Kind::ECInstanceId;
            }

        ECN::ECValueCR GetValue () const {return m_value; }
        DateTime GetDateTime () const
            {
            BeAssert (IsDateTime ());
            if (m_kind == Kind::ECValue)
                return m_value.GetDateTime ();
            else
                return m_dateTimeValue; 
            }

        IGeometryCP GetIGeometry () const
            {
            BeAssert (IsIGeometry ());
            return m_geomValue.get ();
            }

        ECInstanceId GetECInstanceId () const
            {
            BeAssert (IsECInstanceId ());
            return m_ecinstanceId;
            }
        };

private:
    Utf8String m_ecsql;
    std::vector<ParameterValue> m_parameterValueList;
    ECSqlExpectedResultsDictionary m_expectedResults;
    bool m_rollbackAfterwards;

public:
    explicit ECSqlTestItem (Utf8CP ecsql);
    ECSqlTestItem (Utf8CP ecsql, bool rollbackAfterwards);
    ~ECSqlTestItem () {}
    // only movable, not copyable
    ECSqlTestItem (ECSqlTestItem&& rhs);
    ECSqlTestItem& operator= (ECSqlTestItem&& rhs);

    void AddParameterValue (ParameterValue&& parameterValue);

    void AddExpectedResult (std::unique_ptr<ECSqlExpectedResult> expectedResult);

    Utf8StringCR GetECSql () const;
    bool HasECSqlBuilder () const;
    std::vector<ParameterValue> const& GetParameterValues () const;
    ECSqlExpectedResultsDictionary const& GetExpectedResults () const;
    bool GetRollbackAfterwards () const;

    ECSqlExpectedResult::Category GetExpectedResultCategory () const;
    Utf8CP GetExpectedResultDescription () const;
    };

//=======================================================================================    
// @bsiclass                                                 Krischan.Eberle     04/2013
//=======================================================================================    
struct ECSqlTestDataset : NonCopyableClass
    {
private:
    std::vector<ECSqlTestItem> m_innerList;

public:
    ECSqlTestDataset () {}
    ~ECSqlTestDataset () {}
    // only movable, not copyable
    ECSqlTestDataset (ECSqlTestDataset&& rhs);
    ECSqlTestDataset& operator= (ECSqlTestDataset&& rhs);

    ECSqlTestItem& AddTestItem (ECSqlTestItem& testItem);

    std::vector<ECSqlTestItem> const& GetTestItems () const;
    };

END_ECDBUNITTESTS_NAMESPACE