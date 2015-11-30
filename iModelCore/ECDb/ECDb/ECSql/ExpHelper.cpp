/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ExpHelper.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
BentleyStatus ExpHelper::ToPrimitiveType (PrimitiveType& primitiveType, Utf8StringCR type)  
    {
    if (type.EqualsI("int") || type.EqualsI("integer") || type.EqualsI("int32"))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (type.EqualsI("float") || type.EqualsI("double") || type.EqualsI("real"))
        primitiveType = PRIMITIVETYPE_Double;
    else if (type.EqualsI("long") || type.EqualsI("int64") || type.EqualsI("bigint"))
        primitiveType = PRIMITIVETYPE_Long;
    else if (type.EqualsI("string"))
        primitiveType = PRIMITIVETYPE_String;
    else if (type.EqualsI("timestamp") || type.EqualsI("datetime") || type.EqualsI("date"))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (type.EqualsI("binary") || type.EqualsI("blob"))
        primitiveType = PRIMITIVETYPE_Binary;
    else if (type.EqualsI("point2d"))
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (type.EqualsI("point3d"))
        primitiveType = PRIMITIVETYPE_Point3D;
    else if (type.EqualsI("boolean") || type.EqualsI("bool"))
        primitiveType = PRIMITIVETYPE_Boolean;
    else if (type.EqualsI("geometry"))
        primitiveType = PRIMITIVETYPE_IGeometry;
    else
        return ERROR;

    return SUCCESS;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString (ECN::PrimitiveType type)
    {
    switch (type)
        {
        case PRIMITIVETYPE_Binary:
            return "Binary";
        case PRIMITIVETYPE_Boolean:
            return "Boolean";
        case PRIMITIVETYPE_DateTime:
            return "DateTime";
        case PRIMITIVETYPE_Double:
            return "Double";
        case PRIMITIVETYPE_IGeometry:
            return "Geometry";
        case PRIMITIVETYPE_Integer:
            return "Integer";
        case PRIMITIVETYPE_Long:
            return "Long";
        case PRIMITIVETYPE_Point2D:
            return "Point2D";
        case PRIMITIVETYPE_Point3D:
            return "Point3D";
        case PRIMITIVETYPE_String:
            return "String";
        
        default:
            BeAssert(false && "Unhandled case");
            return nullptr;
        }
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString (JoinDirection direction)
    {
    switch (direction)
        {
        case JoinDirection::Forward: return "FORWARD";
        case JoinDirection::Reverse: return "REVERSE";
        case JoinDirection::Implied: return "";
        }
    BeAssert(false && "unhandled case");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString (ECSqlJoinType joinType)
    {
    switch (joinType)
        {
        case ECSqlJoinType::LeftOuterJoin: return "LEFT OUTER JOIN";
        case ECSqlJoinType::RightOuterJoin: return "RIGHT OUTER JOIN";
        case ECSqlJoinType::FullOuterJoin: return "FULL OUTER JOIN";
        case ECSqlJoinType::InnerJoin: return "INNER JOIN";
        case ECSqlJoinType::CrossJoin: return "CROSS JOIN";
        case ECSqlJoinType::NaturalJoin: return "NATURAL";
        case ECSqlJoinType::JoinUsingRelationship: return "USING";
        }
    BeAssert(false && "unhandled case");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(SqlSetQuantifier setQuantifier)
    {
    switch (setQuantifier)
        {
        case SqlSetQuantifier::All: return "ALL";
        case SqlSetQuantifier::Distinct: return "DISTINCT";
        case SqlSetQuantifier::NotSpecified: return "";
        }
    BeAssert(false && "unhandled case");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(SubqueryTestOperator op)
    {
    switch (op)
        {
        case SubqueryTestOperator::Unique: return "UNIQUE";
        case SubqueryTestOperator::Exists: return "EXISTS";
        }
    BeAssert(false && "unhandled case");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(BinarySqlOperator op)
    {
    switch(op)
        {
        //Arthimatics
        case BinarySqlOperator::Divide:        return "/";
        case BinarySqlOperator::Minus:         return "-";
        case BinarySqlOperator::Modulo:        return "%";
        case BinarySqlOperator::Multiply:      return "*";
        case BinarySqlOperator::Plus:          return "+";
        //string concatenation
        case BinarySqlOperator::Concat:        return "||";
        //Bitwise
        case BinarySqlOperator::ShiftLeft:    return "<<";
        case BinarySqlOperator::ShiftRight:   return ">>";
        case BinarySqlOperator::BitwiseAnd:   return "&";
        case BinarySqlOperator::BitwiseOr:    return "|";
        case BinarySqlOperator::BitwiseXOr:   return "^";
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(BooleanSqlOperator op)
    {
    switch(op)
        {
        //Relational
        case BooleanSqlOperator::And:           return "AND";
        case BooleanSqlOperator::Or:            return "OR";
        //Boolean
        case BooleanSqlOperator::EqualTo:            return "=";
        case BooleanSqlOperator::GreaterThanOrEqualTo:            return ">=";
        case BooleanSqlOperator::GreaterThan:            return ">";
        case BooleanSqlOperator::LessThanOrEqualTo:            return "<=";
        case BooleanSqlOperator::LessThan:            return "<";
        case BooleanSqlOperator::NotEqualTo:            return "<>";

        case BooleanSqlOperator::In:            return "IN";
        case BooleanSqlOperator::NotIn:        return "NOT IN";
        case BooleanSqlOperator::Between:       return "BETWEEN";
        case BooleanSqlOperator::NotBetween:   return "NOT BETWEEN";
        case BooleanSqlOperator::Is:            return "IS";
        case BooleanSqlOperator::IsNot:        return "IS NOT";
        //Pattern
        case BooleanSqlOperator::Like:          return "LIKE";
        case BooleanSqlOperator::NotLike:      return "NOT LIKE";      

        case BooleanSqlOperator::Match:         return "MATCH";
        case BooleanSqlOperator::NotMatch:     return "NOT MATCH";
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   08/2013
//---------------------------------------------------------------------------------------
Utf8CP ExpHelper::ToString (SqlCompareListType type)
    {
    switch (type)
        {
        case SqlCompareListType::All:
            return "ALL";
        case SqlCompareListType::Any:
            return "ANY";
        case SqlCompareListType::Some:
            return "SOME";
        }

    BeAssert(false && "case not handled");
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   08/2013
//---------------------------------------------------------------------------------------
Utf8CP ExpHelper::ToString(UnarySqlOperator op)
    {
    switch(op)
        {
        case UnarySqlOperator::Plus:       return "+";
        case UnarySqlOperator::Minus:      return "-";
        case UnarySqlOperator::BitwiseNot: return "~";
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   05/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP ExpHelper::ToString(ECSqlType type)
    {
    switch (type)
        {
            case ECSqlType::Delete:
                return "DELETE";
            case ECSqlType::Insert:
                return "INSERT";
            case ECSqlType::Select:
                return "SELECT";
            case ECSqlType::Update:
                return "UPDATE";

            default:
                BeAssert(false && "ExpHelper::ToString(ECSqlType) needs to be updated to a new value of the ECSqlType enum.");
                return "";
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
