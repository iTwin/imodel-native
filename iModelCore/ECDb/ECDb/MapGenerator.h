/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapGenerator.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


typedef bool (*ECNodeFilterCallback)   (ECNodeCR node);
typedef bool (*ECNodeCompareCallback)   (ECNodeP node1, ECNodeP node2);

typedef bool (*ECClusterFilterCallback)(ECClusterCR cluster);
typedef bool (*ECClusterCompareCallback)(ECClusterP node1, ECClusterP node2);

typedef bmap<ECClassCP, ECNodePtr> NodeMapByECClass;

enum ECClusterType
    {
    ECCLUSTERTYPE_Unknown,
    ECCLUSTERTYPE_CustomAttributes,
    ECCLUSTERTYPE_NonCyclic,
    ECCLUSTERTYPE_MaximumConnectivity,
    ECCLUSTERTYPE_Relationships,
    ECCLUSTERTYPE_Loner,
    ECCLUSTERTYPE_Merge,
    ECCLUSTERTYPE_Struct,
    ECCLUSTERTYPE_UnmappableClasses,
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
*Allow a nullable values use for caching primitive values. Clear() will set it to null
*forcing the value to be re-evaluated.
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T> 
struct Nullable
    {
    private: 
        T m_value;
        bool m_null;
    private:
        void SetValue(T value) { m_null = false; m_value = value;}
    public:
        Nullable():m_null(true)   {}
        void operator = (T value) { SetValue (value);}
        bool    IsNull  ()        { return m_null;}
        T       GetValue()        {return m_value;}
        void    Clear   ()        { m_null =true;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECCluster : public RefCountedBase
    {
friend struct ECClusterList;
private:
    ECClusterListP   m_parent;
    Utf8String       m_name;
    NodeMapByECClass m_nodes; 
    int              m_noOfProperties;
    int              m_noOfColumns;
    bool             m_rebuildPropertyMap;
    ECClusterType    m_type;
    bmap<ECPropertyCP,ECClassCP> m_uniquePropertyMap;
private:
    void                Dettach() { m_parent = nullptr;}
                        ECCluster(ECClusterListR clusterList, Utf8CP name);
    static ECClusterPtr Create   (ECClusterListR clusterList, Utf8CP name);
    void                BuildUniquePropertyMap();
    void                AddToUniquePropertyMap(ECNodeR node, IECMapInfoProviderCR mapInfoProvider);
public:
    ECClusterListP  GetParent() {return m_parent;}
    void            SetName (Utf8CP name) { m_name = name;}
    Utf8StringCR    GetName ()const { return m_name;}
    bool            Add (ECNodeR node);
    bool            Remove (ECNodeR node);
    ECNodePtr       Find (ECClassCR ecClass);
    bool            Contains(ECNodeCR node);
    bool            Contains(ECClassCR ecClass);
    int             Size() { return (int)m_nodes.size();}
    void            Clear ();
    int             GetRootNodes(ECNodeListR rootNodes);
    int             GetLeafNodes(ECNodeListR leafNodes);
    int             GetNoOfUniqueProperties();
    int             GetNoOfUniqueColumns();
    void            GetProperties(ECPropertyListR properties);
    int             GetNoOfUniqueColumnsIfMergedWith(ECClusterR candiate);
    int             GetNoOfUniqueColumnsIfNodeAdded(ECNodeR node);
    void            MergeFrom(ECClusterR source);
    void            GetNodes(ECNodeListR nodes);
    void            SetType (ECClusterType type) { m_type = type; }
    ECClusterType   GetType () const { return m_type; }
    void            GetPropertiesWithDuplicateName(ECPropertyListR list);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClusterList : public RefCountedBase
    {
private:
    bvector<ECClusterPtr> m_clusters;
    ECGraphPtr            m_graph;
    ECClusterList(ECGraphR graph);
public:
    ECClusterP      Add     (Utf8CP name);
    bool            Remove  (ECClusterR cluster);
    ECClusterP      Find    (Utf8CP clusterName);
    ECClusterP      FindClusterWithNode (ECNodeCR node);
    int             Size    () {return (int)m_clusters.size();}
    ECClusterP      operator [](int index);
    int             GetNoOfUniqueProperties();
    int             GetNoOfUniqueColumns();
    ECGraphR        GetGraph() {return *m_graph;}
    static ECClusterListPtr Create(ECGraphR graph); 
    void            WriteDebugInfoToFile(Utf8String fileName);
    void            Select(bvector<ECClusterP>& clusters,  ECClusterFilterCallback where = nullptr, ECClusterCompareCallback orderby= nullptr);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECGraph: public RefCountedBase
    {
friend struct ECNode;
private:
    NodeMapByECClass                m_nodes;
    bvector<ECSchemaPtr>            m_schemas;
    int                             m_nUniqueProperties;
    int                             m_nProperties;
    IECMapInfoProviderPtr           m_propertyMapStrategyProvider;
private:
    void                 AddSchema (ECSchemaR schema);
    ECNodePtr            GetOrAddNode(ECClassCR ecClass);
    static void          Sort(ECNodeListR list,ECNodeCompareCallback criteria)
                             {
                             std::sort(list.begin(),list.end(), criteria);
                             }
                         ECGraph();
public:
    IECMapInfoProviderCR GetECMapInfoProvider()const { return *m_propertyMapStrategyProvider;}
    int                  GetUniquePropertyCount() const { return m_nUniqueProperties;}
    int                  GetPropertyCount() const { return m_nProperties;}
    ECNodePtr            FindNode (ECClassCR ecClass);
    void                 ClearClusterInformation();
    void                 Select (ECNodeListR nodeList, ECNodeFilterCallback where = nullptr, ECNodeCompareCallback orderby = nullptr);
    static ECGraphPtr    Create   (bvector<ECSchemaPtr>& schemaList, IECMapInfoProviderR provider);
    int                  Size(){return  (int)m_nodes.size();}
    ECNodeP              FindNodeWithMaximumNumberOfColumns();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECNode : public RefCountedBase
    {
friend struct ECCluster;
friend struct ECGraph;

private:
    ECClassCR       m_class; 
    ECClassId       m_classId;
    ECClusterP      m_cluster;
    ECGraphR        m_graph;
    int             m_noOfLocalProperties;
    int             m_noOfProperties;
    int             m_noOfLocalColumns;
    int             m_noOfColumns;
    ECNodeList      m_derivedNodes;
    ECNodeList      m_baseNodes;
    ECPropertyList  m_properties;
private:
    void             SetCluster(ECClusterP cluster){ m_cluster = cluster;}
                     ECNode (ECGraphR graph, ECClassCR ecClass);
    static ECNodePtr Create (ECGraphR graph, ECClassCR eclass);
    bool             Init ();
public:
    ECClassCR        GetClass () const { return m_class;}
    ECClusterP       GetCluster () const { return m_cluster;}
    ECGraphR         GetGraph () const {return m_graph;}
    ECClassId        GetClassId() const {return m_classId;}
    ECNodeListCR     GetDerivedNodes() const { return m_derivedNodes;}
    ECNodeListCR     GetBaseNodes() const { return m_baseNodes;}
    int              GetLocalPropertyCount() const {return m_noOfLocalProperties; }
    int              GetPropertyCount() const {return m_noOfProperties; }
    int              GetLocalColumnCount() const {return m_noOfLocalColumns; }
    int              GetColumnCount() const {return m_noOfColumns; }
    bool             CanStoreData() const{ return m_noOfProperties > 0;}
    int              GetConnectivity() const  { return (int)(m_baseNodes.size() + m_derivedNodes.size()); }
    bool             HasMoreBaseClassesThanDerivedClasses() const { return m_baseNodes.size()> m_derivedNodes.size();}
    ECPropertyListCR GetProperties() const { return m_properties;}
    };

struct IECMapInfoProvider : public RefCountedBase
    {
public:
    virtual int  GetNumberOfColumnUseToStoreProperty(ECPropertyCR property) const =0;
    };

struct DefaultECMapInfoProvider : public IECMapInfoProvider
    {
private:
    DefaultECMapInfoProvider(){}
public:
    virtual int  GetNumberOfColumnUseToStoreProperty(ECPropertyCR property) const override
        {
        if (property.GetIsPrimitive())
            {
                PrimitiveECPropertyCP p = property.GetAsPrimitiveProperty();
                switch(p->GetType())
                    {
                case PRIMITIVETYPE_Point2D:
                    return 2;
                case PRIMITIVETYPE_Point3D:
                    return 3;
                case PRIMITIVETYPE_IGeometry:
                    return 0; //we don't handle this
                default:
                    return 1; //use one column for all other primitive types
                    }
            }
        else if (property.GetIsStruct())
            {
            return 0; //store in separate table
            }
        else if (property.GetIsArray())
            {
            ArrayECPropertyCP p = property.GetAsArrayProperty();
            if (p->GetKind() == ARRAYKIND_Primitive)
                return 1; //BLOB
            else  // ArrayKind.ARRAYKIND_Struct
                return 0; //store in separate table
            }
        return 0; //we don't store it at all.
        }
    static IECMapInfoProviderPtr Create()
        {
        return new DefaultECMapInfoProvider();
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClusteringAlgorithmConfiguration
    {
    private:
        int m_maxPropertiesPerTable;
    public:
        int GetMaxColumnsPerTable() const { return m_maxPropertiesPerTable; }
        void SetMaxPropertiesPerTable(int properties) { m_maxPropertiesPerTable = properties; }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ClusteringAlgorithm : public RefCountedBase
    {
protected:
    virtual ECClusterListPtr _Cluster (ClusteringAlgorithmConfigurationCR config, ECGraphR graph) = 0;
    virtual Utf8CP           _GetName () const = 0;

public:
    ECClusterListPtr Cluster(ClusteringAlgorithmConfigurationCR config, ECGraphR graph) { return _Cluster(config, graph); }
    Utf8CP GetName() const { return _GetName(); }    


    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultClustringAlgorithm : public ClusteringAlgorithm
    {
private:
    ClusteringAlgorithmConfigurationCP m_config;
    Utf8String m_name;
private:
    void ClusterCustomAttributes         (ECClusterListR clusterList, ECGraphR graph);
    void ClusterNonCyclicClasses         (ECClusterListR clusterList, ECGraphR graph);
    void ClusterByHighConnectivity       (ECClusterListR clusterList, ECGraphR graph);
    void ClusterLonerClasses             (ECClusterListR clusterList, ECGraphR graph);
    void MergeClusters                   (ECClusterListR clusterList, ECGraphR graph);
    void ClusterNonDomainAndEmptyClasses (ECClusterListR clusterList, ECGraphR graph);
    void ClusterRelationships            (ECClusterListR clusterList, ECGraphR graph);
    void ClusterStructs                  (ECClusterListR clusterList, ECGraphR graph);
    void ClusterClassesWithExistingHints (ECClusterListR clusterList, ECGraphR graph);
    void AddAllDerivedClassesToCluster   (ECNodeR node, ECClusterR cluster);
    bool TryAddToABaseCluster                   (ECNodeR node);
    bool TryAddToADerivedCluster                (ECNodeR node);
    bool TryAddToASiblingCluster                (ECNodeR node);
    bool TrySelectAClusterWithMinimumProperties (bvector<ECClusterP>& clusters, ECNodeR n);
private:
    static bool FilterByNonMappedClasses (ECNodeCR node) 
        {
        return 
             node.GetCluster() == nullptr                      &&
            !node.GetClass().GetIsCustomAttributeClass()    &&
            !node.GetClass().GetIsStruct()                  &&
            ((!node.CanStoreData() &&  node.GetClass().GetRelationshipClassCP() == nullptr) ||
               (!node.GetClass().GetIsDomainClass() && node.GetClass().GetRelationshipClassCP() != nullptr));
        }
    static bool FilterByECDbHint (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && (
            node.GetClass().GetCustomAttributeLocal(BSCAC_ECDbClassHint).IsValid() || 
            node.GetClass().GetCustomAttributeLocal (BSCAC_ECDbRelationshipClassHint).IsValid());
        }
    static bool FilterByOrphanNodes (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr;
        }
    static bool FilterByStruct (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetClass().GetIsStruct() ;
        }
    static bool FilterByRelationshipClassess (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetClass().GetRelationshipClassCP() != nullptr ;
        }
    static bool FilterByCustomAttributes (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetClass().GetIsCustomAttributeClass() ;
        }
    static bool FilterByClassesWithNoBaseOrDerivedClass (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetBaseNodes().size() == 0 && node.GetDerivedNodes().size() == 0;
        }
    static bool FilterByClassThatHasExactlyOneBaseClass (ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetBaseNodes().size() == 1;
        }
    static bool FilterByClassesWithConnectivityGreaterThanZero(ECNodeCR node) 
        {
        return node.GetCluster() == nullptr && node.GetConnectivity() > 0;
        }
    static bool OrderByNumberOfDerivedClasses(ECNodeP node1, ECNodeP node2) 
        {
        return node1->GetDerivedNodes().size() < node2->GetDerivedNodes().size();
        }
    static bool OrderByDescendingNumberOfDerivedClasses(ECNodeP node1, ECNodeP node2) 
        {
        return node1->GetDerivedNodes().size() > node2->GetDerivedNodes().size();
        }
    static bool OrderByDescendingConnectivity(ECNodeP node1, ECNodeP node2) 
        {
        return node1->GetConnectivity() > node2->GetConnectivity();
        }
    static bool OrderByClusterWithMinmumProperties(ECClusterP cluster1, ECClusterP cluster2) 
        {
        return cluster1->GetNoOfUniqueProperties() < cluster2->GetNoOfUniqueProperties();
        }
    static bool OrderByBySizeOfCluster(ECClusterP node1, ECClusterP node2) 
        {
        return node1->GetNoOfUniqueColumns() < node2->GetNoOfUniqueColumns();
        }
    static bool FilterClusterByType(ECClusterCR node) 
        {
        return  node.GetType() != ECCLUSTERTYPE_Loner && 
                node.GetType() != ECCLUSTERTYPE_CustomAttributes &&
                node.GetType() != ECCLUSTERTYPE_UnmappableClasses&&
                node.GetType() != ECCLUSTERTYPE_Relationships;
        }

protected :
    virtual ECClusterListPtr _Cluster (ClusteringAlgorithmConfigurationCR config, ECGraphR graph) override;
    virtual Utf8CP           _GetName () const override;

    DefaultClustringAlgorithm();
public:
    static ClusteringAlgorithmPtr Create()
        {
        return new DefaultClustringAlgorithm();
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct MapGenerator
    {
private:
    bmap<ECSchemaCP, ECSchemaPtr> m_supplementalSchemas;
    ECSchemaPtr                   m_bscaSchema;
private:
    BentleyStatus          FindAndSetBSCASchema          (ECDbSchemaManagerCR sm, bvector<ECSchemaPtr>& cache );
    ECSchemaPtr            GetOrAddSupplementalSchema    (ECSchemaCR primarySchema);
    ECClassP               GetOrAddSupplementalClass     (ECClassCR classInPrimarySchema);
    ECPropertyP            GetOrAddSupplementalProperty  (ECPropertyCR propertyInPrimarySchema, ECClassR supplementalClass);
    //void                   WriteDebug("")
    static ECObjectsStatus CreateSupplementalSchema (ECSchemaPtr& supplementalSchema, ECSchemaCR primarySchema, ECSchemaCR bscaSchema);

public:
    void AnnotateECSchemas (bvector<ECSchemaPtr>& schemaList, ECDbSchemaManagerCR sm);
    void GetSupplementalSchemas(bvector<ECSchemaPtr>& schemas);

    };


END_BENTLEY_SQLITE_EC_NAMESPACE

