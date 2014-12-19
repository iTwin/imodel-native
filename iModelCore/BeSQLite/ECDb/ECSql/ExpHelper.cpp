/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ExpHelper.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
ECSqlStatus ExpHelper::ToPrimitiveType (PrimitiveType& primitiveType, Utf8StringCR type)  
    {
    Utf8String strType = type;
    strType.ToLower();
    if (strType.Equals("int") || strType.Equals("integer") || strType.Equals("int32"))
        primitiveType = PRIMITIVETYPE_Integer;
    else if (strType.Equals("float") || strType.Equals("double")  || strType.Equals("real"))
        primitiveType = PRIMITIVETYPE_Double;
    else if (strType.Equals("long") || strType.Equals("int64") || strType.Equals("bigint"))
        primitiveType = PRIMITIVETYPE_Long;
    else if (strType.Equals("string"))
        primitiveType = PRIMITIVETYPE_String;
    else if (strType.Equals("timestamp") || strType.Equals ("datetime") || strType.Equals("date"))
        primitiveType = PRIMITIVETYPE_DateTime;
    else if (strType.Equals("binary"))
        primitiveType = PRIMITIVETYPE_Binary;
    else if (strType.Equals("point2d"))
        primitiveType = PRIMITIVETYPE_Point2D;
    else if (strType.Equals("point3d"))
        primitiveType = PRIMITIVETYPE_Point3D;
    else if (strType.Equals("boolean"))
        primitiveType = PRIMITIVETYPE_Boolean;
    else
        {
        return ECSqlStatus::InvalidECSql;
        }

    return ECSqlStatus::Success;
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
            return "IGeometry";
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
Utf8CP ExpHelper::ToString(SqlBinaryOperator op)
    {
    switch(op)
        {
        //Arthimatics
        case SqlBinaryOperator::DIVIDE:        return "/";
        case SqlBinaryOperator::MINUS:         return "-";
        case SqlBinaryOperator::MODULUS:       return "%";
        case SqlBinaryOperator::MULTIPLY:      return "*";
        case SqlBinaryOperator::PLUS:          return "+";
        //string concatination
        case SqlBinaryOperator::CONCAT:        return "||";
        //Bitwise
        case SqlBinaryOperator::SHIFT_LEFT:    return "<<";
        case SqlBinaryOperator::SHIFT_RIGHT:   return ">>";
        case SqlBinaryOperator::BITWISE_AND:   return "&"; //Alternate BITWISE_OR(op1, op2) 
        case SqlBinaryOperator::BITWISE_OR:    return "|"; //Alternate BITWISE_AND(op1, op2)  
        case SqlBinaryOperator::BITWISE_XOR:   return "^"; //Alternate BITWISE_XOR(op1, op2) 
                
        /*==Non standard==
        case BinaryOperator::MATCH:         return "MATCH"; 
        case BinaryOperator::GLOB:          return "GLOB"; 
        case BinaryOperator::REGEXP:        return "REGEXP"; 
        */
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(SqlBooleanOperator op)
    {
    switch(op)
        {
        //Relational
        case SqlBooleanOperator::AND:           return "AND";
        case SqlBooleanOperator::OR:            return "OR";
        //Boolean
        case SqlBooleanOperator::EQ:             return "=";
        case SqlBooleanOperator::GE:            return ">=";
        case SqlBooleanOperator::GT:            return ">";
        case SqlBooleanOperator::LE:            return "<=";
        case SqlBooleanOperator::LT:            return "<";
        case SqlBooleanOperator::NE:            return "<>";

        case SqlBooleanOperator::IN:            return "IN";
        case SqlBooleanOperator::NOT_IN:        return "NOT IN";
        case SqlBooleanOperator::BETWEEN:       return "BETWEEN";
        case SqlBooleanOperator::NOT_BETWEEN:   return "NOT BETWEEN";
        case SqlBooleanOperator::IS:            return "IS";
        case SqlBooleanOperator::IS_NOT:        return "IS NOT";
        //Pattern
        case SqlBooleanOperator::LIKE:          return "LIKE";
        case SqlBooleanOperator::NOT_LIKE:      return "NOT LIKE";
                
        /*==Non standard==
        case BooleanOperator::MATCH:         return "MATCH"; 
        case BooleanOperator::GLOB:          return "GLOB"; 
        case BooleanOperator::REGEXP:        return "REGEXP"; 
        */
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

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(SqlUnaryOperator op)
    {
    switch(op)
        {
        case SqlUnaryOperator::PLUS:       return "+";
        case SqlUnaryOperator::MINUS:      return "-";
        case SqlUnaryOperator::BITWISE_NOT:return "~";
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }

//=======================================================================================
//! @bsiclass                                                Affan.Khan      05/2013
//+===============+===============+===============+===============+===============+======
Utf8CP ExpHelper::ToString(SqlBooleanUnaryOperator op)
    {
    switch(op)
        {
        case SqlBooleanUnaryOperator::NOT:        return "NOT";
        }
    BeAssert(false && "case not handled");
    return nullptr;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
