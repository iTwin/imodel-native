/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************* Exp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP const Exp::ASTERISK_TOKEN = "*";



//-----------------------------------------------------------------------------------------
// @bsimethod
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

// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool Exp::Contains(Exp::Type candidateType) const
    {
    if (GetType() == candidateType)
        return true;

    for (Exp const* child : m_children)
        {
        if (child->Contains(candidateType))
            return true;
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
std::vector<Exp const*> Exp::Find(Exp::Type candidateType, bool recursive) const
    {
    std::vector<Exp const*> tmp;
    Find(tmp, candidateType, recursive);
    return tmp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void Exp::Find(std::vector<Exp const*>& expList, Exp::Type candidateType, bool recursive) const
    {
    if (GetType() == candidateType)
        expList.push_back(this);

    if (!recursive)
        return;

    for (Exp const* child : m_children)
        child->Find(expList, candidateType, recursive);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
size_t Exp::AddChild(std::unique_ptr<Exp> child)
    {
    BeAssert(child != nullptr);
    child->m_parent = this;
    m_children.m_collection.push_back(std::move(child));
    //return index of added child
    return m_children.size() - 1;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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

    for (Exp* child : m_children)
        {
        if (SUCCESS != child->FinalizeParsing(ctx))
            return ERROR;
        }



    if (!IsComplete())
        {
        FinalizeParseStatus stat = _FinalizeParsing(ctx, FinalizeParseMode::AfterFinalizingChildren);
        if (ctx.GetDeferFinalize())
            return SUCCESS;

        if (stat == FinalizeParseStatus::Error)
            return ERROR;

        BeAssert(IsParameterExp() || stat != FinalizeParseStatus::NotCompleted && "Every expression except for parameter exps is expected to be either completed or return an error from finalize parsing.");
        if (stat == FinalizeParseStatus::Completed)
            SetIsComplete();
        }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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

//****************************** PropertyPath *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Pop()
    {
    m_path.pop_back();
    if (IsEmpty())
        Clear();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyPath PropertyPath::Skip(size_t k) const
    {
    PropertyPath path;
    for (auto i = k; i < m_path.size(); i++)
        path.Push(m_path[i].GetName(), m_path[i].GetArrayIndex());

    return path;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyPath PropertyPath::Take(size_t k) const
    {
    PropertyPath path;
    for (auto i = 0; i < k; i++)
        path.Push(m_path[i].GetName(), m_path[i].GetArrayIndex());

    return path;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Clear()
    {
    m_path.clear();
    m_classMap = nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Reset()
    {
    m_classMap = nullptr;
    for (Location& loc : m_path)
        loc.ClearResolvedProperty();
    }


//****************************** PropertyPath::Location *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyPath::Location::SetProperty(ECPropertyCR property)
    {
    // NEEDSWORK_STRUCTS: BeAssert(property.GetName().Equals(GetPropertyName()));
    m_property = &property;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyPath::Location::ToString(bool includeArrayIndexes) const
    {
    if (GetArrayIndex() < 0 || !includeArrayIndexes)
        return m_name;

    Utf8String tmp;
    tmp.Sprintf("%s[%d]", m_name.c_str(), GetArrayIndex());
    return tmp;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
