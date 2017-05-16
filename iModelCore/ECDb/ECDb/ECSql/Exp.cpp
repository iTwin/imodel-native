/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/Exp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************* Exp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP const Exp::ASTERISK_TOKEN = "*";


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void Exp::FindRecursive(std::vector<Exp const*>& expList, Exp::Type ofType) const
    {
    if (GetType() == ofType)
        expList.push_back(this);

    for (Exp const* child : m_derivedTables)
        child->FindRecursive(expList, ofType);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                    09/2015
//+---------------+---------------+---------------+---------------+---------------+--------
void Exp::FindInDirectDecendents(std::vector<Exp const*>& expList, Exp::Type ofType) const
    {
    if (GetType() == ofType)
        expList.push_back(this);

    for (Exp const* child : m_derivedTables)
        {
        if (child->GetType() == ofType)
            expList.push_back(this);
        }
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
std::vector<Exp const*> Exp::Find(Exp::Type ofType, bool recusive) const
    {
    std::vector<Exp const*> tmp;
    if (recusive)
        FindRecursive(tmp, ofType);
    else
        FindInDirectDecendents(tmp, ofType);

    return tmp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
size_t Exp::AddChild(std::unique_ptr<Exp> child)
    {
    BeAssert(child != nullptr);
    child->m_parent = this;
    m_derivedTables.m_collection.push_back(std::move(child));
    //return index of added child
    return m_derivedTables.size() - 1;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus Exp::FinalizeParsing(ECSqlParseContext& ctx)
    {
    //some expressions need to finalize themselves before its children and some after their children.
    //So _FinalizeParsing is called two times on each Exp.
    if (!IsComplete())
        {
        FinalizeParseStatus stat = _FinalizeParsing(ctx, FinalizeParseMode::BeforeFinalizingChildren);
        switch (stat)
            {
                case FinalizeParseStatus::Completed:
                    SetIsComplete();
                    break;

                case FinalizeParseStatus::Error:
                    return ERROR;
            }
        }

    for (Exp* child : m_derivedTables)
        {
        if (SUCCESS != child->FinalizeParsing(ctx))
            return ERROR;
        }

    if (!IsComplete())
        {
        FinalizeParseStatus stat = _FinalizeParsing(ctx, FinalizeParseMode::AfterFinalizingChildren);
        if (stat == FinalizeParseStatus::Error)
            return ERROR;

        BeAssert(IsParameterExp() || stat != FinalizeParseStatus::NotCompleted && "Every expression except for parameter exps is expected to be either completed or return an error from finalize parsing.");
        if (stat == FinalizeParseStatus::Completed)
            SetIsComplete();
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


//************************* Exp::Collection *******************************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::Collection::Replace(ExpCR replacee, std::vector<std::unique_ptr<Exp>>& replaceWith)
    {
    std::vector<std::unique_ptr<Exp>> copiedCollection = move(m_collection);
    BeAssert(m_collection.empty());

    bool found = false;
    for (size_t i = 0; i < copiedCollection.size(); i++)
        {
        auto& expr = copiedCollection[i];
        if (expr.get() != &replacee)
            m_collection.push_back(move(expr));
        else
            {
            for (auto& newExp : replaceWith)
                {
                m_collection.push_back(move(newExp));
                }

            found = true;
            }
        }

    return found;
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
void PropertyPath::Pop()
    {
    m_path.pop_back();
    if (IsEmpty())
        Clear();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyPath::Resolve(ClassMap const& classMap, Utf8String* errorMessage)
    {
    if (IsEmpty())
        return ERROR;

    Utf8String accessString = ToString(false, false);
    PropertyMap const * propertyMap = classMap.GetPropertyMaps().Find(accessString.c_str());
    if (propertyMap == nullptr)
        {
        Reset();
        if (errorMessage != nullptr)
            errorMessage->Sprintf("Fail to find accessString '%s'.", ToString().c_str());

        return ERROR;
        }

    PropertyMap::Path propertyPath = propertyMap->GetPath();
    if (propertyPath.size() != m_path.size())
        {
        BeAssert(false && "Programmer Error");
        return ERROR;
        }

    for (size_t i = 0; i < m_path.size(); i++)
        {
        Location& element = m_path[i];
        if (element.HasArrayIndex())
            {
            Reset();
            if (errorMessage != nullptr)
                errorMessage->Sprintf("Array indices are not yet supported in ECSQL. Invalid property access string: %s", ToString().c_str());
            return ERROR;
            }

        element.SetProperty(propertyPath[i].GetProperty());
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
Utf8String PropertyPath::ToString(bool escape, bool includeArrayIndexes /*= true*/) const
    {
    Utf8String str;
    bool isFirstLoc = true;
    for (Location const& loc : m_path)
        {
        if (!isFirstLoc)
            str.append(".");

        if (escape)
            str.append("[");

        str.append(loc.ToString(includeArrayIndexes));

        if (escape)
            str.append("]");

        isFirstLoc = false;
        }

    return str;
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

    Utf8StringCR schemaName = subParts.at(0);
    Utf8StringCR className = subParts.at(1);
    Utf8StringCR propertyPath = subParts.at(2);

    bvector<Utf8String> propertyNames;
    BeStringUtilities::Split(propertyPath.c_str(), ".", nullptr, propertyNames);
    for (Utf8StringCR propertyName : propertyNames)
        {
        resolvedPropertyPath.Push(propertyName);
        }

    ECClassCP targetClass = ecdb.Schemas().GetClass(schemaName, className, SchemaLookupMode::AutoDetect);
    if (targetClass == nullptr)
        {
        BeAssert(false && "Failed to find ECClass");
        return ERROR;
        }

    ClassMap const* targetClassMap = ecdb.Schemas().GetDbMap().GetClassMap(*targetClass);
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
    // NEEDSWORK_STRUCTS: BeAssert(property.GetName().Equals(GetPropertyName()));
    m_property = &property;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyPath::Location::ToString(bool includeArrayIndexes) const
    {
    if (GetArrayIndex() < 0 || !includeArrayIndexes)
        return m_propertyName;

    Utf8String tmp;
    tmp.Sprintf("%s[%d]", m_propertyName.c_str(), GetArrayIndex());
    return tmp;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
