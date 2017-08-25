/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/NavNodesDataSource.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/IECPresentationManager.h>
#include <ECPresentationRules/RelatedInstanceNodesSpecification.h>
#include "RulesEngineTypes.h"
#include "NavNodeProviders.h"
#include "DataSourceInfo.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                03/2017
+===============+===============+===============+===============+===============+======*/
struct DataSourceFilter
{
    struct RelatedInstanceInfo
        {
        bset<ECN::ECClassId> m_relationshipClassIds;
        BeSQLite::EC::ECInstanceId m_instanceId;
        ECN::RequiredRelationDirection m_direction;
        RelatedInstanceInfo() {}
        RelatedInstanceInfo(bset<ECN::ECClassId> relationshipClassIds, ECN::RequiredRelationDirection direction, BeSQLite::EC::ECInstanceId id)
            : m_relationshipClassIds(relationshipClassIds), m_direction(direction), m_instanceId(id)
            {}
        bool IsValid() const {return !m_relationshipClassIds.empty() && m_instanceId.IsValid();}
        };

private:
    RelatedInstanceInfo const* m_relatedInstanceInfo;
    Utf8String m_instanceFilter;

private:
    void InitFromJson(RapidJsonValueCR json);

public:
    DataSourceFilter() : m_relatedInstanceInfo(nullptr) {}
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter const&);
    ECPRESENTATION_EXPORT DataSourceFilter(DataSourceFilter&&);
    DataSourceFilter(RapidJsonValueCR json) : m_relatedInstanceInfo(nullptr) {InitFromJson(json);}
    DataSourceFilter(Utf8CP instanceFilter) : m_instanceFilter(instanceFilter) {}
    ECPRESENTATION_EXPORT DataSourceFilter(RelatedInstanceInfo const&, Utf8CP instanceFilter);
    ~DataSourceFilter() {DELETE_AND_CLEAR(m_relatedInstanceInfo);}
    DataSourceFilter& operator=(DataSourceFilter const&);
    DataSourceFilter& operator=(DataSourceFilter&&);
    rapidjson::Document AsJson() const;
    RelatedInstanceInfo const* GetRelatedInstanceInfo() const {return m_relatedInstanceInfo;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                02/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE INavNodesDataSource : IDataSource<NavNodeCPtr>
{
protected:
    virtual NavNodePtr _GetNode(size_t index) const = 0;
    NavNodeCPtr _Get(size_t index) const override {return _GetNode(index);}
public:
    NavNodePtr GetNode(size_t index) const {return _GetNode(index);}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                09/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE NavNodesDataSource : INavNodesDataSource
{
private:
    NavNodesProviderCPtr m_nodesProvider;

private:
    NavNodesDataSource(NavNodesProviderCR nodesProvider) : m_nodesProvider(&nodesProvider) {}

protected:
    ECPRESENTATION_EXPORT NavNodePtr _GetNode(size_t index) const override;
    ECPRESENTATION_EXPORT size_t _GetSize() const override;

public:
    static NavNodesDataSourcePtr Create(NavNodesProviderCR nodesProvider) {return new NavNodesDataSource(nodesProvider);}
    JsonNavNodePtr GetNode(size_t index) const;
    NavNodesProviderCPtr GetProvider() const {return m_nodesProvider;}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE PagingDataSource : INavNodesDataSource
{
private:
    INavNodesDataSourceCPtr m_source;
    size_t m_pageStart;
    size_t m_pageSize;

private:
    PagingDataSource(INavNodesDataSourceCR source, size_t pageStart, size_t pageSize) 
        : m_source(&source), m_pageStart(pageStart), m_pageSize(pageSize)
        {}

protected:
    ECPRESENTATION_EXPORT NavNodePtr _GetNode(size_t index) const override;
    ECPRESENTATION_EXPORT size_t _GetSize() const override;

public:
    static PagingDataSourcePtr Create(INavNodesDataSourceCR source, size_t pageStart, size_t pageSize)
        {
        return new PagingDataSource(source, pageStart, pageSize);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE EmptyNavNodesDataSource : INavNodesDataSource
{
private:
    EmptyNavNodesDataSource() {}
protected:
    NavNodePtr _GetNode(size_t index) const override {return nullptr;}
    size_t _GetSize() const override {return 0;}
public:
    static EmptyNavNodesDataSourcePtr Create() {return new EmptyNavNodesDataSource();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
