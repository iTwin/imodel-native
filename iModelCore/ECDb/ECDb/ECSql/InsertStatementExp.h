/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassRefExp.h"
#include "ListExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      11/2013
//+===============+===============+===============+===============+===============+======
struct InsertStatementExp final : Exp
    {
private:
    size_t m_classNameExpIndex;
    size_t m_propertyNameListExpIndex;
    size_t m_valuesExpIndex;
    bool m_isOriginalPropertyNameListUnset;

    std::vector<RangeClassInfo> m_rangeClassRefExpCache;

    FinalizeParseStatus _FinalizeParsing(ECSqlParseContext&, FinalizeParseMode mode) override;
    bool _TryDetermineParameterExpType(ECSqlParseContext&, ParameterExp&) const override;

    FinalizeParseStatus Validate (ECSqlParseContext&) const;

    void _ToECSql(ECSqlRenderContext& ctx) const override;
    Utf8String _ToString() const override { return "Insert"; }

    PropertyNameListExp* GetPropertyNameListExpP () const;

    bool IsOriginalPropertyNameListUnset () const {return m_isOriginalPropertyNameListUnset;}

public :
    InsertStatementExp (std::unique_ptr<ClassNameExp>& classNameExp, std::unique_ptr<PropertyNameListExp>& propertyNameListExp,
                        std::vector<std::unique_ptr<ValueExp>>& valuesExp);

    ClassNameExp const* GetClassNameExp () const;
    PropertyNameListExp const* GetPropertyNameListExp () const;
    ValueExpListExp const* GetValuesExp() const;
    };



END_BENTLEY_SQLITE_EC_NAMESPACE

