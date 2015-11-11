/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/Exp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;

//************************* Exp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP const Exp::ASTERISK_TOKEN = "*";

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
std::set<ECDbSqlTable const*> Exp::GetReferencedTables() const
    {
    std::set<ECDbSqlTable const*> tmp;
    if (!this->IsComplete())
        {
        BeAssert(false && "This operation is supported on resolved expressions");
        return std::move(tmp);
        }

    auto expList = Find(Type::PropertyName, true);
    for (auto exp : expList)
        {
        auto propertyNameExp = static_cast<PropertyNameExp const*>(exp);
        if (!propertyNameExp->IsPropertyRef())
            {
            auto const& table = propertyNameExp->GetTypeInfo().GetPropertyMap()->GetFirstColumn()->GetTable();
            if (table.GetPersistenceType() == PersistenceType::Persisted)
                tmp.insert(&table);
            }
        }

    return std::move(tmp);
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void Exp::FindRecusive(std::vector<Exp const*>& expList, Exp::Type ofType) const
    {
    if (GetType() == ofType)
        expList.push_back(this);

    for (auto child : GetChildren())
        child->FindRecusive(expList, ofType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void Exp::FindInDirectDecedentOnly(std::vector<Exp const*>& expList, Exp::Type ofType) const
    {
    if (GetType() == ofType)
        expList.push_back(this);

    for (auto child : GetChildren())
        if (child->GetType() == ofType)
            expList.push_back(this);
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
Exp const* Exp::FindParent(Exp::Type type) const
    {
    Exp const* p = this;
    do
        {
        p = p->GetParent();
        } while (p != nullptr && p->GetType() != type);

        return p;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
std::vector<Exp const*> Exp::Find( Exp::Type ofType, bool recusive) const
    {
    std::vector<Exp const*> tmp;
    if (recusive)
        FindRecusive(tmp, ofType);
    else
        FindInDirectDecedentOnly(tmp, ofType);

    return std::move(tmp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus Exp::_FinalizeParsing(ECSqlParseContext&, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::_TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const
    {
    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection const& Exp::GetChildren () const
    {
    return m_children;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection& Exp::GetChildrenR () const
    {
    return m_children;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
size_t Exp::AddChild (unique_ptr<Exp> child)
    {
    BeAssert (child != nullptr);
    child->m_parent = this;
    m_children.m_collection.push_back (move (child));
    //return index of added child
    return m_children.size () - 1;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus Exp::FinalizeParsing (ECSqlParseContext& ctx)
    {
    //some expressions need to finalize themselves before its children and some after their children.
    //So _FinalizeParsing is called two times on each Exp.
    if (!IsComplete ())
        {
        FinalizeParseStatus stat = _FinalizeParsing (ctx, FinalizeParseMode::BeforeFinalizingChildren);
        switch (stat)
            {
            case FinalizeParseStatus::Completed:
                SetIsComplete ();
                break;

            case FinalizeParseStatus::Error:
                return ERROR;
            }
        }

    for (auto child : GetChildrenR ())
        {
        if (SUCCESS != child->FinalizeParsing(ctx))
            return ERROR;
        }

    if (!IsComplete ())
        {
        FinalizeParseStatus stat = _FinalizeParsing(ctx, FinalizeParseMode::AfterFinalizingChildren);
        if (stat == FinalizeParseStatus::Error)
            return ERROR;

        BeAssert (IsParameterExp() || stat != FinalizeParseStatus::NotCompleted && "Every expression except for parameter exps is expected to be either completed or return an error from finalize parsing.");
        if (stat == FinalizeParseStatus::Completed)
            SetIsComplete ();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    if (_TryDetermineParameterExpType(ctx, parameterExp))
        return true;

    Exp const* parentExp = GetParent();
    if (parentExp != nullptr)
        return parentExp->TryDetermineParameterExpType(ctx, parameterExp);

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    05/2015
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String Exp::ToECSql() const
    {
    return _ToECSql();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String Exp::ToString () const
    {
    return _ToString ();
    }


//************************* Exp::Collection *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
size_t Exp::Collection::size () const
    {
    return m_collection.size ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::Collection::empty () const
    {
    return m_collection.empty ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ExpCP Exp::Collection::operator[] (size_t i) const
    {
    return m_collection[i].get ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ExpP Exp::Collection::operator[] (size_t i)
    {
    return m_collection[i].get ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection::const_iterator<ExpCP> Exp::Collection::begin () const
    {
    return const_iterator<ExpCP> (m_collection.begin ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection::const_iterator<ExpP> Exp::Collection::begin ()
    {
    return const_iterator<ExpP> (m_collection.begin ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection::const_iterator<ExpCP> Exp::Collection::end () const
    {
    return const_iterator<ExpCP> (m_collection.end ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::Collection::const_iterator<ExpP> Exp::Collection::end ()
    {
    return const_iterator<ExpP> (m_collection.end ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::Collection::Replace (ExpCR replacee, std::vector<std::unique_ptr<Exp>>& replaceWith)
    {
    std::vector<std::unique_ptr<Exp>> copiedCollection = move (m_collection);
    BeAssert (m_collection.empty ());

    bool found = false;
    for (size_t i = 0; i < copiedCollection.size (); i++)
        {
        auto& expr = copiedCollection[i];
        if (expr.get () != &replacee)
            m_collection.push_back (move (expr));
        else
            {
            for (auto& newExp : replaceWith)
                {
                m_collection.push_back (move (newExp));
                }
            found = true;
            }
        }

    return found;
    }

//****************************** Exp::ECSqlSpecialTokenIndexMap *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
void Exp::SystemPropertyExpIndexMap::AddIndex (ECSqlSystemProperty systemPropertyExp, size_t index)
    {
    m_indexMap[systemPropertyExp] = index;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
int Exp::SystemPropertyExpIndexMap::GetIndex (ECSqlSystemProperty systemPropertyExp) const
    {
    auto it = m_indexMap.find (systemPropertyExp);
    if (it == m_indexMap.end ())
        return UNSET_CHILDINDEX;
    else
        return (int) it->second;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool Exp::SystemPropertyExpIndexMap::IsUnset (ECSqlSystemProperty systemPropertyExp) const
    {
    return GetIndex (systemPropertyExp) == UNSET_CHILDINDEX;
    }


//****************************** PropertyPath *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyPath& PropertyPath::operator=(PropertyPath const& rhs)
    {
    if (this != &rhs)
        {
        m_path = rhs.m_path;
        m_classMap = rhs.m_classMap;
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyPath& PropertyPath::operator=(PropertyPath&& rhs)
    {
    if (this != &rhs)
        {
        m_path = std::move(rhs.m_path);
        m_classMap = std::move(rhs.m_classMap);
        }

    return *this;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyPath::IsResolved() const
    {
    for (Location const& entry : m_path)
        {
        if (!entry.IsResolved())
            return false;
        }

    return true;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Push(Utf8CP propertyName, size_t arrayIndex)
    {
    m_path.push_back(Location(propertyName, (int) arrayIndex));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Push(Utf8CP propertyName)
    {
    m_path.push_back(Location(propertyName, Location::NOT_ARRAY));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Pop()
    {
    m_path.pop_back();
    if (IsEmpty())
        Clear();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyPath::Resolve(IClassMap const& classMap, Utf8String* errorMessage)
    {
    if (IsEmpty())
        return ERROR;

    ECClassCP cursorClass = &classMap.GetClass();
    const size_t leafIndex = m_path.size() - 1;
    bool isLeafEntry = false;
    for (size_t i = 0; i < m_path.size(); i++)
        {
        if (i == leafIndex)
            isLeafEntry = true;

        Location& element = m_path[i];
        Utf8CP propertyName = element.GetPropertyName();
        if (element.HasArrayIndex())
            {
            Reset();
            if (errorMessage != nullptr)
                errorMessage->Sprintf("Array indices are not yet supported in ECSQL. Invalid property access string: %s", ToString().c_str());
            return ERROR;
            }

        ECPropertyCP property = cursorClass->GetPropertyP(propertyName, true);

        if (property == nullptr && i == 0)
            {
            PropertyMapCP propertyMap = classMap.GetPropertyMap(propertyName);
            if (propertyMap != nullptr)
                property = &propertyMap->GetProperty();
            }

        if (property == nullptr)
            {
            Reset();
            if (errorMessage != nullptr)
                errorMessage->Sprintf("ECProperty '%s' in property access string '%s' does not exist.", propertyName, ToString().c_str());
            
            return ERROR;
            }

        element.SetProperty(*property);

        if (property->GetIsPrimitive())
            {
            if (!isLeafEntry || element.HasArrayIndex())
                {
                Reset();
                if (errorMessage != nullptr)
                    errorMessage->Sprintf("Invalid property access string '%s'.", ToString().c_str());
                return ERROR;
                }
            }
        else if (property->GetIsStruct())
            {
            if (element.HasArrayIndex())
                {
                Reset();
                if (errorMessage != nullptr)
                    errorMessage->Sprintf("Invalid property access string '%s'.", ToString().c_str());
                return ERROR;
                }

            cursorClass = &property->GetAsStructProperty()->GetType();
            }
        else if (property->GetIsArray())
            {
            auto arrayProperty = property->GetAsArrayProperty();
            auto structArrayProperty = property->GetAsStructArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
                {
                if (!isLeafEntry)
                    {
                    Reset();
                    if (errorMessage != nullptr)
                        errorMessage->Sprintf("Invalid property access string '%s'.", ToString().c_str());
                    return ERROR;
                    }
                }
            else if (nullptr != structArrayProperty)
                {
                cursorClass = structArrayProperty->GetStructElementType();
                }
            }
        }

    BeAssert(IsResolved() && "Must be resolved by now");
    m_classMap = &classMap;
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Clear()
    {
    m_path.clear();
    m_classMap = nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyPath::ToString(bool includeArrayIndexes /*= true*/) const
    {
    Utf8String str;
    bool isFirstLoc = true;
    for (Location const& loc : m_path)
        {
        if (!isFirstLoc)
            str.append(".");

        str.append(loc.ToString(includeArrayIndexes));

        isFirstLoc = false;
        }

    return std::move(str);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus PropertyPath::TryParseQualifiedPath(PropertyPath& resolvedPropertyPath, Utf8StringCR qualifedPath, ECDbCR ecdb)
    {
    resolvedPropertyPath.Clear();
    bvector<Utf8String> subParts;
    BeStringUtilities::Split(qualifedPath.c_str(), ":", nullptr, subParts);
    if (subParts.size() != 3)
        {
        BeAssert(false && "Invalid qualified path");
        return ERROR;
        }

    auto& schemaName = subParts.at(0);
    auto& className = subParts.at(1);
    auto& propertyPath = subParts.at(2);

    bvector<Utf8String> propertyNames;
    BeStringUtilities::Split(propertyPath.c_str(), ".", nullptr, propertyNames);
    for (Utf8StringCR propertyName : propertyNames)
        resolvedPropertyPath.Push(propertyName.c_str());

    ECClassCP targetClass = ecdb.Schemas().GetECClass(schemaName.c_str(), className.c_str(), ResolveSchema::AutoDetect);
    if (targetClass == nullptr)
        {
        BeAssert(false && "Failed to find ECClass");
        return ERROR;
        }

    IClassMap const* targetClassMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(*targetClass);
    if (targetClassMap == nullptr)
        {
        BeAssert(false && "No class map found for class.");
        return ERROR;
        }

    return resolvedPropertyPath.Resolve(*targetClassMap);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyPath::TryGetQualifiedPath(Utf8StringR qualifiedPath) const
    {
    if (m_classMap == nullptr)
        {
        BeAssert(false && "Invalid property path");
        return ERROR;
        }

    qualifiedPath.assign(GetClassMap()->GetClass().GetFullName());
    qualifiedPath.append(":");
    qualifiedPath.append(ToString(false));
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Reset()
    {
    m_classMap = nullptr;
    for (Location& loc : m_path)
        loc.ClearResolvedProperty();
    }


//****************************** PropertyPath::Location *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Location::SetProperty(ECPropertyCR property)
    {
    BeAssert(property.GetName().Equals(GetPropertyName()));
    m_property = &property;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyPath::Location::ToString(bool includeArrayIndexes) const
    {
    if (GetArrayIndex() == NOT_ARRAY || !includeArrayIndexes)
        return m_propertyName;

    Utf8String tmp;
    tmp.Sprintf("%s[%d]", m_propertyName.c_str(), GetArrayIndex());
    return tmp;
    }



END_BENTLEY_SQLITE_EC_NAMESPACE
