/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StronglyConnectedGraph.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once
#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min

/*---------------------------------------------------------------------------------**//**
Adapted Tarjan's Strongly connected graph algorithm to determine cycles in the graph.
http://en.wikipedia.org/wiki/Tarjan's_strongly_connected_components_algorithm
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename NodeType, typename GetChildrenFunctor>
struct SCCGraph
    {
    struct SCCGraphNode
        {
        NodeType    m_node;
        int         m_index;
        int         m_lowIndex;

        SCCGraphNode (NodeType node, int index)
            :m_node (node), m_index(index), m_lowIndex(index)
            {}
        };

    typedef bmap<NodeType, SCCGraphNode*>       Vertices;
    typedef bvector<SCCGraphNode*>              NodeVector;
    typedef bvector<NodeVector>                 SccNodes;

    private:
    Vertices                    m_vertices;
    int                         m_currentIndex;
    GetChildrenFunctor const &  m_getChildren;

    public:
    typedef typename GetChildrenFunctor::ChildCollection ChildNodeCollection;

    struct SCCContext
        {
        NodeVector  m_stack;
        SccNodes    m_components;
        };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
   SCCGraph (GetChildrenFunctor const& fnCtor)
        :m_currentIndex(0), m_getChildren(fnCtor)
        {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
    SCCGraphNode*   StronglyConnect (NodeType inNode , SCCContext& context)
        {
        SCCGraphNode* node = new SCCGraphNode(inNode, m_currentIndex);
        bpair<typename Vertices::iterator, bool> insertionPoint =  m_vertices.insert(typename Vertices::value_type(inNode, node));
        if(!insertionPoint.second)
            {
            delete node;
            node = insertionPoint.first->second;
            }

        context.m_stack.push_back(node);
        m_currentIndex++;

        ChildNodeCollection childList = m_getChildren (inNode);
        for (typename ChildNodeCollection::const_iterator iter = childList.begin(); iter != childList.end(); ++iter)
            {
            typename Vertices::iterator childIter = m_vertices.find (*iter);

            SCCGraphNode* refNode = NULL;
            if (m_vertices.end() == childIter)
                {
                refNode = StronglyConnect (*iter, context);
                node->m_lowIndex = std::min (node->m_lowIndex, refNode->m_lowIndex);
                }
            else
                {
                refNode = childIter->second;
                if (context.m_stack.end() != std::find_if (context.m_stack.begin(), context.m_stack.end(), std::bind1st (std::equal_to<SCCGraphNode*>(), refNode)))
                    node->m_lowIndex = std::min (node->m_lowIndex, refNode->m_index);
                }
            }

        if (node->m_lowIndex == node->m_index && !context.m_stack.empty())
            {
            NodeVector scc;
            SCCGraphNode* stackNode = NULL;
            do
                {
                stackNode = context.m_stack.back();
                scc.push_back(stackNode);
                context.m_stack.pop_back();
                }
            while ((!context.m_stack.empty()) && node != stackNode);

            if (!scc.empty())
                context.m_components.push_back(scc);
            }

        return node;
        }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
    ~SCCGraph ()
        {
        for (typename Vertices::iterator iter = m_vertices.begin(); iter != m_vertices.end(); ++iter)
            delete iter->second;
        m_vertices.clear();
        }
    };

#pragma pop_macro("max")
#pragma pop_macro("min")
