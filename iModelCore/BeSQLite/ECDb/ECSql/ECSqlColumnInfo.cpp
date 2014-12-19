/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlColumnInfo.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeSQLite/ECDb/ECSqlColumnInfo.h>
#include "ECSqlPropertyPathImpl.h"

using namespace std;
using namespace ECN;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//********************** ECSqlPropertyPath **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::ECSqlPropertyPath ()
    : m_pimpl (new ECSqlPropertyPath::Impl ())
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::~ECSqlPropertyPath ()
    {
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    } 

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::ECSqlPropertyPath (ECSqlPropertyPath const& rhs)
    : m_pimpl (rhs.m_pimpl)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath& ECSqlPropertyPath::operator= (ECSqlPropertyPath const& rhs)
    {
    if (this != &rhs)
        m_pimpl = rhs.m_pimpl;

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::ECSqlPropertyPath (ECSqlPropertyPath&& rhs)
    : m_pimpl (move (rhs.m_pimpl))
    {
    //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
    rhs.m_pimpl = nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath& ECSqlPropertyPath::operator= (ECSqlPropertyPath&& rhs)
    {
    if (this != &rhs)
        {
        m_pimpl = move (rhs.m_pimpl);
        //nulling out the pimpl on the RHS to avoid that rhs' destructor tries to delete it
        rhs.m_pimpl = nullptr;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry (ECPropertyCR ecProperty)
    {
    return m_pimpl->AddEntry (ecProperty);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry (int arrayIndex)
    {
    return m_pimpl->AddEntry (arrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::InsertEntriesAtBeginning (ECSqlPropertyPath const& pathToInsert)
    {
    return m_pimpl->InsertParentEntries (pathToInsert);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlPropertyPath::SetLeafArrayIndex (int newArrayIndex)
    {
    return m_pimpl->SetLeafArrayIndex (newArrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECSqlPropertyPath::Size () const
    {
    return m_pimpl->Size ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::EntryCR ECSqlPropertyPath::At (size_t index) const
    {
    return m_pimpl->At (index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::EntryCR ECSqlPropertyPath::GetLeafEntry () const
    {
    return m_pimpl->GetLeafEntry ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::begin () const
    {
    return m_pimpl->begin ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::end () const
    {
    return m_pimpl->end ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlPropertyPath::ToString (ECSqlPropertyPath::FormatOptions options) const
    {
    return m_pimpl->ToString (options);
    }



//********************** ECSqlPropertyPath::Entry **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry::Entry (ECPropertyCR ecProperty)
: m_property (&ecProperty), m_arrayIndex (-1)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry::Entry (int arrayIndex)
    : m_property (nullptr), m_arrayIndex (arrayIndex)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry::Entry (ECSqlPropertyPath::Entry const& rhs)
    : m_property (rhs.m_property), m_arrayIndex (rhs.m_arrayIndex)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry& ECSqlPropertyPath::Entry::operator= (ECSqlPropertyPath::Entry const& rhs)
    {
    if (this != &rhs)
        {
        m_property = rhs.m_property;
        m_arrayIndex = rhs.m_arrayIndex;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry::Entry (ECSqlPropertyPath::Entry&& rhs)
    : m_property (move (rhs.m_property)), m_arrayIndex (move (rhs.m_arrayIndex))
    {
    rhs.m_property = nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry& ECSqlPropertyPath::Entry::operator= (ECSqlPropertyPath::Entry&& rhs)
    {
    if (this != &rhs)
        {
        m_property = move (rhs.m_property);
        m_arrayIndex = move (rhs.m_arrayIndex);

        rhs.m_property = nullptr;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Entry::Kind ECSqlPropertyPath::Entry::GetKind () const
    {
    return m_property != nullptr ? Kind::Property : Kind::ArrayIndex;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECPropertyCP ECSqlPropertyPath::Entry::GetProperty () const
    {
    BeAssert (GetKind () == Kind::Property && "ECSqlPropertyPath::Entry::GetProperty can only be called if GetKind () == Kind::Property.");
    return m_property;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlPropertyPath::Entry::GetArrayIndex () const
    {
    BeAssert (GetKind () == Kind::ArrayIndex && "ECSqlPropertyPath::Entry::GetArrayIndex can only be called if GetKind () == Kind::ArrayIndex.");
    return m_arrayIndex;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlPropertyPath::Entry::SetArrayIndex (int newArrayIndex)
    {
    if (GetKind () != Kind::ArrayIndex)
        {
        BeAssert (GetKind () == Kind::ArrayIndex && "ECSqlPropertyPath::Entry::SetArrayIndex can only be called if GetKind () == Kind::ArrayIndex.");
        return ERROR;
        }

    m_arrayIndex = newArrayIndex;
    return SUCCESS;
    }

//********************** ECSqlPropertyPath::const_iterator **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::const_iterator (ECSqlPropertyPath::const_iterator::Impl* impl)
: m_pimpl (impl)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::~const_iterator()
    {
    if (m_pimpl != nullptr)
        {
        delete m_pimpl;
        m_pimpl = nullptr;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::EntryCP ECSqlPropertyPath::const_iterator::operator* () const
    {
    return GetPimpl ().m_iterator->get ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator& ECSqlPropertyPath::const_iterator::operator++ ()
    {
    GetPimplR ().m_iterator++;
    
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlPropertyPath::const_iterator::operator== (const_iterator const& rhs) const
    {
    return GetPimpl ().m_iterator == rhs.GetPimpl ().m_iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlPropertyPath::const_iterator::operator!= (const_iterator const& rhs) const
    {
    return !(*this == rhs);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::Impl const& ECSqlPropertyPath::const_iterator::GetPimpl () const
    {
    BeAssert (m_pimpl != nullptr);
    return *m_pimpl;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::Impl& ECSqlPropertyPath::const_iterator::GetPimplR () const
    {
    BeAssert (m_pimpl != nullptr);
    return *m_pimpl;
    }


//********************** ECSqlColumnInfo **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo ()
    : m_property (nullptr), m_isGeneratedProperty (false)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo (ECTypeDescriptor const& dataType, ECPropertyCP ecProperty, bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    : m_dataType (dataType), m_property (ecProperty), m_isGeneratedProperty (isGeneratedProperty), m_propertyPath (move (propertyPath)), m_rootClass (&rootClass), m_rootClassAlias (rootClassAlias)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateTopLevel (bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    {
    BeAssert (propertyPath.Size () > 0);
    auto ecProperty = propertyPath.GetLeafEntry ().GetProperty ();
    BeAssert (ecProperty != nullptr);
    auto dataType = DetermineDataType (*ecProperty);
    return ECSqlColumnInfo (dataType, ecProperty, isGeneratedProperty, move (propertyPath), rootClass, rootClassAlias);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateChild (ECSqlColumnInfo const& parent, ECPropertyCR childProperty)
    {
    auto dataType = DetermineDataType (childProperty);
    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning (parent.GetPropertyPath ());
    childPropPath.AddEntry (childProperty);

    return ECSqlColumnInfo (dataType, &childProperty, parent.IsGeneratedProperty (), move (childPropPath), parent.GetRootClass (), parent.GetRootClassAlias ());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateForArrayElement (ECSqlColumnInfo const& parent, int arrayIndex)
    {
    ECTypeDescriptor arrayElementDataType;
    if (parent.GetDataType ().IsPrimitiveArray ())
        arrayElementDataType = ECTypeDescriptor::CreatePrimitiveTypeDescriptor (parent.GetDataType ().GetPrimitiveType ());
    else
        {
        BeAssert (parent.GetDataType ().IsStructArray ());
        arrayElementDataType = ECTypeDescriptor::CreateStructTypeDescriptor ();
        }

    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning (parent.GetPropertyPath ());
    childPropPath.AddEntry (arrayIndex);

    return ECSqlColumnInfo (arrayElementDataType, nullptr, parent.IsGeneratedProperty (), move (childPropPath), parent.GetRootClass (), parent.GetRootClassAlias ());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::~ECSqlColumnInfo ()
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo (ECSqlColumnInfo const& rhs)
    : m_dataType (rhs.m_dataType), m_property (rhs.m_property), m_isGeneratedProperty (rhs.m_isGeneratedProperty), m_propertyPath (rhs.m_propertyPath), m_rootClass (rhs.m_rootClass), m_rootClassAlias (rhs.m_rootClassAlias)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo& ECSqlColumnInfo::operator= (ECSqlColumnInfo const& rhs)
    {
    if (this != &rhs)
        {
        m_dataType = rhs.m_dataType;
        m_property = rhs.m_property;
        m_isGeneratedProperty = rhs.m_isGeneratedProperty;
        m_propertyPath = rhs.m_propertyPath;
        m_rootClass = rhs.m_rootClass;
        m_rootClassAlias = rhs.m_rootClassAlias;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo (ECSqlColumnInfo&& rhs)
    : m_dataType (move (rhs.m_dataType)), m_property (move (rhs.m_property)), m_isGeneratedProperty (move (rhs.m_isGeneratedProperty)), m_propertyPath (move (rhs.m_propertyPath)), m_rootClass (move (rhs.m_rootClass)), m_rootClassAlias (move (rhs.m_rootClassAlias))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo& ECSqlColumnInfo::operator= (ECSqlColumnInfo&& rhs)
    {
    if (this != &rhs)
        {
        m_dataType = move (rhs.m_dataType);
        m_property = move (rhs.m_property);
        m_isGeneratedProperty = move (rhs.m_isGeneratedProperty);
        m_propertyPath = move (rhs.m_propertyPath);
        m_rootClass = move (rhs.m_rootClass);
        m_rootClassAlias = move (rhs.m_rootClassAlias);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECTypeDescriptor const& ECSqlColumnInfo::GetDataType () const
    {
    return m_dataType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECPropertyCP ECSqlColumnInfo::GetProperty () const
    {
    return m_property;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlColumnInfo::IsGeneratedProperty () const
    {
    return m_isGeneratedProperty;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPathCR ECSqlColumnInfo::GetPropertyPath () const
    {
    return m_propertyPath;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath& ECSqlColumnInfo::GetPropertyPathR ()
    {
    return m_propertyPath;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCR ECSqlColumnInfo::GetRootClass () const
    {
    BeAssert (m_rootClass != nullptr && "m_rootClass must never be null.");
    return *m_rootClass;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP ECSqlColumnInfo::GetRootClassAlias () const
    {
    BeAssert (m_rootClass != nullptr && "m_rootClass must never be null.");
    return m_rootClassAlias.c_str ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECTypeDescriptor ECSqlColumnInfo::DetermineDataType (ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsPrimitive ())
        return ECTypeDescriptor::CreatePrimitiveTypeDescriptor (ecProperty.GetAsPrimitiveProperty ()->GetType ());
    else if (ecProperty.GetIsStruct ())
        return ECTypeDescriptor::CreateStructTypeDescriptor ();
    else
        {
        auto arrayProp = ecProperty.GetAsArrayProperty ();
        if (arrayProp->GetKind () == ARRAYKIND_Primitive)
            return ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType ());
        else
            return ECTypeDescriptor::CreateStructArrayTypeDescriptor ();
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE