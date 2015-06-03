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
ECSqlStatus Exp::FinalizeParsing (ECSqlParseContext& ctx)
    {
    //some expressions need to finalize themselves before its children and some after their children.
    //So _FinalizeParsing is called two times on each Exp.
    if (!IsComplete ())
        {
        auto stat = _FinalizeParsing (ctx, FinalizeParseMode::BeforeFinalizingChildren);
        switch (stat)
            {
            case FinalizeParseStatus::Completed:
                SetIsComplete ();
                break;

            case FinalizeParseStatus::Error:
                return ctx.GetStatus ();
            }
        }

    for (auto child : GetChildrenR ())
        {
        auto stat = child->FinalizeParsing (ctx);
        if (stat != ECSqlStatus::Success)
            return stat;
        }

    if (!IsComplete ())
        {
        auto stat = _FinalizeParsing(ctx, FinalizeParseMode::AfterFinalizingChildren);
        BeAssert (IsParameterExp() || stat != FinalizeParseStatus::NotCompleted && "Every expression except for parameter exps is expected to be either completed or return an error from finalize parsing.");
        if (stat == FinalizeParseStatus::Completed)
            SetIsComplete ();

        if (stat == FinalizeParseStatus::Error && ctx.GetStatus () == ECSqlStatus::Success)
            {
            BeAssert (false && "FinalizeParsing returned error, but did not set that error in the status context.");
            ctx.SetError (ECSqlStatus::ProgrammerError, "FinalizeParsing of expression '%s' returned error, but did not set that error in the status context.", ToECSql ().c_str ());
            }
        }

    return ctx.GetStatus ();
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


//****************************** PropertyPath::Location *****************************************
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
