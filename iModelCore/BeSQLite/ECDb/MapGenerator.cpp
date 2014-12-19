/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapGenerator.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbMapLog.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECGraphPtr ECGraph::Create (bvector<ECSchemaPtr>& schemaList, IECMapInfoProviderR provider)
    {
    ECGraphPtr graph = new ECGraph();
    
    graph->m_propertyMapStrategyProvider = &provider;
   
    FOR_EACH (ECSchemaPtr schema, schemaList)
      graph->AddSchema(*schema);
    
    return graph;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNodeP ECGraph::FindNodeWithMaximumNumberOfColumns()
    {
    ECNodeP node = nullptr;
    int maxColumns = 0;
    for( NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        if (it->second->GetColumnCount() > maxColumns)
            {
            maxColumns= it->second->GetColumnCount();
            node = it->second.get();
            }
    return node;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECGraph::AddSchema (ECSchemaR schema)
    {
    FOR_EACH (ECSchemaPtr alreadyAddedSchema, m_schemas)
        if ( alreadyAddedSchema.get() == &schema)
            return;

    m_schemas.push_back(&schema);

    FOR_EACH (ECClassCP ecClass, schema.GetClasses())
        {
        ECNodePtr node = GetOrAddNode(*ecClass);
        BeAssert (node.IsValid());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNodePtr ECGraph::GetOrAddNode (ECClassCR ecClass)
    {
    ECNodePtr node = FindNode(ecClass);
    if (node.IsNull())
        {
        node = ECNode::Create (*this, ecClass);
        if (node.IsValid())
            {
            m_nodes[&ecClass] = node;
            node->Init();
            m_nProperties += node->GetPropertyCount();
            m_nUniqueProperties += node->GetLocalPropertyCount();
            }
        }
    return node;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECGraph::ECGraph()
    : m_nProperties(0), m_nUniqueProperties(0)
    { 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNodePtr ECGraph::FindNode (ECClassCR ecClass)
    {
    NodeMapByECClass::const_iterator it = m_nodes.find(&ecClass);
    if(it == m_nodes.end())
        return nullptr;
    return it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECGraph::Select (ECNodeListR nodeList, ECNodeFilterCallback where, ECNodeCompareCallback orderby)
    {
    if (where == nullptr)
        for( NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
            nodeList.push_back(it->second.get());
    else
        for( NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
            {
            if (where(*it->second) == true)
                nodeList.push_back(it->second.get());
            }
    if (orderby != nullptr)
        Sort (nodeList, orderby);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECGraph::ClearClusterInformation()
    {
    ECClusterP cluster;
    for( NodeMapByECClass::iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        if ((cluster = it->second->GetCluster()) != nullptr)
            cluster->Remove (*it->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNode::ECNode (ECGraphR graph, ECClassCR ecClass)
    :m_graph(graph), m_class(ecClass),m_noOfProperties(0), m_noOfLocalProperties(0), m_noOfLocalColumns(0), m_noOfColumns(0),m_cluster(nullptr)
    {
    m_classId = ecClass.GetId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNode::Init()
    {
    ECNodePtr n;
    bool      r = true;

    FOR_EACH (ECClassCP baseClass, GetClass().GetBaseClasses())
        if ((n = GetGraph().GetOrAddNode(*baseClass)).IsValid())
            m_baseNodes.push_back(n.get());

    FOR_EACH (ECClassCP derivedClass, GetClass().GetDerivedClasses())
        if ((n = GetGraph().GetOrAddNode(*derivedClass)).IsValid())
            m_derivedNodes.push_back(n.get());

    IECMapInfoProviderCR mapProvider = GetGraph().GetECMapInfoProvider() ;

    ECPropertyIterable localProperties = GetClass().GetProperties(false);
    for (ECPropertyIterable::const_iterator itor = localProperties.begin(); 
        itor != localProperties.end(); ++itor, m_noOfLocalProperties++)
        m_noOfLocalColumns += mapProvider.GetNumberOfColumnUseToStoreProperty (**itor);
    
    ECPropertyIterable allProperties = GetClass().GetProperties(true);
    for (ECPropertyIterable::const_iterator itor = allProperties.begin(); 
        itor != allProperties.end(); ++itor, m_noOfProperties++)
        {
        m_noOfColumns += mapProvider.GetNumberOfColumnUseToStoreProperty (**itor);
        m_properties.push_back(*itor);
        }

    return r;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNodePtr ECNode::Create (ECGraphR graph, ECClassCR ecClass)
    {
    return  new ECNode(graph, ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECCluster::ECCluster(ECClusterListR clusterList, Utf8CP name)
    : m_parent(&clusterList), m_name(name), m_rebuildPropertyMap(true), m_noOfColumns(0), m_noOfProperties(0), m_type(ECCLUSTERTYPE_Unknown)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterPtr ECCluster::Create(ECClusterListR clusterList, Utf8CP name)
    {
    ECClusterPtr cluster = new ECCluster(clusterList, name);
    //init if any
    return cluster;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCluster::Add (ECNodeR node)
    {
    if (node.GetCluster() != nullptr)
        {
        BeAssert(node.GetCluster() != nullptr);
        return false;
        }
    BeAssert(Contains(node) != true);
    node.SetCluster (this);
    m_nodes[&node.GetClass()] = &node;
    AddToUniquePropertyMap (node, GetParent()->GetGraph().GetECMapInfoProvider());
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECNodePtr ECCluster::Find(ECClassCR ecClass)
    {
    NodeMapByECClass::const_iterator it = m_nodes.find(&ecClass);
    if (it == m_nodes.end())
        return nullptr;
    return it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::Clear()
    {
    m_uniquePropertyMap.clear();
    m_rebuildPropertyMap =true;
    for( NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        it->second->SetCluster(nullptr);
    m_nodes.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCluster::Contains(ECClassCR ecClass)
    {
    return Find(ecClass).IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCluster::Contains (ECNodeCR node)
    {
    return Contains (node.GetClass());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECCluster::Remove (ECNodeR node)
    {
    BeAssert(node.GetCluster() == this);
    if (node.GetCluster() != this)
        return false;
    node.SetCluster(nullptr);
    m_rebuildPropertyMap = true;
    return m_nodes.erase (&node.GetClass()) > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetRootNodes(ECNodeListR rootNodes)
    {
    rootNodes.clear();
    for (NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        {
        bool foundBaseClassInThisCluster = false;
        FOR_EACH (ECNodeP baseNode, it->second->GetBaseNodes())
            if (Find(baseNode->GetClass()).IsValid())
                {
                foundBaseClassInThisCluster = true;
                break;
                }

        if (!foundBaseClassInThisCluster)
            rootNodes.push_back(it->second.get());
        }
    return (int)rootNodes.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetLeafNodes(ECNodeListR leafNodes)
    {
    leafNodes.clear();
    for (NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        {
        bool foundDerivedClassInThisCluster = false;
        FOR_EACH (ECNodeP baseNode, it->second->GetBaseNodes())
            if (Find(baseNode->GetClass()).IsValid())
                {
                foundDerivedClassInThisCluster = true;
                break;
                }

            if (!foundDerivedClassInThisCluster)
                leafNodes.push_back(it->second.get());
        }
    return (int)leafNodes.size();    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetNoOfUniqueProperties()
    {
    BuildUniquePropertyMap();
    return m_noOfProperties;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::BuildUniquePropertyMap()
    {
    if (!m_rebuildPropertyMap && m_uniquePropertyMap.size() > 0)
        return;
    //rebuild property table
    m_rebuildPropertyMap = false;
    IECMapInfoProviderCR mapInfoProvider = GetParent()->GetGraph().GetECMapInfoProvider();
    m_uniquePropertyMap.clear();
    m_noOfColumns = m_noOfProperties = 0;
    for (NodeMapByECClass::const_iterator it =  m_nodes.begin(); it != m_nodes.end(); it++)
        AddToUniquePropertyMap(*it->second, mapInfoProvider);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::AddToUniquePropertyMap(ECNodeR node, IECMapInfoProviderCR mapInfoProvider)
    {
    if(m_rebuildPropertyMap)
        BuildUniquePropertyMap();

    FOR_EACH (ECPropertyCP property, node.GetProperties())
        if (m_uniquePropertyMap.find(property) == m_uniquePropertyMap.end())
            {
            m_uniquePropertyMap[property] = &node.GetClass();
            m_noOfColumns += mapInfoProvider.GetNumberOfColumnUseToStoreProperty (*property);
            m_noOfProperties++;
            }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetNoOfUniqueColumns()
    {
    BuildUniquePropertyMap();
    return m_noOfColumns;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::GetProperties(ECPropertyListR properties)
    {
    BuildUniquePropertyMap();
    properties.clear();
    for (bmap<ECPropertyCP,ECClassCP>::const_iterator it =  m_uniquePropertyMap.begin(); it != m_uniquePropertyMap.end(); it++)
        properties.push_back(it->first);
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClusterList::Select(bvector<ECClusterP>& clusters,  ECClusterFilterCallback where, ECClusterCompareCallback orderby)
    {
    if (where == nullptr)
        for( bvector<ECClusterPtr>::const_iterator it =  m_clusters.begin(); it != m_clusters.end(); it++)
            clusters.push_back((*it).get());
    else
        for( bvector<ECClusterPtr>::const_iterator it =  m_clusters.begin(); it != m_clusters.end(); it++)
            {
            if (where(**it) == true)
                clusters.push_back((*it).get());
            }
    if (orderby != nullptr)
        std::sort(clusters.begin(), clusters.end(), orderby);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClusterList::WriteDebugInfoToFile(Utf8String fileName)
    {
    FILE* file = fopen(fileName.c_str(),"w");
    if (file == nullptr)
        {
        return;
        }

    fprintf(file, "All Clusters, No Of Unique Columns   : %d\n", GetNoOfUniqueColumns());
    fprintf(file, "All Clusters, No Of Unique Properties: %d\n", GetNoOfUniqueProperties());
    fprintf(file, "Total Number Of Clusters:              %d\n", Size());
    fprintf(file, "Total No Of Classes                  : %d\n", GetGraph().Size());
    fprintf(file, "Total No Of Unique Properties        : %d\n", GetGraph().GetUniquePropertyCount());
    fprintf(file, "Total No Of Properties               : %d\n", GetGraph().GetPropertyCount());


    fprintf (file, "\n=====================================================================\n");

    FOR_EACH (ECClusterPtr cluster, m_clusters)
        {
        ECNodeList list;
        fprintf (file, "Cluster                 : %s\n", cluster->GetName().c_str());
        fprintf (file, "No Of Classes           : %d\n", cluster->Size());
        fprintf (file, "No Of Unique Properties : %d\n", cluster->GetNoOfUniqueProperties());
        fprintf (file, "No Of Unique Columns    : %d\n", cluster->GetNoOfUniqueColumns());
        fprintf (file, "No Of Root Classes      : %d\n", cluster->GetRootNodes(list));
        int i = 1;
        for( NodeMapByECClass::const_iterator it =  cluster->m_nodes.begin(); it != cluster->m_nodes.end(); it++)
            {
            ECNodePtr n = it->second;
            fprintf (file, "\t%3d. %s %d %d %d\n", i++, Utf8String( n->GetClass().GetFullName()).c_str(), n->GetPropertyCount(), n->GetColumnCount(), n->GetClass().GetIsDomainClass());
            }
        fprintf (file, "\n=====================================================================\n");
        }
    fclose(file);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterList::ECClusterList(ECGraphR graph)
    {
    m_graph = &graph;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterListPtr ECClusterList::Create(ECGraphR graph)
    {
    return new ECClusterList (graph);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterP ECClusterList::Add (Utf8CP name)
    {
    ECClusterPtr cluster = new ECCluster(*this, name);
    m_clusters.push_back(cluster);
    return cluster.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClusterList::Remove (ECClusterR cluster)
    {
    for (bvector<ECClusterPtr>::iterator itor = m_clusters.begin(); itor != m_clusters.end(); ++itor)
        if (*itor == &cluster)
            {
            cluster.Clear();
            cluster.Dettach();
            m_clusters.erase (itor);
            return true;
            }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterP ECClusterList::Find (Utf8CP clusterName)
    {
    FOR_EACH (ECClusterPtr cluster, m_clusters)
        if (cluster->GetName().EqualsI(clusterName))
            return cluster.get();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterP ECClusterList::FindClusterWithNode (ECNodeCR node)
    {
    FOR_EACH (ECClusterPtr cluster, m_clusters)
        if (cluster->Find (node.GetClass()).IsValid())
            return cluster.get();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterP ECClusterList::operator [](int index) 
    { 
    if (index >= 0 && index < (int)m_clusters.size())
        return m_clusters[index].get();
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECClusterList::GetNoOfUniqueProperties ()
    {
    int nNoOfUniqueProperties = 0;
    FOR_EACH (ECClusterPtr cluster, m_clusters)
        nNoOfUniqueProperties += cluster->GetNoOfUniqueProperties();
    return nNoOfUniqueProperties;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECClusterList::GetNoOfUniqueColumns() 
    {
    int nNoOfUniqueColumns = 0;
    FOR_EACH (ECClusterPtr cluster, m_clusters)
        nNoOfUniqueColumns += cluster->GetNoOfUniqueColumns();
    return nNoOfUniqueColumns;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DefaultClustringAlgorithm::DefaultClustringAlgorithm()
    :m_name("DefaultClustringAlgorithm")
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClusterListPtr DefaultClustringAlgorithm::_Cluster (ClusteringAlgorithmConfigurationCR config, ECGraphR graph)
    {
    ECClusterListPtr clusterList = ECClusterList::Create(graph);
    m_config = &config;

    int maxNoOfColumns = graph.FindNodeWithMaximumNumberOfColumns()->GetColumnCount();
    if (m_config->GetMaxColumnsPerTable() < maxNoOfColumns)
        const_cast<ClusteringAlgorithmConfigurationP>(m_config)->SetMaxPropertiesPerTable (maxNoOfColumns);

    ClusterClassesWithExistingHints (*clusterList, graph);
    ClusterNonDomainAndEmptyClasses (*clusterList, graph);
    ClusterStructs                  (*clusterList, graph);
    ClusterRelationships            (*clusterList, graph);

    //Cluster using follow priority
    //1. Cluster all CA into one or more clusters
    ClusterCustomAttributes   (*clusterList, graph);
    //2. those that don’t exhibit the “diamond” problem of inheriting the same base class via two paths
    ClusterNonCyclicClasses   (*clusterList, graph);
    //3. Cluster classes starting with maximum connected class and try put it in closes related cluster
    //   if thats not possible create a new cluster and put it in that and continue to grow them.
    ClusterByHighConnectivity (*clusterList, graph);
    //4. Cluster together classes that doest have any parents or children.
    ClusterLonerClasses       (*clusterList, graph);
    //5. MergeClusters that cause reduction in number of columns
    MergeClusters (*clusterList, graph);
    //5. Deal with relationships
    return clusterList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DefaultClustringAlgorithm::_GetName () const
    {
    return m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::MergeClusters (ECClusterListR clusterList, ECGraphR graph)
    {
    bool                tryAgain;
    int                 nNoOfClustersMerged = 0;
    int                 nPropertiesSaved    = 0;
    bvector<ECClusterP> clusters;
    clusterList.Select(clusters, FilterClusterByType, OrderByBySizeOfCluster);

    printf("MergeClusters(): Before Merge no of clusters %d with columns %d\n", clusterList.Size(), clusterList.GetNoOfUniqueColumns());
    do
        {
        std::sort(clusters.begin(), clusters.end(), OrderByBySizeOfCluster);
        tryAgain = false;
        bvector<ECClusterP>::iterator candiateItor = clusters.begin();
        while (candiateItor != clusters.end())
            {
            ECClusterP canditate = *candiateItor;
            ECClusterP bestTarget = nullptr;
            int best  = 0;
            if (canditate->Size() == 0)
                continue;
            bvector<ECClusterP>::const_iterator targetItor = clusters.begin();
            for( ; targetItor != clusters.end(); targetItor++)
                {
                ECClusterP target= *targetItor;
                if (target->Size() == 0 || 
                    canditate == target || 
                    target->GetNoOfUniqueColumns() == m_config->GetMaxColumnsPerTable())
                    continue;
                int noOfColumnAfterMerge = target->GetNoOfUniqueColumnsIfMergedWith(*canditate);
                if (noOfColumnAfterMerge >= m_config->GetMaxColumnsPerTable()) 
                    continue;
                int maxNoOfColumnAfterMerge = canditate->GetNoOfUniqueColumns() + target->GetNoOfUniqueColumns();
                int quality = maxNoOfColumnAfterMerge - noOfColumnAfterMerge;
                if (quality > best)
                    {
                     best = quality;
                     bestTarget = target;
                    }
                }
                if (bestTarget != nullptr)
                    {
                    bestTarget->MergeFrom(*canditate);
                    bestTarget->SetType (ECCLUSTERTYPE_Merge);
                    tryAgain = true;
                    clusters.erase(candiateItor);
                    nPropertiesSaved += best;
                    nNoOfClustersMerged++;
                    break; // start over
                    }
             candiateItor = clusters.erase (candiateItor); //either merged or there is no match for it
            }
        } while (tryAgain && clusters.size() > 1);
    //clean up delete are cluster with zero elements
    clusterList.Select(clusters, nullptr, nullptr);
    for ( bvector<ECClusterP>::iterator itor = clusters.begin(); itor != clusters.end(); itor++)
        if ((*itor)->Size() == 0)
            clusterList.Remove(**itor);
    printf("MergeClusters(): Merged %d clusters saving %d columns\n", nNoOfClustersMerged, nPropertiesSaved);
    printf("MergeClusters(): After Merge no of clusters %d with columns %d\n", clusterList.Size(), clusterList.GetNoOfUniqueColumns());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetNoOfUniqueColumnsIfMergedWith(ECClusterR candiate)
    {
    BuildUniquePropertyMap();
    bmap<ECPropertyCP,ECClassCP> uniquePropertyMap = m_uniquePropertyMap; //copy
    int nMergeColumns = GetNoOfUniqueColumns();    
    IECMapInfoProviderCR provider= GetParent()->GetGraph().GetECMapInfoProvider();
    for (NodeMapByECClass::const_iterator it =  candiate.m_nodes.begin(); it != candiate.m_nodes.end(); it++)
        {
        FOR_EACH (ECPropertyCP property, it->second->GetProperties ())
            if (uniquePropertyMap.find(property) == uniquePropertyMap.end())
                {
                uniquePropertyMap[property] = it->first;
                nMergeColumns += provider.GetNumberOfColumnUseToStoreProperty (*property);
                }
        }
    return nMergeColumns;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int ECCluster::GetNoOfUniqueColumnsIfNodeAdded(ECNodeR node)
    {
    BuildUniquePropertyMap();
    bmap<ECPropertyCP,ECClassCP> uniquePropertyMap = m_uniquePropertyMap; //copy
    int nMergeColumns = GetNoOfUniqueColumns();    
    IECMapInfoProviderCR provider= GetParent()->GetGraph().GetECMapInfoProvider();
    FOR_EACH (ECPropertyCP property, node.GetProperties ())
        if (uniquePropertyMap.find(property) == uniquePropertyMap.end())
            {
            uniquePropertyMap[property] = &node.GetClass();
            nMergeColumns += provider.GetNumberOfColumnUseToStoreProperty (*property);
            }
    return nMergeColumns;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::GetNodes(ECNodeListR nodes)
    {
    nodes.clear();
    for (NodeMapByECClass::iterator it = m_nodes.begin(); it != m_nodes.end(); it++)
        nodes.push_back(it->second.get());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::MergeFrom(ECClusterR source)
    {
    ECNodeList nodes;
    source.GetNodes(nodes);

    for (ECNodeList::iterator it = nodes.begin(); it != nodes.end(); it++)
        {
        source.Remove(**it);
        Add(**it);
        }
    source.Clear();
    //GetParent()->Remove(source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterCustomAttributes (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterP cluster = nullptr;
    int i              = 0;  // Use to give serialize name for clusters 
    //1. Get only custom attributes and order them by number of derived classes
    graph.Select (nodes, FilterByCustomAttributes, OrderByNumberOfDerivedClasses);
    //2. Create CA class clusters each of which should not have more then the maximum number of properties requested.
    FOR_EACH( ECNodeP n, nodes)
        {
        if (cluster == nullptr || 
            cluster->GetNoOfUniqueColumnsIfNodeAdded(*n) >= m_config->GetMaxColumnsPerTable())
            {
            cluster = clusterList.Add (SqlPrintfString("ecdata_CACluster%02d", i++));
            cluster->SetType(ECCLUSTERTYPE_CustomAttributes);
            }
        cluster->Add (*n);
        }
  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterRelationships (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterP cluster = nullptr;
    int i              = 0;  // Use to give serialize name for clusters 
    //1. Get only custom attributes and order them by number of derived classes
    graph.Select (nodes, FilterByRelationshipClassess);
    //2. Create CA class clusters each of which should not have more then the maximum number of properties requested.
    FOR_EACH( ECNodeP n, nodes)
        {
        if (cluster == nullptr || 
            cluster->GetNoOfUniqueColumnsIfNodeAdded(*n) >= m_config->GetMaxColumnsPerTable())
            {
            cluster = clusterList.Add (SqlPrintfString("ecdata_Relationship%02d", i++));
            cluster->SetType(ECCLUSTERTYPE_Relationships);
            }
        cluster->Add (*n);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterClassesWithExistingHints (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterPtr cluster = clusterList.Add("ClassesWithExistingHints");
    //We want to ignore classes with ECDBClassHint on them.
    cluster->SetType (ECCLUSTERTYPE_UnmappableClasses); 
    graph.Select (nodes, FilterByECDbHint);
    FOR_EACH( ECNodeP n, nodes)
        {
        if (!cluster->Contains (*n))
            {
            MapStrategy mapStrategy = ClassMapInfo::GetMapStrategyHint (n->GetClass());
            cluster->Add (*n);
            if (mapStrategy == MapStrategy::TablePerHierarchy)
                {
                //Add all derived classes
                AddAllDerivedClassesToCluster(*n, *cluster);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::AddAllDerivedClassesToCluster (ECNodeR node, ECClusterR cluster)
    {
    FOR_EACH(ECNodeP derivedNode, node.GetDerivedNodes())
        {
        cluster.Add(*derivedNode);
        AddAllDerivedClassesToCluster (*derivedNode, cluster);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterStructs (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterP cluster = nullptr;
    int i              = 0;  
    graph.Select (nodes, FilterByStruct);
    FOR_EACH( ECNodeP n, nodes)
        {
        if (cluster == nullptr || 
            cluster->GetNoOfUniqueColumnsIfNodeAdded(*n) >= m_config->GetMaxColumnsPerTable())
            {
            cluster = clusterList.Add (SqlPrintfString("ecdata_Struct%02d", i++));
            cluster->SetType (ECCLUSTERTYPE_Struct);
            }
        cluster->Add (*n);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterNonDomainAndEmptyClasses (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterP cluster = nullptr;
    int i              = 0;
    graph.Select (nodes, FilterByNonMappedClasses);
    FOR_EACH( ECNodeP n, nodes)
        {
        if (cluster == nullptr || 
            cluster->GetNoOfUniqueColumnsIfNodeAdded(*n) >= m_config->GetMaxColumnsPerTable())
            {
            cluster = clusterList.Add (SqlPrintfString("ecdata_NonMappedClasses%02d", i++));
            cluster->SetType (ECCLUSTERTYPE_UnmappableClasses);
            }
        cluster->Add (*n);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterNonCyclicClasses (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList                  nodes; 
    bmap<ECClusterP,ECNodeList> unfitNodes;
    int                         i = 0; 
    graph.Select (nodes, FilterByClassThatHasExactlyOneBaseClass, OrderByDescendingNumberOfDerivedClasses);
    FOR_EACH (ECNodeP n, nodes)
        {
         ECNodeP    baseNode    = *n->GetBaseNodes().begin();
         ECClusterP nodeCluster = n->GetCluster();
         ECClusterP baseCluster = baseNode->GetCluster();

         if (nodeCluster != nullptr)
             if (nodeCluster->GetType() == ECCLUSTERTYPE_UnmappableClasses)
                 continue;
         if (baseCluster != nullptr)
             if (baseCluster->GetType() == ECCLUSTERTYPE_UnmappableClasses)
                 continue;

         if (nodeCluster == nullptr && baseCluster == nullptr)
             {
             ECClusterP cluster = clusterList.Add (SqlPrintfString("ecdata_NonCyclicClassCluster%02d", i++));
             cluster->SetType (ECCLUSTERTYPE_NonCyclic);
             cluster->Add (*n);
             cluster->Add (*baseNode);
             }
         else if (nodeCluster == nullptr)
             { 
             if (baseCluster->GetNoOfUniqueColumnsIfNodeAdded (*n) > m_config->GetMaxColumnsPerTable())
                 unfitNodes[baseCluster].push_back (n);
             else
                 baseCluster->Add (*n);
             }
         else if (baseCluster == nullptr)
             {
             if (nodeCluster->GetNoOfUniqueColumnsIfNodeAdded (*n) > m_config->GetMaxColumnsPerTable())
                 unfitNodes[nodeCluster].push_back (n);
             else
                 nodeCluster->Add (*baseNode);
             }
        }
    //process unfit nodes
    //size_t j;
    //do
    //    {
    //    j = 0;
    //    for (bmap<ECClusterP,ECNodeList>::iterator it =  unfitNodes.begin(); it != unfitNodes.end(); it++)
    //        {
    //       // ECClusterP  cluster = it->first;
    //        ECNodeListR nodes   = it->second;
    //        if (nodes.size() > 1)
    //            {
    //            ECClusterP overflowCluster = clusterList.Add (SqlPrintfString("ecdata_NonCyclicClassCluster%02d", i++));
    //            FOR_EACH(ECNodeP node, nodes)
    //                 if (overflowCluster->GetNoOfUniqueColumnsIfNodeAdded (*node) < m_config->GetMaxColumnsPerTable())
    //                     {
    //                     overflowCluster->Add(*node);
    //                     for (ECNodeList::iterator itor = nodes.begin(); itor != nodes.end(); ++itor)
    //                         if (*itor == node)
    //                             {
    //                             nodes.erase (itor);
    //                             break;
    //                             }
    //                     }
    //            }
    //        else
    //            j++;
    //        }
    //    } while ( j != unfitNodes.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterByHighConnectivity (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList                  nodes; 
    int                         i = 0;  // Use to give serialize name for clusters 

    graph.Select (nodes, FilterByClassesWithConnectivityGreaterThanZero, OrderByDescendingConnectivity);
    FOR_EACH (ECNodeP n, nodes)
        {
        if ( n->HasMoreBaseClassesThanDerivedClasses() /* prioritize which cluster to try first*/)
            {
            if (TryAddToABaseCluster (*n))    continue;
            if (TryAddToADerivedCluster (*n)) continue;
            }
        else
            {
            if (TryAddToADerivedCluster (*n)) continue;
            if (TryAddToABaseCluster (*n))    continue;
            }
        if (TryAddToASiblingCluster (*n)) continue;
        // fits no where create a new cluster
        ECClusterP cluster = clusterList.Add(SqlPrintfString("ecdata_HightConnectedClasses%02d", i++));
        cluster->Add (*n);
        cluster->SetType (ECCLUSTERTYPE_MaximumConnectivity);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultClustringAlgorithm::TryAddToABaseCluster (ECNodeR node)
    {
    bvector<ECClusterP> clusters;
    FOR_EACH (ECNodeP n, node.GetBaseNodes())
        {
        if (n->GetCluster() != nullptr)
            clusters.push_back (n->GetCluster());
        }

    std::sort(clusters.begin(), clusters.end(), OrderByClusterWithMinmumProperties); 
    return TrySelectAClusterWithMinimumProperties(clusters, node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultClustringAlgorithm::TryAddToADerivedCluster (ECNodeR node)
    {
    bvector<ECClusterP> clusters;
        {
        FOR_EACH (ECNodeP n, node.GetDerivedNodes())
            if (n->GetCluster() != nullptr)
                clusters.push_back (n->GetCluster());
        }
    std::sort(clusters.begin(), clusters.end(), OrderByClusterWithMinmumProperties); 
    return TrySelectAClusterWithMinimumProperties (clusters, node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultClustringAlgorithm::TrySelectAClusterWithMinimumProperties (bvector<ECClusterP>& clusters, ECNodeR n)
    {
    FOR_EACH (ECClusterP cluster, clusters)
        if(cluster->GetType() != ECCLUSTERTYPE_UnmappableClasses && cluster->GetType() != ECCLUSTERTYPE_Relationships)
            if (cluster->GetNoOfUniqueColumnsIfNodeAdded(n) < m_config->GetMaxColumnsPerTable())
                {
                cluster->Add (n);
                return true;
                }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool DefaultClustringAlgorithm::TryAddToASiblingCluster (ECNodeR node)
    {
    bmap<ECClusterP, bool> clusterMap;
    bvector<ECClusterP>    clusters;

    FOR_EACH (ECNodeP baseNode, node.GetBaseNodes())
        FOR_EACH (ECNodeP sibliningNode, baseNode->GetDerivedNodes())
            {
            ECClusterP cluster = sibliningNode->GetCluster();
            if (cluster != nullptr)
                clusterMap[cluster] = true;
            }
    for (bmap<ECClusterP, bool>::const_iterator it = clusterMap.begin(); it != clusterMap.end(); it++)
        clusters.push_back(it->first);

    std::sort (clusters.begin(), clusters.end(), OrderByClusterWithMinmumProperties);
    return TrySelectAClusterWithMinimumProperties (clusters, node);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void DefaultClustringAlgorithm::ClusterLonerClasses (ECClusterListR clusterList, ECGraphR graph)
    {
    ECNodeList nodes; 
    ECClusterP cluster = nullptr;
    int i              = 0;  // Use to give serialize name for clusters 
    //1. Get only classes with no derived or base classes
    graph.Select (nodes, FilterByClassesWithNoBaseOrDerivedClass);
    //2. Create loner class clusters each of which should not have more then the maximum number of properties requested.
    FOR_EACH( ECNodeP n, nodes)
        {
        if ( cluster == nullptr || 
            cluster->GetNoOfUniqueColumnsIfNodeAdded(*n) >= m_config->GetMaxColumnsPerTable())
            {
            Utf8String name;
            name.Sprintf("ecdata_LonerGroup%d", i++);
            cluster = clusterList.Add (name.c_str());
            cluster->SetType (ECCLUSTERTYPE_Loner);
            }
        cluster->Add (*n);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECCluster::GetPropertiesWithDuplicateName (ECPropertyListR list)
    {
    ECPropertyList uniqueProps;
    GetProperties (uniqueProps);
    bmap<WCharCP,ECPropertyList, CompareIWChar> map;

    for (ECPropertyList::const_iterator it = uniqueProps.begin(); it != uniqueProps.end(); it++)
        map[(*it)->GetName().c_str()].push_back (*it);

    for (bmap<WCharCP,ECPropertyList, CompareIWChar>::const_iterator it = map.begin(); it != map.end(); it++)
        if (it->second.size() > 1)
            for(ECPropertyList::const_iterator pitor = it->second.begin(); pitor != it->second.end(); pitor++)
                list.push_back(*pitor);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MapGenerator::CreateSupplementalSchema (ECSchemaPtr& supplementalSchema, ECSchemaCR primarySchema, ECSchemaCR bscaSchema)
    {
    WString supplementalSchemaName;
    WString namespacePrefix = L"ecdbs";
    WString purpose = L"ECDbMapping";

    //Make sure that we getting the bsca schema that contain hints
    BeAssert (bscaSchema.GetVersionMajor() >= 1 && bscaSchema.GetVersionMinor() >= 6);

    //create supplemental schema name
    supplementalSchemaName.Sprintf(L"%ls_Supplemental_%ls", primarySchema.GetName().c_str(), purpose.c_str());

    //create supplemental schema instance
    ECObjectsStatus r = ECSchema::CreateSchema (supplementalSchema, supplementalSchemaName, 1, 0 );
    supplementalSchema->SetNamespacePrefix (namespacePrefix);
    //add bsca reference schemas
    r = supplementalSchema->AddReferencedSchema (const_cast<ECSchemaR>(bscaSchema));

    // Add Custom attribute
    ECClassCP supplementalSchemaMetaDataClass = bscaSchema.GetClassCP(L"SupplementalSchemaMetaData");
    if (supplementalSchemaMetaDataClass == nullptr)
        {
        supplementalSchema = nullptr;
        return ECOBJECTS_STATUS_ClassNotFound;
        }
    IECInstancePtr supplementalSchemaMetaData = supplementalSchemaMetaDataClass->GetDefaultStandaloneEnabler()->CreateInstance();
    //set custom attribute values
    r = supplementalSchemaMetaData->SetValue(L"PrimarySchemaName",         ECValue (primarySchema.GetName().c_str()));
    r = supplementalSchemaMetaData->SetValue(L"PrimarySchemaMajorVersion", ECValue ((Int32)primarySchema.GetVersionMajor()));
    r = supplementalSchemaMetaData->SetValue(L"PrimarySchemaMinorVersion", ECValue ((Int32)primarySchema.GetVersionMinor()));
    r = supplementalSchemaMetaData->SetValue(L"Precedence",                ECValue ((Int32)99));
    r = supplementalSchemaMetaData->SetValue(L"Purpose",                   ECValue (purpose.c_str()));
    r = supplementalSchemaMetaData->SetValue(L"IsUserSpecific",            ECValue (false));
    //set custom attribute
    r = supplementalSchema->SetCustomAttribute (*supplementalSchemaMetaData);

    if ( r != ECOBJECTS_STATUS_Success)
        supplementalSchema = nullptr;
    return r;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr MapGenerator::GetOrAddSupplementalSchema(ECSchemaCR primarySchema)
    {
     bmap<ECSchemaCP, ECSchemaPtr>::const_iterator itor = m_supplementalSchemas.find (&primarySchema);
     if (itor == m_supplementalSchemas.end())
         {
         ECSchemaPtr supplementalSchema;
         if (CreateSupplementalSchema(supplementalSchema, primarySchema, *m_bscaSchema) == ECOBJECTS_STATUS_Success)
             {
             m_supplementalSchemas[&primarySchema] = supplementalSchema;
             return supplementalSchema;
             }
         else
             return nullptr;
         }
     return m_supplementalSchemas[&primarySchema];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP MapGenerator::GetOrAddSupplementalClass (ECClassCR classInPrimarySchema)
    {
    ECSchemaPtr supplementalSchema = GetOrAddSupplementalSchema (classInPrimarySchema.GetSchema());
    BeAssert (supplementalSchema.IsValid());
    if (supplementalSchema == nullptr)
        return nullptr;
    ECClassP supplementalClass = supplementalSchema->GetClassP(classInPrimarySchema.GetName().c_str());
    if (supplementalClass == nullptr)
        {
        if (classInPrimarySchema.GetRelationshipClassCP() == nullptr)
            {
            if (supplementalSchema->CreateClass (supplementalClass, classInPrimarySchema.GetName()) != ECOBJECTS_STATUS_Success)
                return nullptr;
            }
        else
            {
            ECRelationshipClassP rel;
            if (supplementalSchema->CreateRelationshipClass(rel, classInPrimarySchema.GetName()) != ECOBJECTS_STATUS_Success)
                return nullptr;
            supplementalClass = rel;
            }
        }
    return supplementalClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP MapGenerator::GetOrAddSupplementalProperty (ECPropertyCR propertyInPrimarySchema, ECClassR supplementalClass)
    {
    ECPropertyP supplementalProperty = supplementalClass.GetPropertyP(propertyInPrimarySchema.GetName());
    if (supplementalProperty == nullptr)
        {
        PrimitiveECPropertyP newProperty;
        if (supplementalClass.CreatePrimitiveProperty (newProperty, propertyInPrimarySchema.GetName(), PRIMITIVETYPE_String) != ECOBJECTS_STATUS_Success)
            return nullptr;
        return newProperty;
        }
    return supplementalProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MapGenerator::FindAndSetBSCASchema (ECDbSchemaManagerCR sm, bvector<ECSchemaPtr>& cache)
    {
    if (m_bscaSchema.IsNull())
        {
        //annotate ECSchema
        ECSchemaP bentleyStandardCustomAttributes = nullptr;
        BentleyStatus rc = sm.GetECSchema(bentleyStandardCustomAttributes, "Bentley_Standard_CustomAttributes.01.05", true);
        if (rc != SUCCESS)
            {
            FOR_EACH (ECSchemaPtr schema, cache)
                {
                if (schema->GetName().EqualsI(L"Bentley_Standard_CustomAttributes") && schema->GetVersionMajor() >= 1 && schema->GetVersionMinor() >= 6 )
                    {
                    bentleyStandardCustomAttributes = schema.get();
                    break;
                    }
                }
            }
        BeAssert (bentleyStandardCustomAttributes != nullptr);

        if (bentleyStandardCustomAttributes == nullptr)
            return ERROR;
        m_bscaSchema = bentleyStandardCustomAttributes;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MapGenerator::GetSupplementalSchemas(bvector<ECSchemaPtr>& schemas)
    {
    schemas.clear();
    for(bmap<ECSchemaCP, ECSchemaPtr>::const_iterator itor = m_supplementalSchemas.begin(); itor != m_supplementalSchemas.end(); itor++)
        schemas.push_back(itor->second);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MapGenerator::AnnotateECSchemas (bvector<ECSchemaPtr>& cache, ECDbSchemaManagerCR sm)
    {
    ClusteringAlgorithmPtr alog            = DefaultClustringAlgorithm::Create();
    IECMapInfoProviderPtr  mapInfoProvider = DefaultECMapInfoProvider::Create();
    m_supplementalSchemas.clear();

    //create supplemental schema
     ClusteringAlgorithmConfiguration config;
     config.SetMaxPropertiesPerTable (300);
     ECGraphPtr graph = ECGraph::Create (cache, *mapInfoProvider);
     ECClusterListPtr clusters = alog->Cluster(config, *graph);
     
     auto rc = FindAndSetBSCASchema(sm, cache);
     if (rc != SUCCESS)
         {
         //ERROR
         }

     ECClassP classECDbClassHint             = m_bscaSchema->GetClassP (BSCAC_ECDbClassHint);
#if !defined (NDEBUG)
     ECClassP classECDbSchemaHint            = m_bscaSchema->GetClassP (BSCAC_ECDbSchemaHint);
 #endif
     ECClassP classECDbPropertyHint          = m_bscaSchema->GetClassP (BSCAC_ECDbPropertyHint);
     ECClassP classECDbRelationshipClassHint = m_bscaSchema->GetClassP (BSCAC_ECDbRelationshipClassHint);
     
     BeAssert (classECDbClassHint             != nullptr);
     BeAssert (classECDbSchemaHint            != nullptr);
     BeAssert (classECDbPropertyHint          != nullptr);
     BeAssert (classECDbRelationshipClassHint != nullptr);


     //"ec_DataTable_%04d
     int nTableId = 0;
     IECInstancePtr ca;
     bvector<ECClusterP> clusterList;
     clusters->Select (clusterList);


     for (ECClusterP cluster : clusterList)
         {
         WString    tableName; // cluster->GetName().c_str() we will just serialize table names
         tableName.Sprintf(L"ec_DataTable_%04d", nTableId++);
         ECNodeList nodes;
         cluster->GetNodes(nodes);

         switch(cluster->GetType())
             {
             case ECCLUSTERTYPE_CustomAttributes:
             case ECCLUSTERTYPE_Loner:
             case ECCLUSTERTYPE_Struct:
             case ECCLUSTERTYPE_NonCyclic:
             case ECCLUSTERTYPE_Relationships:
             case ECCLUSTERTYPE_MaximumConnectivity:
             case ECCLUSTERTYPE_Merge:
                 {
                 for (ECNodeP node : nodes)
                     {
                     ECClassP supplementalClass = GetOrAddSupplementalClass(node->GetClass());
                     BeAssert (supplementalClass != nullptr);
                     //add class hint
                     ca = classECDbClassHint->GetDefaultStandaloneEnabler()->CreateInstance();
                     ca->SetValue (L"MapStrategy", ECValue(L"SharedTableForThisClass"));
                     ca->SetValue (L"TableName", ECValue(tableName.c_str()));
                     supplementalClass->SetCustomAttribute (*ca);
                     //Add relationship hint
                     if (cluster->GetType() == ECCLUSTERTYPE_Relationships)
                         {
                         ca = classECDbRelationshipClassHint->GetDefaultStandaloneEnabler()->CreateInstance();
                         ca->SetValue (L"PreferredDirection", ECValue(L"Bidirectional"));
                         ca->SetValue (L"AllowDuplicateRelationships", ECValue(true));
                         supplementalClass->SetCustomAttribute(*ca);                         }
                     }
                 //rename duplicate column name
                 ECPropertyList propertiesThatRequireRenaming;
                 cluster->GetPropertiesWithDuplicateName (propertiesThatRequireRenaming);
                 for (ECPropertyCP property : propertiesThatRequireRenaming)
                     {
                     ECClassCR ecClass = property->GetClass();
                     ECClassP supplementalClass = GetOrAddSupplementalClass(ecClass);
                     ECPropertyP supplementalProperty = GetOrAddSupplementalProperty(*property, *supplementalClass);
                     BeAssert (supplementalProperty != nullptr);
                     ca = classECDbPropertyHint->GetDefaultStandaloneEnabler()->CreateInstance();
                     WString columnName = property->GetClass().GetSchema().GetNamespacePrefix() + L"_" + ecClass.GetName() + L"_" + property->GetName();
                     ca->SetValue(L"ColumnName", ECValue(columnName.c_str()));
                     supplementalProperty->SetCustomAttribute (*ca);
                     }
                 break;
                 }
             case ECCLUSTERTYPE_UnmappableClasses:
                 {
                 for (ECNodeP node : nodes)
                     {
                     ECClassP supplementalClass = GetOrAddSupplementalClass(node->GetClass());
                     BeAssert (supplementalClass != nullptr);
                     ca = classECDbRelationshipClassHint->GetDefaultStandaloneEnabler()->CreateInstance();
                     ca->SetValue(L"MapStrategy", ECValue(L"DoNotMap"));
                     supplementalClass->SetCustomAttribute(*ca);
                     }
                 break;
                 }
             default:
                 BeAssert( false && "Case not handled");
             }
         }
        SupplementedSchemaBuilder builder;
        for (bmap<ECSchemaCP, ECSchemaPtr>::const_iterator itor = m_supplementalSchemas.begin(); itor != m_supplementalSchemas.end(); itor++)
         {
         ECSchemaPtr primarySchema      = const_cast<ECSchemaP>(itor->first);
         if (primarySchema->IsStandardSchema())
             continue;

         ECSchemaP   supplementalSchema = const_cast<ECSchemaP>(itor->second.get());
         bvector<ECSchemaP> supplementalSchemaList;
         supplementalSchemaList.push_back(supplementalSchema);

         if(!ECSchema::IsSchemaReferenced(*primarySchema, *m_bscaSchema) && 
             !primarySchema->IsStandardSchema())
             {
             primarySchema->AddReferencedSchema (*m_bscaSchema);
             }

         SupplementedSchemaStatus status = builder.UpdateSchema (*primarySchema, supplementalSchemaList);
              
         if (status != SUPPLEMENTED_SCHEMA_STATUS_Success)
             {
              printf("Error");
             }
         }

     bvector<ECSchemaPtr> supplementalSchemas;
     GetSupplementalSchemas (supplementalSchemas);
     for (ECSchemaPtr schema : supplementalSchemas)
         {
         WString fileName = L"C:\\" + schema->GetFullSchemaName() + L".ecschema.xml";
         schema->WriteToXmlFile(fileName.c_str());
         }
     clusters->WriteDebugInfoToFile("c:\\debug.txt");
     printf("Done");
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
