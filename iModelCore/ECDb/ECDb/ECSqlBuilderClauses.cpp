/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSqlBuilderClauses.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** ClassClause ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause::ClassClause ()
    : m_class (nullptr), m_alias (""), m_isPolymorphic (true)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause::ClassClause (ECClassCR ecClass, Utf8CP alias, bool isPolymorphic)
    : m_class (&ecClass), m_alias (alias != nullptr ? alias : ""), m_isPolymorphic (isPolymorphic)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause::ClassClause (ClassClause const& rhs)
    : m_class (rhs.m_class), m_alias (rhs.m_alias), m_isPolymorphic (rhs.m_isPolymorphic)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause& ClassClause::operator= (ClassClause const& rhs)
    {
    if (this != &rhs)
        {
        m_class = rhs.m_class;
        m_alias = rhs.m_alias;
        m_isPolymorphic = rhs.m_isPolymorphic;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause::ClassClause (ClassClause&& rhs)
    : m_class (move (rhs.m_class)), m_alias (move (rhs.m_alias)), m_isPolymorphic (move (rhs.m_isPolymorphic))
    {
    rhs.m_class = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ClassClause& ClassClause::operator= (ClassClause&& rhs)
    {
    if (this != &rhs)
        {
        m_class = move (rhs.m_class);
        m_alias = move (rhs.m_alias);
        m_isPolymorphic = move (rhs.m_isPolymorphic);

        rhs.m_class = nullptr;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassClause::operator== (ClassClause const& rhs) const
    {
    //if same objects, return without comparing fields
    if (this == &rhs)
        {
        return true;
        }

    // if both are null, don't check any other fields
    return (IsNull () && rhs.IsNull ()) ||
            //compare classes by value (with ECSchemaComparer.h) and not by reference
            //(IsNull check ensures that m_class is not nullptr)
            (*m_class == *rhs.m_class &&
            m_alias == rhs.m_alias &&
            m_isPolymorphic == rhs.m_isPolymorphic);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassClause::operator!= (ClassClause const& rhs) const
    {
    return !(*this == rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ClassClause::IsNull () const
    {
    return m_class == nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ClassClause::GetAlias () const 
    {
    return m_alias;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassClause::ToString () const
    {
    return ToString (false);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassClause::ToString (bool ignoreIsPolymorphic) const
    {
    if (IsNull ())
        {
        return "";
        }

    Utf8String ecsql;
    if (!ignoreIsPolymorphic && !m_isPolymorphic)
        {
        //ONLY is the ECSQL (and SQL-99) keyword to indicate non-polymorphic target classes
        ecsql.append ("ONLY ");
        }

    //add fully qualified class name, i.e. <schema namespace prefix>.<class name> 
    ecsql.append (GetClass().GetECSqlName());

    if (!m_alias.empty ())
        {
        ecsql.append (" ");
        ecsql.append (m_alias);
        }

    return ecsql;
    }


//****************** SelectClause ****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::SelectClause ()
    : m_isNull (true), m_selectAll (false), m_selectDistinct (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::SelectClause (bool selectAll)
    : m_isNull (false), m_selectAll (selectAll), m_selectDistinct (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::SelectClause (Utf8CP selectClause)
    : m_isNull (false), m_selectAll (false), m_selectDistinct (false), m_clause (selectClause)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::SelectClause (SelectClause const& rhs)
    : m_isNull (rhs.m_isNull), m_selectAll (rhs.m_selectAll), m_selectDistinct (rhs.m_selectDistinct), m_clause (rhs.m_clause)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::~SelectClause ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause& SelectClause::operator= (SelectClause const& rhs)
    {
    if (this != &rhs)
        {
        m_isNull = rhs.m_isNull;
        m_selectAll = rhs.m_selectAll;
        m_selectDistinct = rhs.m_selectDistinct;
        m_clause = rhs.m_clause;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause::SelectClause (SelectClause&& rhs)
    : m_isNull (move (rhs.m_isNull)), m_selectAll (move (rhs.m_selectAll)), m_selectDistinct (move (rhs.m_selectDistinct)), m_clause (move (rhs.m_clause))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
SelectClause& SelectClause::operator= (SelectClause&& rhs)
    {
    if (this != &rhs)
        {
        m_isNull = move (rhs.m_isNull);
        m_selectAll = move (rhs.m_selectAll);
        m_selectDistinct = move (rhs.m_selectDistinct);
        m_clause = move (rhs.m_clause);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool SelectClause::operator== (SelectClause const& rhs) const
    {
    //if same objects, return without comparing fields
    if (this == &rhs)
        {
        return true;
        }
         
    // if both are null, don't check any other fields
    return (m_isNull && rhs.m_isNull) ||
            (m_isNull == rhs.m_isNull && 
            m_selectAll == rhs.m_selectAll && 
            m_selectDistinct == rhs.m_selectDistinct &&
            m_clause == rhs.m_clause);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool SelectClause::operator!= (SelectClause const& rhs) const
    {
    return ! (*this == rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool SelectClause::IsNull () const
    {
    return m_isNull;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool SelectClause::IsSelectAll () const
    {
    return m_selectAll;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR SelectClause::GetClause () const
    {
    return m_clause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SelectClause::ToString () const
    {
    if (IsNull ())
        {
        return "";
        }

    if (IsSelectAll ())
        {
        return "SELECT *";
        }

    Utf8String ecsql ("SELECT ");
    ecsql.append (m_clause);
    return ecsql;
    }


//****************** JoinUsingClause ****************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause::JoinUsingClause ()
    : m_joinDirection (JoinDirection::Implied)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause::JoinUsingClause
(
ECN::ECRelationshipClassCR relationshipClass,
Utf8CP alias,
JoinDirection joinDirection
) : m_relationshipClassClause (relationshipClass, alias, false), m_joinDirection (joinDirection)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause::JoinUsingClause (JoinUsingClause const& rhs)
: m_relationshipClassClause (rhs.m_relationshipClassClause), m_joinDirection (rhs.m_joinDirection)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause::~JoinUsingClause ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause& JoinUsingClause::operator= (JoinUsingClause const& rhs)
    {
    if (this != &rhs)
        {
        m_relationshipClassClause = rhs.m_relationshipClassClause;
        m_joinDirection = rhs.m_joinDirection;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause::JoinUsingClause (JoinUsingClause&& rhs)
    : m_relationshipClassClause (move (rhs.m_relationshipClassClause)), m_joinDirection (move (rhs.m_joinDirection))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
JoinUsingClause& JoinUsingClause::operator= (JoinUsingClause&& rhs)
    {
    if (this != &rhs)
        {
        m_relationshipClassClause = move (rhs.m_relationshipClassClause);
        m_joinDirection = move (rhs.m_joinDirection);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool JoinUsingClause::operator== (JoinUsingClause const& rhs) const
    {
    //if same objects, return without comparing fields
    if (this == &rhs)
        return true;

    // if both are null, don't check any other fields
    return (IsNull () && rhs.IsNull ()) ||
           (m_relationshipClassClause == rhs.m_relationshipClassClause &&
           m_joinDirection == rhs.m_joinDirection);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool JoinUsingClause::operator!= (JoinUsingClause const& rhs) const
    {
    return !(*this == rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
bool JoinUsingClause::IsNull () const
    {
    return m_relationshipClassClause.IsNull ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2014
//+---------------+---------------+---------------+---------------+---------------+------
ECRelationshipClassCR JoinUsingClause::GetRelationshipClass () const
    {
    return *m_relationshipClassClause.GetClass ().GetRelationshipClassCP ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
JoinDirection JoinUsingClause::GetJoinDirection () const
    {
    return m_joinDirection;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String JoinUsingClause::ToString() const
    {
    if (IsNull ())
        {
        return "";
        }

    Utf8String joinUsing ("USING ");
    joinUsing.append (m_relationshipClassClause.ToString (true).c_str ());

    switch (m_joinDirection)
        {
        case JoinDirection::Forward:
            {
            joinUsing.append (" FORWARD");
            break;
            }

        case JoinDirection::Reverse:
            {
            joinUsing.append (" REVERSE");
            break;
            }

        default:    
        case JoinDirection::Implied:
            break;
        }

    return joinUsing;
    }
    

//****************** WhereClause ****************************************************

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::WhereClause() : m_type (WHERE_TYPE_None)
    {
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::WhereClause (Utf8CP expression) : m_type (WHERE_TYPE_Expression), m_expression (expression)
    {
    if (Utf8String::IsNullOrEmpty (expression))
        m_type = WHERE_TYPE_None;
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::WhereClause (Utf8CP sourceClassName, Utf8CP sourceECInstanceIdExpression, Utf8CP targetClassName, Utf8CP targetECInstanceIdExpression)
: m_type (WHERE_TYPE_Relationship), m_sourceClassName (sourceClassName), m_sourceECInstanceIdExpression (sourceECInstanceIdExpression),
m_targetClassName (targetClassName), m_targetECInstanceIdExpression (targetECInstanceIdExpression)
    {
    if (Utf8String::IsNullOrEmpty (sourceClassName) && Utf8String::IsNullOrEmpty (sourceECInstanceIdExpression) &&
        Utf8String::IsNullOrEmpty (targetClassName) && Utf8String::IsNullOrEmpty (targetECInstanceIdExpression))
        m_type = WHERE_TYPE_None;

    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::WhereClause (WhereClause const& other)
: m_type (other.m_type), m_expression (other.m_expression),
    m_sourceClassName (other.m_sourceClassName), m_sourceECInstanceIdExpression (other.m_sourceECInstanceIdExpression), 
    m_targetClassName (other.m_targetClassName), m_targetECInstanceIdExpression (other.m_targetECInstanceIdExpression)
    {
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::~WhereClause ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause& WhereClause::operator= (WhereClause const& rhs)
    {
    if (this != &rhs)
        {
        m_type = rhs.m_type;
        m_expression = rhs.m_expression;
        m_sourceClassName = rhs.m_sourceClassName;
        m_sourceECInstanceIdExpression = rhs.m_sourceECInstanceIdExpression;
        m_targetClassName = rhs.m_targetClassName;
        m_targetECInstanceIdExpression  = rhs.m_targetECInstanceIdExpression;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::WhereClause (WhereClause&& rhs)
    : m_type (move (rhs.m_type)), m_expression (move (rhs.m_expression)),
    m_sourceClassName (move (rhs.m_sourceClassName)), m_sourceECInstanceIdExpression (move (rhs.m_sourceECInstanceIdExpression)), 
    m_targetClassName (move (rhs.m_targetClassName)), m_targetECInstanceIdExpression (move (rhs.m_targetECInstanceIdExpression))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause& WhereClause::operator= (WhereClause&& rhs)
    {
    if (this != &rhs)
        {
        m_type = move (rhs.m_type);
        m_expression = move (rhs.m_expression);
        m_sourceClassName = move (rhs.m_sourceClassName);
        m_sourceECInstanceIdExpression = move (rhs.m_sourceECInstanceIdExpression);
        m_targetClassName = move (rhs.m_targetClassName);
        m_targetECInstanceIdExpression  = move (rhs.m_targetECInstanceIdExpression);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool WhereClause::operator== (WhereClause const& rhs) const
    {
    //if same objects or both are null, return without comparing fields
    if (this == &rhs || (IsNull () && rhs.IsNull ()))
        {
        return true;
        }

    //IsNull checks for that already
    BeAssert (m_type != WHERE_TYPE_None);

    if (m_type != rhs.m_type)
        {
        return false;
        }

    if (m_type == WHERE_TYPE_Expression)
        {
        return m_expression == rhs.m_expression;
        }

    BeAssert (m_type == WHERE_TYPE_Relationship);
    return (m_sourceClassName == rhs.m_sourceClassName && m_sourceECInstanceIdExpression == rhs.m_sourceECInstanceIdExpression &&
            m_targetClassName == rhs.m_targetClassName && m_targetECInstanceIdExpression == rhs.m_targetECInstanceIdExpression);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool WhereClause::operator!= (WhereClause const& rhs) const
    {
    return !(*this == rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool WhereClause::IsNull () const
    {
    return m_type == WHERE_TYPE_None;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
WhereClause::Type WhereClause::GetType () const
    {
    return m_type;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR WhereClause::GetExpression () const
    {
    return m_expression;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 02/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool WhereClause::GetRelationshipEnd (Utf8CP& endClass, Utf8CP& endInstanceId, ECRelationshipEnd relationshipEnd) const
    {
    if (m_type != WhereClause::WHERE_TYPE_Relationship)
        return false;

    if (relationshipEnd == ECRelationshipEnd_Source)
        {
        endClass = (m_sourceClassName[0] == 0) ? nullptr : m_sourceClassName.c_str();
        endInstanceId = (m_sourceECInstanceIdExpression[0] == 0) ? nullptr : m_sourceECInstanceIdExpression.c_str();
        }
    else /* if (relationshipEnd == ECRelationshipEnd_Target) */
        {
        endClass = (m_targetClassName[0] == 0) ? nullptr : m_targetClassName.c_str();
        endInstanceId = (m_targetECInstanceIdExpression[0] == 0) ? nullptr : m_targetECInstanceIdExpression.c_str();
        }

    return true;
    }



//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WhereClause::ToString () const
    {
    if (IsNull ())
        {
        return "";
        }

    Utf8String whereStr = "WHERE ";
    switch (m_type)
        {
        case WHERE_TYPE_Expression:
            whereStr.append (m_expression);
            break;
        case WHERE_TYPE_Relationship:
            {
            bool requiresAnd = false;
            if (!m_sourceECInstanceIdExpression.empty ())
                {
                whereStr.append ("SourceECInstanceId = ");
                whereStr.append (m_sourceECInstanceIdExpression);
                requiresAnd = true;
                }
            if (!m_targetECInstanceIdExpression.empty ())
                {
                if (requiresAnd)
                    {
                    whereStr.append (" AND ");
                    }

                whereStr.append ("TargetECInstanceId = ");
                whereStr.append (m_targetECInstanceIdExpression);
                }
            break;
            }
        default:
            BeAssert (false);
        };

    return whereStr;
    }

//****************** LimitClause ****************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::LimitClause ()
    : m_isNull (true)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::LimitClause (Utf8CP limitClause)
    : m_isNull (false), m_limitClause (limitClause), m_offsetClause ("")
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::LimitClause (Utf8CP limitClause, Utf8CP offsetClause)
    : m_isNull (false), m_limitClause (limitClause), m_offsetClause (offsetClause != nullptr ? offsetClause : "")
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::LimitClause (LimitClause const& rhs)
    : m_isNull (rhs.m_isNull), m_limitClause (rhs.m_limitClause), m_offsetClause (rhs.m_offsetClause)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::~LimitClause ()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause& LimitClause::operator= (LimitClause const& rhs)
    {
    if (this != &rhs)
        {
        m_isNull = rhs.m_isNull;
        m_limitClause = rhs.m_limitClause;
        m_offsetClause = rhs.m_offsetClause;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause::LimitClause (LimitClause&& rhs)
    : m_isNull (move (rhs.m_isNull)), m_limitClause (move (rhs.m_limitClause)), m_offsetClause (move (rhs.m_offsetClause))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 04/2013
//+---------------+---------------+---------------+---------------+---------------+------
LimitClause& LimitClause::operator= (LimitClause&& rhs)
    {
    if (this != &rhs)
        {
        m_isNull = move (rhs.m_isNull);
        m_limitClause = move (rhs.m_limitClause);
        m_offsetClause = move (rhs.m_offsetClause);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitClause::operator== (LimitClause const& rhs) const
    {
    //if same objects, return without comparing fields
    if (this == &rhs || (m_isNull && rhs.m_isNull))
        {
        return true;
        }

    return m_isNull == rhs.m_isNull && 
            m_limitClause == rhs.m_limitClause &&
            m_offsetClause == rhs.m_offsetClause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitClause::operator!= (LimitClause const& rhs) const
    {
    return ! (*this == rhs);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitClause::IsNull () const
    {
    return m_isNull;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR LimitClause::GetLimitClause () const
    {
    return m_limitClause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool LimitClause::HasOffsetClause () const
    {
    return !m_offsetClause.empty ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR LimitClause::GetOffsetClause () const
    {
    return m_offsetClause;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String LimitClause::ToString () const
    {
    if (IsNull ())
        return "";

    Utf8String str ("LIMIT ");
    str.append (GetLimitClause ());
    
    if (HasOffsetClause ())
        {
        str.append (" OFFSET ");
        str.append (GetOffsetClause ());
        }

    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
