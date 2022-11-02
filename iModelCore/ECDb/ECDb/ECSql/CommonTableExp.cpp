/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** CommonTableBlockExp *************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
CommonTableBlockExp::CommonTableBlockExp(Utf8CP name, std::vector<Utf8String> colList, std::unique_ptr<SelectStatementExp> stmt)
    :RangeClassRefExp(Exp::Type::CommonTableBlock, false), m_name(name), m_columnList(colList),m_deferredExpand(true) {
    AddChild(std::move(stmt));
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool CommonTableBlockExp::ExpandDerivedProperties() const { 
    // when we encounter wild card we will leave it deferred.
    if (!m_deferredExpand) {
        return true;
    }
    auto query = GetQuery();
    auto cols = std::min(query->GetSelection()->GetChildrenCount(), m_columnList.size());
    for (auto i = 0; i < cols; ++i) {
        auto target = query->GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
        if (target->IsWildCard()) {
            return false;
        }
    }
    for (auto i = 0; i < cols; ++i) {
        auto target = query->GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
        auto property = std::make_unique<CommonTablePropertyNameExp>(m_columnList[i].c_str(), *target, [&](Utf8String col) {
            return FindType(col);
        });
        const_cast<CommonTableBlockExp*>(this)->AddChild(std::make_unique<DerivedPropertyExp>(std::move(property), nullptr));
    }
    m_deferredExpand = false;
    return !m_deferredExpand;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus CommonTableBlockExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    ExpandDerivedProperties();
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren) {
        // Make sure columns in CTE Block are unique
        std::set<Utf8CP, CompareIUtf8Ascii> uniqueCols;
        for (auto& col : m_columnList) {
            auto it = uniqueCols.insert(col.c_str());
            if (false == it.second) {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Invalid ECSql : Common table '%s' has duplicate column with same name '%s'. %s", GetName().c_str(), col.c_str(), ToECSql().c_str());
                return FinalizeParseStatus::Error;
            }
        }

        // The column and value count must match. This is a differed error from parsing.
        const auto columns = GetColumns().size();
        const auto values = GetQuery()->GetSelection()->GetChildrenCount();
        if (columns != values) {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Invalid ECSql : Common table '%s' has %d values for columns %d. %s", GetName().c_str(), columns, values, ToECSql().c_str());
            return FinalizeParseStatus::Error;
        }
        
        return FinalizeParseStatus::Completed;
    }

    BeAssert(false && "Programmer Error");
    return FinalizeParseStatus::Error;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlTypeInfo CommonTableBlockExp::FindType (Utf8StringCR col) const {
    ECSqlTypeInfo resolvedTypeInfo;

    auto it = std::find(m_columnList.begin(), m_columnList.end(), col);
    if (it != m_columnList.end()) {
        auto columnIdx = std::distance(m_columnList.begin(), it);
        auto singleStmtExp = GetQuery()->GetFlatListOfStatements();
        for (auto stmtIdx = 0; stmtIdx < singleStmtExp.size(); ++stmtIdx) {
            auto typeInfo = singleStmtExp[stmtIdx]->GetSelection()->GetChildren().Get<DerivedPropertyExp>(columnIdx)->GetExpression()->GetTypeInfo();
            // try to find non-null type info
            if (resolvedTypeInfo.GetKind() == ECSqlTypeInfo::Kind::Unset || resolvedTypeInfo.IsNull() && !typeInfo.IsNull()) {
                resolvedTypeInfo = typeInfo;
            }
        }
    }
    return resolvedTypeInfo;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableBlockExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql(GetName()).AppendToECSql("(");
    if (!GetColumns().empty())
        ctx.AppendToECSql(GetColumns().front());

    for (size_t i = 1; i < this->GetColumns().size(); ++i) {
        ctx.AppendToECSql(", ").AppendToECSql(GetColumns().at(i));
    }
    ctx.AppendToECSql(") AS (");
    GetQuery()->ToECSql(ctx);
    ctx.AppendToECSql(")");
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String CommonTableBlockExp::_ToString() const {
    return "";
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8StringCR CommonTableBlockExp::_GetId() const {
    if (GetAlias().empty())
        return m_name;

    return GetAlias();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableBlockExp::_ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const {
    auto ctb = [&]() -> CommonTableBlockNameExp const* {
        if (ctx.CurrentArg()->GetType() != ECSqlParseContext::ParseArg::Type::RangeClass)
            return nullptr;
        auto rangeClasses = dynamic_cast<ECSqlParseContext::RangeClassArg const*>(ctx.CurrentArg());
        if (rangeClasses == nullptr)
            return nullptr;
        for(auto& rangeClass : rangeClasses->GetRangeClassInfos()) {
            if (rangeClass.GetExp().GetType() != Exp::Type::CommonTableBlockName)
                continue;
            auto cur = rangeClass.GetExp().GetAsCP<CommonTableBlockNameExp>();
            if (cur->GetName().EqualsIAscii(GetName())) {
                return cur;
            }
        }
        return nullptr;
    }();

    BeAssert(ctb != nullptr);
    if (ctb == nullptr) {
        return;
    }
    auto selection = GetQuery()->GetSelection();
    auto nCols = std::max(m_columnList.size(), selection->GetChildrenCount());
    for (auto i = 0; i < nCols; ++i) {
        auto target = selection->GetChildren().Get<DerivedPropertyExp>(i);
        auto property = std::make_unique<CommonTablePropertyNameExp>(m_columnList[i].c_str(), *target, [&](Utf8String col) {
            return FindType(col);
        }, ctb);
        expandedSelectClauseItemList.push_back(std::make_unique<DerivedPropertyExp>(std::move(property), nullptr));
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMatchResult CommonTableBlockExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const {
    if (!ExpandDerivedProperties()) {
        return PropertyMatchResult::NotFound();
    }
    auto path = propertyPath;
    if (path.Size() > 1) {
        if (path.First().GetName().EqualsIAscii(GetName()) || path.First().GetName().EqualsIAscii(options.GetAlias())) {
            path = path.Skip(1);
        } else {
            // No nested property access supported
            return PropertyMatchResult::NotFound();
        }
    }

    BeAssert(path.Size() == 1);
    if (path.Size() != 1)
        return PropertyMatchResult::NotFound();

    for (int i = 0; i < m_columnList.size(); ++i) {
        if (path.First().GetName().EqualsIAscii(m_columnList[i])) {
            const auto expIdx = i + 1;
            const auto derivedProp = expIdx < GetChildren().size() ? GetChildren().Get<DerivedPropertyExp>(expIdx) : nullptr;
            if (derivedProp != nullptr) {
                return PropertyMatchResult(options, propertyPath, propertyPath, *derivedProp, 0, true);
            }
        }
    }
    return PropertyMatchResult::NotFound();
}


//******************************** CommonTableExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
CommonTableExp::CommonTableExp(std::unique_ptr<SelectStatementExp> stmt, std::vector<std::unique_ptr<CommonTableBlockExp>> cteList, bool recursive)
    :Exp(Exp::Type::CommonTable), m_recursive(recursive) {
    for( auto& cte : cteList ) {
        AddChild(std::move(cte));
    }
    AddChild(std::move(stmt));
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus CommonTableExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
     if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren) {
        return FinalizeParseStatus::Completed;
    }
    
    BeAssert(false && "Programmer Error");
    return FinalizeParseStatus::Error;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql("WITH ");
    if (m_recursive)
        ctx.AppendToECSql("RECURSIVE ");

    auto blocks = GetCteList();
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (i > 0) {
            ctx.AppendToECSql(", ");
        }
        blocks[i]->ToECSql(ctx);
    }

    this->GetQuery()->ToECSql(ctx);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String CommonTableExp::_ToString() const {
    return "";    
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
std::vector<CommonTableBlockExp const*> CommonTableExp::GetCteList() const {
    std::vector<CommonTableBlockExp const *> list;
    for (auto child : GetChildren()) {
        if (child->GetType() != Exp::Type::CommonTableBlock)
            continue;
        
        list.push_back(child->GetAsCP<CommonTableBlockExp>());
    }
    return list;
}
//***********************************CommonTableBlockNameExp*******************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
const CommonTableBlockExp*  CommonTableBlockNameExp::ResolveBlock(ECSqlParseContext const& ctx, bool logError) const {
    if (m_blockExp) {
        return m_blockExp;
    }

    if (auto exp  = FindParent(Exp::Type::CommonTable)) {
        auto& cte = exp->GetAs<CommonTableExp>();
        auto blocks = cte.GetCteList();
        for (auto block : blocks) {
            if (block->GetName().EqualsIAscii(this->GetName()) ) {
                // set reference to match CTE block name
                m_blockExp = block;
            }
        }
        if (m_blockExp == nullptr) {
            if (logError) {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Invalid expression : Unable to find a common table expression block with '%s' name.", ToECSql().c_str());
            }
        }
    } else {
        if (logError) {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Invalid ECSQL class expression '%s': Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]", ToECSql().c_str());
        }
    }
    return m_blockExp;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus CommonTableBlockNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren) {
        if (ResolveBlock(ctx, true) == nullptr) {
            return FinalizeParseStatus::Error;
        }

        return FinalizeParseStatus::Completed;
    }
    
    BeAssert(false && "Programmer Error");
    return FinalizeParseStatus::Error;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8StringCR CommonTableBlockNameExp::_GetId() const {
    if (GetAlias().empty())
        return m_name;

    return GetAlias();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableBlockNameExp::_ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const {
    auto blockExp = ResolveBlock(ctx, false);
    BeAssert(blockExp != nullptr && "Programmer Error, this should be set");
    if (blockExp) {
        return blockExp->ExpandSelectAsterisk(expandedSelectClauseItemList, ctx);
    }
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMatchResult CommonTableBlockNameExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const {
    auto blockExp = ResolveBlock(ctx, false);
    BeAssert(blockExp != nullptr && "Programmer Error, this should be set");
    if (blockExp) {
        PropertyMatchOptions overrideOptions = options;
        if (!GetAlias().empty()) {
            overrideOptions.SetAlias(GetAlias().c_str());
        }
        return blockExp->FindProperty(ctx, propertyPath, overrideOptions);
    }

    return PropertyMatchResult::NotFound();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableBlockNameExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql(m_name);
    if (!GetAlias().empty())
        ctx.AppendToECSql(" ").AppendToECSql(GetAlias());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String CommonTableBlockNameExp::_ToString () const {
    return m_name;
}
//*********************************************CommonTablePropertyNameExp*******************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus CommonTablePropertyNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
     if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren) {
        if (!GetTarget().GetExpression()->GetTypeInfo().IsNull())
            SetTypeInfo(GetTarget().GetExpression()->GetTypeInfo());
        else {
            auto typeInfo = m_typeInfoCallBack(m_name);
            if(typeInfo.GetKind() == ECSqlTypeInfo::Kind::Unset) {
                ctx.SetDeferFinalize(true);
                return FinalizeParseStatus::NotCompleted;
            }

            SetTypeInfo(typeInfo);
        }
        return FinalizeParseStatus::Completed;
    }
    
    BeAssert(false && "Programmer Error");
    return FinalizeParseStatus::Error;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
