/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaComparers.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSchemaComparers.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle     06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool operator== (ECClassCR lhs, ECClassCR rhs)
    {
    if (lhs.HasId () && rhs.HasId ())
        {
        return lhs.GetId () == rhs.GetId ();
        }

    return strcmp (lhs.GetFullName (), rhs.GetFullName ()) == 0;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle     06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool operator!= (ECClassCR lhs, ECClassCR rhs)
    {
    return !(lhs == rhs);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle     06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool operator== (ECPropertyCR lhs, ECPropertyCR rhs)
    {
    //PropertyIds are unique in the ECDb file. So they can be compared directly
    if (lhs.HasId () && rhs.HasId ())
        {
        return lhs.GetId () == rhs.GetId ();
        }

    return lhs.GetClass () == rhs.GetClass () && strcmp (lhs.GetName ().c_str (), rhs.GetName ().c_str ()) == 0;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle     06/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool operator!= (ECPropertyCR lhs, ECPropertyCR rhs)
    {
    return !(lhs == rhs);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
