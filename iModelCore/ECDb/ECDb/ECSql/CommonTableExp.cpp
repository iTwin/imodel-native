/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** CommonTablePropertyNameExp *************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTablePropertyNameExp::_ToJson(BeJsValue val, JsonFormat const&) const {
    //! ITWINJS_PARSE_TREE: CommonTablePropertyNameExp
    val.SetEmptyObject();
    val["id"] = "PropertyNameExp";
    Utf8String path;
    path.append(!m_blockName->GetAlias().empty()? m_blockName->GetAlias() : m_blockName->GetName()).append(".").append(m_name);
    val["path"] = path;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String CommonTablePropertyNameExp::_ToString() const {
    Utf8String path;
    path.append(!m_blockName->GetAlias().empty()? m_blockName->GetAlias() : m_blockName->GetName()).append(".").append(m_name);
    return path;
}

//****************************** CommonTableBlockExp *************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
CommonTableBlockExp::CommonTableBlockExp(Utf8CP name, std::vector<Utf8String> colList, std::unique_ptr<SelectStatementExp> stmt)
    :RangeClassRefExp(Exp::Type::CommonTableBlock, PolymorphicInfo::Only()), m_name(name), m_columnList(colList),m_deferredExpand(true) {
    AddChild(std::move(stmt));
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
CommonTableBlockExp::CommonTableBlockExp(Utf8CP name, std::unique_ptr<SelectStatementExp> stmt)
    :RangeClassRefExp(Exp::Type::CommonTableBlock, PolymorphicInfo::Only()), m_name(name), m_deferredExpand(true) {
    AddChild(std::move(stmt));
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool CommonTableBlockExp::ExpandDerivedProperties(ECSqlParseContext& ctx) const {
    if(m_columnList.size() == 0)
        return ExpandDerivedPropertiesForEmptyColumnList(ctx);
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
bool CommonTableBlockExp::ExpandDerivedPropertiesForEmptyColumnList(ECSqlParseContext& ctx) const {
    // when we encounter wild card we will leave it deferred.
    if (!m_deferredExpand) {
        return true;
    }
    auto query = GetQuery();
    auto cols = query->GetSelection()->GetChildrenCount();
    for (auto i = 0; i < cols; ++i) {
        auto target = query->GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
        if (target->IsWildCard()) {
            return false;
        }
    }
    for (Exp const* expr : GetQuery()->GetSelection()->GetChildren())
        {
        DerivedPropertyExp const& selectClauseItemExp = expr->GetAs<DerivedPropertyExp>();
        std::unique_ptr<PropertyNameExp> propNameExp = std::make_unique<PropertyNameExp>(ctx, *this, selectClauseItemExp); // we here set the clasref as CommonTableBlockExp
        const_cast<CommonTableBlockExp*>(this)->AddChild(std::make_unique<DerivedPropertyExp>(std::move(propNameExp), nullptr));
        }
    m_deferredExpand = false;
    return !m_deferredExpand;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus CommonTableBlockExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    ExpandDerivedProperties(ctx);
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    if (mode == Exp::FinalizeParseMode::AfterFinalizingChildren) {
        // Make sure columns in CTE Block are unique
        std::set<Utf8CP, CompareIUtf8Ascii> uniqueCols;
        for (auto& col : m_columnList) {
            auto it = uniqueCols.insert(col.c_str());
            if (false == it.second) {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0452,
                    "Invalid ECSql : Common table '%s' has duplicate column with same name '%s'. %s", GetName().c_str(), col.c_str(), ToECSql().c_str());
                return FinalizeParseStatus::Error;
            }
        }

        // The column and value count must match. This is a differed error from parsing.
        const auto columns = GetColumns().size();
        const auto values = GetQuery()->GetSelection()->GetChildrenCount();
        if (columns != values && m_columnList.size() != 0) {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0453,
                "Invalid ECSql : Common table '%s' has %d values for columns %d. %s", GetName().c_str(), columns, values, ToECSql().c_str());
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
//+---------------+---------------+---------------+---------------+---------------+------
void CommonTableBlockExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: CommonTableBlockExp
    val.SetEmptyObject();
    val["id"] = "CommonTableBlockExp";
    val["name"] = m_name;
    if (!GetColumns().empty()) {
        auto args = val["args"];
        args.toArray();
        for(auto& col : GetColumns())
            args.appendValue() = col;
    }
    GetQuery()->ToJson(val["asQuery"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CommonTableBlockExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql(GetName());
    if (!GetColumns().empty())
        ctx.AppendToECSql("(").AppendToECSql(GetColumns().front());

    for (size_t i = 1; i < this->GetColumns().size(); ++i) {
        ctx.AppendToECSql(", ").AppendToECSql(GetColumns().at(i));
        if(i == GetColumns().size()-1)
            ctx.AppendToECSql(")");
    }
    ctx.AppendToECSql(" AS (");
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
    if(m_columnList.size() == 0){
        for (Exp const* expr : GetQuery()->GetSelection()->GetChildren())
        {
        DerivedPropertyExp const& selectClauseItemExp = expr->GetAs<DerivedPropertyExp>();
        std::unique_ptr<PropertyNameExp> propNameExp = std::make_unique<PropertyNameExp>(ctx, *this, selectClauseItemExp); // we here set the clasref as CommonTableBlockExp
        expandedSelectClauseItemList.push_back(std::make_unique<DerivedPropertyExp>(std::move(propNameExp), nullptr));
        }
    }
    else
    {
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
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMatchResult CommonTableBlockExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const &propertyPath, const PropertyMatchOptions &options) const {
    if(m_columnList.size() == 0){
        if(Utf8String::IsNullOrEmpty(options.GetAlias().c_str()))
        {
            /*This is added because for cte blocks without columns we treat the block select statement as a subquery
            now if the cte block name has alias then that alias is respected otherwise the cte block name
            itself can also be used while referencing properties in outside select statement.
            ex:- with cte as (select * from meta.ECClassDef) select cte.ECInstanceId from cte;*/
            PropertyMatchOptions overrideOptions = options;
            overrideOptions.SetAlias(m_name.c_str());
            return GetQuery()->FindProperty(ctx, propertyPath, overrideOptions);
        }
        return GetQuery()->FindProperty(ctx, propertyPath, options);
    }
    if (!ExpandDerivedProperties(ctx)) {
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
//+---------------+---------------+---------------+---------------+---------------+------
void CommonTableExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: CommonTableExp
    val.SetEmptyObject();
    val["id"] = "CommonTableExp";
    val["recursive"] = Recursive();
    auto blocks = val["blocks"];
    blocks.toArray();
    for(auto& cteList : GetCteList())
        cteList->ToJson(blocks.appendValue(), fmt);

    GetQuery()->ToJson(val["select"], fmt);
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
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0454,
                    "Invalid expression : Unable to find a common table expression block with '%s' name.", ToECSql().c_str());
            }
        }
    } else {
        if (logError) {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0455,
                "Invalid ECSQL class expression '%s': Valid syntax: [<table space>.]<schema name or alias>.<class name>[.function call]", ToECSql().c_str());
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
//+---------------+---------------+---------------+---------------+---------------+------
void CommonTableBlockNameExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: CommonTableBlockNameExp
    val.SetEmptyObject();
    val["id"] = "CommonTableBlockNameExp";
    val["name"] = m_name;
    if(!GetAlias().empty())
        val["alias"] = GetAlias();
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
        if (auto targetTypeInfo = GetTarget().GetExpression()->GetTypeInfo(); !targetTypeInfo.IsNull() && !targetTypeInfo.IsUnset())
            SetTypeInfo(targetTypeInfo);
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
