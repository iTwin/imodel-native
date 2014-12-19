/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaComparers.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <BeSQLite/ECDb/ECDbTypes.h>

//non-member functions that should be considered part of the interface
//of the type they operate on should be in same namespace as the type (
//see C++ Coding Standards #57
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! Operator that checks two ECClasses for equality.
//! Equality in the context of ECDb means that it first checks equality of the class ids.
//! If at least one of the classes doesn't have a class id, then the classes' full names
//! (ignoring schema version differences) are compared
//! @Note: This comparer should be used whenever two ECClasses are to be compared. Comparison
//! by pointer / reference must be avoided!
//! @param[in] lhs LHS ECClass
//! @param[in] rhs RHS ECClass
//! @return true if the classes are equal by the definition explained above. false otherwise.
// @bsiclass                                                 Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
bool operator== (ECN::ECClassCR lhs, ECN::ECClassCR rhs);

//=======================================================================================
//! Operator that checks two ECClasses for inequality.
//! Equality in the context of ECDb means that it first checks equality of the class ids.
//! If at least one of the classes doesn't have a class id, then the classes' full names
//! (ignoring schema version differences) are compared
//! @Note: This comparer should be used whenever two ECClasses are to be compared. Comparison
//! by pointer / reference must be avoided!
//! @param[in] lhs LHS ECClass
//! @param[in] rhs RHS ECClass
//! @return true if the classes are not equal by the definition explained above. false otherwise.
// @bsiclass                                                 Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
bool operator!= (ECN::ECClassCR lhs, ECN::ECClassCR rhs);

//=======================================================================================
//! Operator that checks two ECProperties for equality.
//! Equality in the context of ECDb means that it first checks equality of the property ids.
//! If at least one of the properties doesn't have a class id, then the owning ECClasses, and the property names are compared
//! are compared
//! @Note: This comparer should be used whenever two ECProperties are to be compared. Comparison
//! by pointer / reference must be avoided!
//! @param[in] lhs LHS ECProperty
//! @param[in] rhs RHS ECProperty
//! @return true if the ECProperties are equal by the definition explained above. false otherwise.
// @bsiclass                                                 Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
bool operator== (ECN::ECPropertyCR lhs, ECN::ECPropertyCR rhs);

//=======================================================================================
//! Operator that checks two ECClasses for inequality.
//! Equality in the context of ECDb means that it first checks equality of the property ids.
//! If at least one of the properties doesn't have a class id, then the owning ECClasses, and the property names are compared
//! are compared
//! @Note: This comparer should be used whenever two ECProperties are to be compared. Comparison
//! by pointer / reference must be avoided!
//! @param[in] lhs LHS ECProperty
//! @param[in] rhs RHS ECProperty
//! @return true if the ECProperties are not equal by the definition explained above. false otherwise.
// @bsiclass                                                 Krischan.Eberle      06/2013
//+===============+===============+===============+===============+===============+======
bool operator!= (ECN::ECPropertyCR lhs, ECN::ECPropertyCR rhs);

END_BENTLEY_ECOBJECT_NAMESPACE
