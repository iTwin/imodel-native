/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../Shared/Queries/PresentationQuery.h"
#include "../Shared/ExtendedData.h"
#include "NavigationQueryContracts.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryResultParameters
{
private:
    ChildNodeSpecificationCP m_specification;
    NavNodeExtendedData m_navNodeExtendedData;
    NavigationQueryResultType m_resultType;
    std::unordered_set<ECRelationshipClassCP> m_usedRelationshipClasses;
    std::unordered_set<ECClassCP> m_selectInstanceClasses;

public:
    NavNodeExtendedData& GetNavNodeExtendedDataR() {return m_navNodeExtendedData;}
    void SetResultType(NavigationQueryResultType type) {m_resultType = type;}
    void SetSpecification(ChildNodeSpecificationCP spec) {m_specification = spec;}
    std::unordered_set<ECRelationshipClassCP>& GetUsedRelationshipClasses() {return m_usedRelationshipClasses;}
    std::unordered_set<ECClassCP>& GetSelectInstanceClasses() {return m_selectInstanceClasses;}
    void OnContractSelected(NavigationQueryContractCR contract) {m_resultType = contract.GetResultType();}
    ECPRESENTATION_EXPORT void MergeWith(NavigationQueryResultParameters const&);

public:
    NavigationQueryResultParameters()
        : m_resultType(NavigationQueryResultType::Invalid), m_specification(nullptr)
        {}
    NavigationQueryResultParameters(NavigationQueryResultParameters const& other)
        : m_navNodeExtendedData(other.m_navNodeExtendedData), m_specification(other.m_specification), m_resultType(other.m_resultType),
        m_usedRelationshipClasses(other.m_usedRelationshipClasses), m_selectInstanceClasses(other.m_selectInstanceClasses)
        {}
    NavigationQueryResultParameters(NavigationQueryResultParameters&& other)
        : m_navNodeExtendedData(std::move(other.m_navNodeExtendedData)), m_specification(other.m_specification), m_resultType(other.m_resultType),
        m_usedRelationshipClasses(std::move(other.m_usedRelationshipClasses)), m_selectInstanceClasses(std::move(other.m_selectInstanceClasses))
        {}
    NavigationQueryResultParameters& operator=(NavigationQueryResultParameters const& other)
        {
        m_navNodeExtendedData = other.m_navNodeExtendedData;
        m_specification = other.m_specification;
        m_resultType = other.m_resultType;
        m_usedRelationshipClasses = other.m_usedRelationshipClasses;
        m_selectInstanceClasses = other.m_selectInstanceClasses;
        return *this;
        }
    NavigationQueryResultParameters& operator=(NavigationQueryResultParameters&& other)
        {
        m_specification = other.m_specification;
        m_resultType = other.m_resultType;
        m_navNodeExtendedData = std::move(other.m_navNodeExtendedData);
        m_usedRelationshipClasses = std::move(other.m_usedRelationshipClasses);
        m_selectInstanceClasses = std::move(other.m_selectInstanceClasses);
        return *this;
        }
    ECPRESENTATION_EXPORT bool operator==(NavigationQueryResultParameters const& other) const;
    NavNodeExtendedData const& GetNavNodeExtendedData() const {return m_navNodeExtendedData;}
    NavigationQueryResultType GetResultType() const {return m_resultType;}
    ChildNodeSpecificationCP GetSpecification() const {return m_specification;}
    std::unordered_set<ECRelationshipClassCP> const& GetUsedRelationshipClasses() const {return m_usedRelationshipClasses;}
    std::unordered_set<ECClassCP> const& GetSelectInstanceClasses() const {return m_selectInstanceClasses;}
    };

#define NAVIGATIONQUERY_EXTENDEDDATA_Ranges     "Ranges"
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct NavigationQueryExtendedData : RapidJsonAccessor
    {
    NavigationQueryExtendedData(RapidJsonValueCR data) : RapidJsonAccessor(data) {}
    NavigationQueryExtendedData(PresentationQueryBuilder& query) : RapidJsonAccessor(query) {}
    NavigationQueryExtendedData(PresentationQueryBuilder const& query) : RapidJsonAccessor(query) {}

    bool HasRangesData() const {return GetJson().HasMember(NAVIGATIONQUERY_EXTENDEDDATA_Ranges);}
    int GetRangeIndex(BeSQLite::DbValue const&) const;
    Utf8String GetRangeLabel(int rangeIndex) const;
    Utf8CP GetRangeImageId(int rangeIndex) const;
    ECPRESENTATION_EXPORT void AddRangesData(ECPropertyCR, PropertyGroupCR);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
