/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSqlBuilder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************** ECSqlBuilder ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8CP const ECSqlBuilder::ECINSTANCEID_SYSTEMPROPERTY = "ECInstanceId";

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilder::ECSqlBuilder (StatementType type)
    : m_statementType (type)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilder::ECSqlBuilder (ECSqlBuilderCR rhs)
    : m_statementType (rhs.m_statementType)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderR ECSqlBuilder::operator= (ECSqlBuilderCR rhs)
    {
    if (this != &rhs)
        {
        m_statementType = rhs.m_statementType;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilder::ECSqlBuilder (ECSqlBuilder&& rhs)
    : m_statementType (move (rhs.m_statementType))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderR ECSqlBuilder::operator= (ECSqlBuilder&& rhs)
    {
    if (this != &rhs)
        {
        m_statementType = move (rhs.m_statementType);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlBuilder::operator== (ECSqlBuilderCR rhs) const
    {
    if (this == &rhs)
        {
        return true;
        }

    return _Equals (rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlBuilder::operator!= (ECSqlBuilderCR rhs) const
    {
    return !(*this == rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderCP ECSqlBuilder::Clone () const
    {
    return _Clone ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilder::StatementType ECSqlBuilder::GetType () const
    {
    return m_statementType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlBuilder::ToString () const
    {
    return _ToString ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
Utf8String ECSqlBuilder::ToECSqlSnippet (ECN::ECClassCR ecClass)
    {
    return ecClass.GetECSqlName();
    }


//****************** ECSqlSelectBuilder ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilder::ECSqlSelectBuilder ()
: ECSqlBuilder (StatementType::Select), m_orderBy ("")
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilder::ECSqlSelectBuilder (ECSqlSelectBuilderCR rhs)
    : ECSqlBuilder (rhs), m_select (rhs.m_select), m_from (rhs.m_from),
    m_join (rhs.m_join), m_using (rhs.m_using), m_where (rhs.m_where), m_orderBy (rhs.m_orderBy),
    m_limit (rhs.m_limit)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilder::~ECSqlSelectBuilder()
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::operator= (ECSqlSelectBuilderCR rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (rhs);

        m_select = rhs.m_select;
        m_from = rhs.m_from;
        m_join = rhs.m_join;
        m_using = rhs.m_using;
        m_where = rhs.m_where;
        m_orderBy = rhs.m_orderBy;
        m_limit = rhs.m_limit;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilder::ECSqlSelectBuilder (ECSqlSelectBuilder&& rhs)
    : ECSqlBuilder (move (rhs)), m_select (move (rhs.m_select)), m_from (move (rhs.m_from)),
    m_join (move (rhs.m_join)), m_using (move (rhs.m_using)), 
    m_where (move (rhs.m_where)), m_orderBy (move (rhs.m_orderBy)),
        m_limit (move (rhs.m_limit))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::operator= (ECSqlSelectBuilder&& rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (move (rhs));

        m_select = move (rhs.m_select);
        m_from = move (rhs.m_from);
        m_join = move (rhs.m_join);
        m_using = move (rhs.m_using);
        m_where = move (rhs.m_where);
        m_orderBy = move (rhs.m_orderBy);
        m_limit = move (rhs.m_limit);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Select (Utf8CP selectClause)
    {
    PRECONDITION (m_select.IsNull () && "Multiple select clauses not supported.", *this);
    m_select = SelectClause (selectClause);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::SelectAll()
    {
    PRECONDITION (m_select.IsNull () && "Multiple select clauses not supported.", *this);
    m_select = SelectClause (true);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::From (ECClassCR fromClass, Utf8CP fromAlias, bool isPolymorphic)
    {
    m_from = ClassClause (fromClass, fromAlias, isPolymorphic);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::From (ECClassCR fromClass, bool isPolymorphic)
    {
    return From (fromClass, nullptr, isPolymorphic);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Join (ECClassCR joinClass, Utf8CP joinClassAlias, bool isPolymorphic)
    {
    m_join = ClassClause (joinClass, joinClassAlias, isPolymorphic);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Join (ECClassCR joinClass, bool isPolymorphic)
    {
    return Join (joinClass, nullptr, isPolymorphic);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Using 
(
ECRelationshipClassCR relationshipClass,
Utf8CP relationshipClassAlias,
JoinDirection joinDirection
)
    {
    m_using = JoinUsingClause (relationshipClass, relationshipClassAlias, joinDirection);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Using
(
ECRelationshipClassCR relationshipClass,
JoinDirection joinDirection
)
    {
    return Using (relationshipClass, nullptr, joinDirection);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Where (Utf8CP ecWhereClause)
    {
    m_where = WhereClause (ecWhereClause);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::WhereSourceEndIs (Utf8CP sourceClass, Utf8CP sourceECInstanceId)
    {
    m_where = WhereClause (sourceClass, sourceECInstanceId, nullptr, nullptr);
    return *this;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::WhereTargetEndIs (Utf8CP targetClass, Utf8CP targetECInstanceId)
    {
    m_where = WhereClause (nullptr, nullptr, targetClass, targetECInstanceId);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::WhereRelationshipEndsAre (Utf8CP sourceClass, Utf8CP sourceECInstanceId, Utf8CP targetClass, Utf8CP targetECInstanceId)
    {
    m_where = WhereClause (sourceClass, sourceECInstanceId, targetClass, targetECInstanceId);
    return *this;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::OrderBy (Utf8CP orderByClause)
    {
    m_orderBy = orderByClause;
    return *this;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 08/2012
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSelectBuilderR ECSqlSelectBuilder::Limit (Utf8CP limitClause, Utf8CP offsetClause)
    {
    m_limit = LimitClause (limitClause, offsetClause);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlSelectBuilder::_Equals (ECSqlBuilderCR rhsBase) const
    {
    //check for pointer equality already done in base class
    if (GetType () != rhsBase.GetType ())
        {
        return false;
        }

    ECSqlSelectBuilder const& rhs = static_cast<ECSqlSelectBuilder const&> (rhsBase);
    return m_select == rhs.m_select &&
            m_from == rhs.m_from &&
            m_join == rhs.m_join &&
            m_using == rhs.m_using &&
            m_where == rhs.m_where &&
            m_orderBy == rhs.m_orderBy &&
            m_limit == rhs.m_limit;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderCP ECSqlSelectBuilder::_Clone () const
    {
    return new ECSqlSelectBuilder (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlSelectBuilder::_ToString () const
    {
    Utf8String ecSql = m_select.ToString ();

    if (!m_from.IsNull ())
        {
        ecSql.append (" FROM ");
        ecSql.append (m_from.ToString ());
        }

    if (!m_join.IsNull ())
        {
        ecSql.append (" JOIN ");
        ecSql.append (m_join.ToString ());
        }

    if (!m_using.IsNull ())
        {
        ecSql.append (" ");
        ecSql.append (m_using.ToString ());
        }

    if (!m_where.IsNull ())
        {
        ecSql.append (" ");
        ecSql.append (m_where.ToString ());
        }

    if (!m_orderBy.empty ())
        {
        ecSql.append (" ORDER BY ");
        ecSql.append (m_orderBy);
        }
        
    if (!m_limit.IsNull ())
        {
        ecSql.append (" ");
        ecSql.append (m_limit.ToString ());
        }

    return ecSql;
    }

//****************** ECSqlInsertBuilder ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilder::ECSqlInsertBuilder ()
: ECSqlBuilder (StatementType::Insert)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilder::ECSqlInsertBuilder (ECSqlInsertBuilderCR rhs)
    : ECSqlBuilder (rhs), m_targetClass (rhs.m_targetClass), 
      m_targetPropertiesClause (rhs.m_targetPropertiesClause), m_valuesClause (rhs.m_valuesClause)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilderR ECSqlInsertBuilder::operator= (ECSqlInsertBuilderCR rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (rhs);

        m_targetClass = rhs.m_targetClass;
        m_targetPropertiesClause = rhs.m_targetPropertiesClause;
        m_valuesClause = rhs.m_valuesClause;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilder::ECSqlInsertBuilder (ECSqlInsertBuilder&& rhs)
    : ECSqlBuilder (move (rhs)), m_targetClass (move (rhs.m_targetClass)), 
    m_targetPropertiesClause (move (rhs.m_targetPropertiesClause)), m_valuesClause (move (rhs.m_valuesClause))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilderR ECSqlInsertBuilder::operator= (ECSqlInsertBuilder&& rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (move (rhs));

        m_targetClass = move (rhs.m_targetClass);
        m_targetPropertiesClause = move (rhs.m_targetPropertiesClause);
        m_valuesClause = move (rhs.m_valuesClause);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilder::~ECSqlInsertBuilder ()
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlInsertBuilder::_Equals (ECSqlBuilderCR rhsBase) const
    {
    //check for pointer equality already done in base class
    if (GetType () != rhsBase.GetType ())
        {
        return false;
        }

    ECSqlInsertBuilder const& rhs = static_cast<ECSqlInsertBuilder const&> (rhsBase);
    return m_targetClass == rhs.m_targetClass && 
            m_targetPropertiesClause == rhs.m_targetPropertiesClause && 
            m_valuesClause == rhs.m_valuesClause;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderCP ECSqlInsertBuilder::_Clone () const
    {
    return new ECSqlInsertBuilder (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilderR ECSqlInsertBuilder::InsertInto (ECClassCR targetClass)
    {
    //Aliases and polymorphism not supported for insert.
    m_targetClass = ClassClause (targetClass, nullptr, false);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlInsertBuilderR ECSqlInsertBuilder::AddValue (Utf8CP targetProperty, Utf8CP targetValue)
    {
    if (Utf8String::IsNullOrEmpty (targetProperty))
        {
        BeAssert (false && "ECSqlInsertBuilder::Value> Parameter targetProperty must not be null or empty string.");
        return *this;
        }

    if (targetValue == nullptr)
        {
        BeAssert (false && "ECSqlInsertBuilder::Value> Parameter targetValue must not be null.");
        return *this;
        }

    m_targetPropertiesClause.push_back (Utf8String (targetProperty));
    m_valuesClause.push_back (Utf8String (targetValue));

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause const& ECSqlInsertBuilder::GetTargetClass () const
    {
    return m_targetClass;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bvector<Utf8String> const& ECSqlInsertBuilder::GetTargetPropertiesClause () const
    {
    return m_targetPropertiesClause;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bvector<Utf8String> const& ECSqlInsertBuilder::GetValuesClause () const
    {
    return m_valuesClause;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlInsertBuilder::TryFindECInstanceIdProperty (int& propertyIndex) const
    {
    Utf8String ecInstanceIdPropName (ECINSTANCEID_SYSTEMPROPERTY);
    bvector<Utf8String>::const_iterator begin = m_targetPropertiesClause.begin ();
    bvector<Utf8String>::const_iterator end = m_targetPropertiesClause.end ();
    bvector<Utf8String>::const_iterator it = std::find (begin, end, ecInstanceIdPropName);
    if (it == end)
        {
        propertyIndex = -1;
        return false;
        }
    else
        {
        propertyIndex = static_cast <int> (it - begin);
        return true;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlInsertBuilder::_ToString () const
    {
    Utf8String ecsql;
    if (!m_targetClass.IsNull ())
        {
        ecsql.append ("INSERT INTO ");
        //ignore the IsPolymorphic flag as Inserts don't support the concept of polymorphic inserts
        ecsql.append (m_targetClass.ToString (true));
        }

    const size_t targetPropertyCount = m_targetPropertiesClause.size ();
    if (targetPropertyCount != 0)
        {
        ecsql.append (" (");
        const size_t lastIndex = targetPropertyCount - 1;
        size_t i = 0;
        for (Utf8StringCR targetProperty : m_targetPropertiesClause)
            {
            ecsql.append (targetProperty);

            if (i != lastIndex)
                {
                ecsql.append (", ");
                }

            i++;
            }

        ecsql.append (")");
        }
    
    const size_t valueCount = m_valuesClause.size ();
    if (valueCount != 0)
        {
        ecsql.append (" VALUES (");
        const size_t lastIndex = valueCount - 1;
        size_t i = 0;
        for (Utf8StringCR value : m_valuesClause)
            {
            ecsql.append (value);

            if (i != lastIndex)
                {
                ecsql.append (", ");
                }

            i++;
            }

        ecsql.append (")");
        }

    return ecsql;
    }

//****************** ECSqlUpdateBuilder ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilder::ECSqlUpdateBuilder ()
: ECSqlBuilder (StatementType::Update)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilder::ECSqlUpdateBuilder (ECSqlUpdateBuilderCR rhs)
    : ECSqlBuilder (rhs), m_targetClass (rhs.m_targetClass), m_setClause (rhs.m_setClause), m_where (rhs.m_where)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilderR ECSqlUpdateBuilder::operator= (ECSqlUpdateBuilderCR rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (rhs);

        m_targetClass = rhs.m_targetClass;
        m_setClause = rhs.m_setClause;
        m_where = rhs.m_where;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilder::ECSqlUpdateBuilder (ECSqlUpdateBuilder&& rhs)
    : ECSqlBuilder (move (rhs)), m_targetClass (move (rhs.m_targetClass)), m_setClause (move (rhs.m_setClause)), m_where (move (rhs.m_where))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilderR ECSqlUpdateBuilder::operator= (ECSqlUpdateBuilder&& rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (move (rhs));

        m_targetClass = move (rhs.m_targetClass);
        m_setClause = move (rhs.m_setClause);
        m_where = move (rhs.m_where);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilder::~ECSqlUpdateBuilder ()
    {
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderCP ECSqlUpdateBuilder::_Clone () const
    {
    return new ECSqlUpdateBuilder (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilderR ECSqlUpdateBuilder::Update (ECClassCR targetClass, bool isPolymorphic)
    {
    //SQL spec doesn't allow aliases in update statements
    m_targetClass = ClassClause (targetClass, nullptr, isPolymorphic);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilderR ECSqlUpdateBuilder::AddSet (Utf8CP propertyName, Utf8CP rhsExpression)
    {
    PRECONDITION (!Utf8String::IsNullOrEmpty (propertyName), *this);
    PRECONDITION (!Utf8String::IsNullOrEmpty (rhsExpression), *this);

    SetClauseItem setClauseItem (propertyName, rhsExpression);
    m_setClause.push_back (setClauseItem);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilderR ECSqlUpdateBuilder::Where (Utf8CP whereClause)
    {
    m_where = WhereClause (whereClause);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause const& ECSqlUpdateBuilder::GetTargetClass () const
    {
    return m_targetClass;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlUpdateBuilder::SetClause const& ECSqlUpdateBuilder::GetSetClause () const
    {
    return m_setClause;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause const& ECSqlUpdateBuilder::GetWhereClause () const
    {
    return m_where;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlUpdateBuilder::_Equals (ECSqlBuilderCR rhsBase) const
    {
    //check for pointer equality already done in base class
    if (GetType () != rhsBase.GetType ())
        {
        return false;
        }

    ECSqlUpdateBuilder const& rhs = static_cast<ECSqlUpdateBuilder const&> (rhsBase);
    return m_targetClass == rhs.m_targetClass && m_setClause == rhs.m_setClause && m_where == rhs.m_where;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlUpdateBuilder::_ToString () const
    {
    Utf8String ecsql;
    if (!m_targetClass.IsNull ())
        {
        ecsql.append ("UPDATE ");
        ecsql.append (m_targetClass.ToString ());
        }

    const size_t setClauseItemCount = m_setClause.size ();

    if (setClauseItemCount > 0)
        {
        ecsql.append (" SET ");
        const size_t lastSetClauseItemIndex = setClauseItemCount - 1;
        for (size_t i = 0; i < setClauseItemCount; i++)
            {
            SetClauseItem setClauseItem = m_setClause[i];
            Utf8String setClauseItemStr;
            setClauseItemStr.Sprintf ("%s = %s", setClauseItem.first.c_str (), setClauseItem.second.c_str ());
            ecsql.append (setClauseItemStr.c_str ());

            if (i < lastSetClauseItemIndex)
                {
                ecsql.append (", ");
                }
            }
        }

    if (!m_where.IsNull ())
        {
        ecsql.append (" ");
        ecsql.append (m_where.ToString ());
        }

    return ecsql;
    }


//****************** ECSqlDeleteBuilder ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilder::ECSqlDeleteBuilder ()
: ECSqlBuilder (StatementType::Delete)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilder::ECSqlDeleteBuilder (ECSqlDeleteBuilderCR rhs)
    : ECSqlBuilder (rhs), m_targetClass (rhs.m_targetClass), m_where (rhs.m_where)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilderR ECSqlDeleteBuilder::operator= (ECSqlDeleteBuilderCR rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (rhs);

        m_targetClass = rhs.m_targetClass;
        m_where = rhs.m_where;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilder::ECSqlDeleteBuilder (ECSqlDeleteBuilder&& rhs)
    : ECSqlBuilder (move (rhs)), m_targetClass (move (rhs.m_targetClass)), m_where (move (rhs.m_where))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilderR ECSqlDeleteBuilder::operator= (ECSqlDeleteBuilder&& rhs)
    {
    if (this != &rhs)
        {
        //calls operator in base class
        ECSqlBuilder::operator= (move (rhs));

        m_targetClass = move (rhs.m_targetClass);
        m_where = move (rhs.m_where);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilder::~ECSqlDeleteBuilder ()
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlBuilderCP ECSqlDeleteBuilder::_Clone () const
    {
    return new ECSqlDeleteBuilder (*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilderR ECSqlDeleteBuilder::DeleteFrom (ECN::ECClassCR targetClass, bool isPolymorphic)
    {
    //SQL spec doesn't allow aliases in delete statements
    m_targetClass = ClassClause (targetClass, nullptr, isPolymorphic);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlDeleteBuilderR ECSqlDeleteBuilder::Where (Utf8CP whereClause)
    {
    m_where = WhereClause (whereClause);
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause const& ECSqlDeleteBuilder::GetTargetClass () const
    {
    return m_targetClass;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause const& ECSqlDeleteBuilder::GetWhereClause () const
    {
    return m_where;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlDeleteBuilder::_Equals (ECSqlBuilderCR rhsBase) const
    {
    //check for pointer equality already done in base class
    if (GetType () != rhsBase.GetType ())
        {
        return false;
        }

    ECSqlDeleteBuilder const& rhs = static_cast<ECSqlDeleteBuilder const&> (rhsBase);

    return m_targetClass == rhs.m_targetClass && m_where == rhs.m_where;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlDeleteBuilder::_ToString () const
    {
    Utf8String ecsql;
    
    if (!m_targetClass.IsNull ())
        {
        ecsql.append ("DELETE FROM ");
        ecsql.append (m_targetClass.ToString ());
        }

    if (!m_where.IsNull ())
        {
        ecsql.append (" ");
        ecsql.append (m_where.ToString ());
        }

    return ecsql;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
