//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFHVVHSpatialIndex.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFHVVHSpatialIndex
//-----------------------------------------------------------------------------
// Relative ordering index using an array list
//-----------------------------------------------------------------------------

#pragma once

#include "HIDXIndex.h"
#include "HIDXSearchCriteria.h"
#include "HGFHVVHSpatialAttribute.h"
#include "HGF2DCoordSys.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Martin Roy

    Template class that implements a spatial index based upon the HVVH algorithm.

    Objects of type O need a method GetExtent() that returns an HGF2Dextent object.

    See the document "The HVVH data structure.pdf" for details on the internal
    structure.

    -----------------------------------------------------------------------------
*/
template <class O, class SI = DefaultSubIndexType<O> > class HGFHVVHSpatialIndex
    {
public:

    /////////////////////////////
    // Parameters object
    /////////////////////////////

    // The parameters object for the constructor. It receives the coordinate system
    // that will be used by the index internally.
    class Parameters
        {
    public:
        Parameters(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
            {
            m_pCoordSys = pi_rpCoordSys;
            };
        ~Parameters() {};

        HFCPtr<HGF2DCoordSys> m_pCoordSys;
        };


    /////////////////////////////
    // Standard indexing stuff
    /////////////////////////////


    HGFHVVHSpatialIndex(const Parameters& pi_rParameters,
                        const SI* pi_pSubIndex = 0);
    ~HGFHVVHSpatialIndex();

    // Define a type for a list of objects
    typedef list < O, allocator< O > >  ObjectList;
    typedef typename HIDXIndexable<O>::List      IndexableList;

    // Management

    void            Add(const O pi_Object);
    void            Add(const ObjectList& pi_rObjects);
    void            Remove(const O pi_Object);

    void            AddIndexable(const HFCPtr< HIDXIndexable<O> >& pi_rpObject);
    void            AddIndexables(const HAutoPtr< typename HIDXIndexable<O>::List >& pi_rpObjects);
    void            RemoveIndexable(const HFCPtr< HIDXIndexable<O> >& pi_rpObject);


    // Information retrieval

    ObjectList*
    Query(const HIDXSearchCriteria& pi_rCriteria) const;

    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria&           pi_rCriteria,
                    HAutoPtr< typename HIDXIndexable<O>::List >& pi_rSubset,
                    bool                               pi_Sort = true) const;
    typename HIDXIndexable<O>::List*
    QueryIndexables(const HIDXSearchCriteria& pi_rCriteria,
                    bool                     pi_Sort = true) const;

    bool           SupportsInteractingRetrieval() const;

    typename HIDXIndexable<O>::List* 
    GetInteractingObjects(typename HIDXIndexable<O>::List const& pi_rpObject) const;

    const HFCPtr< HIDXIndexable<O> >
    GetFilledIndexableFor(const HFCPtr< HIDXIndexable<O> >& pi_rpObject) const;

    HIDXSortingRequirement
    GetSortingRequirement() const;


    /////////////////////////////
    // Sorting
    /////////////////////////////


    // Object used to sort STL lists...
    // This is only here so that complex indexes work properly.
    // It will never be queried, but the type must exist.
    class Predicate : binary_function<HFCPtr< HIDXIndexable<O> >, HFCPtr< HIDXIndexable<O> >, bool>
        {
    public:

        Predicate() {};

        bool operator()(HFCPtr< HIDXIndexable<O> > pi_rpFirst, HFCPtr< HIDXIndexable<O> > pi_rpSecond)
            {
            // Nothing to compare.
            return true;
            };
        };


    Predicate       GetPredicate() const;

private:

    /////////////////////////////
    // Internal types and stuff
    /////////////////////////////


    typedef HGFHVVHSpatialAttribute Attribute;

    // The maximum number of objects in the nodes.
    enum
        {
        MAX_NODE_LOAD = 2,
        MAX_CUTNODE_LOAD = 2
        };

    // Cut nodes for a node's cut tree.
    struct CutNode
        {
        CutNode(CutNode* pi_pAncestor)
            {
            // No need to assign right node pointer, as we only
            // use the left one to know if there are sub-nodes.
            pLeftNode = 0;

            pAncestorNode = pi_pAncestor;

            // Set default values for the min and max values.
            // If the node is empty, these values are
            // meaningless.
            MinOnSplitAxis = DBL_MAX;
            MaxOnSplitAxis = (-DBL_MAX);
            MinOnOtherAxis = DBL_MAX;
            MaxOnOtherAxis = (-DBL_MAX);
            };

        ~CutNode()
            {
            if (pLeftNode)
                {
                // If left is not NULL, we have two sub-nodes.

                delete pLeftNode;
                delete pRightNode;
                }
            };

        void CreateSubNodes()
            {
            // We create the nodes. The next call to IsLeaf will return
            // false. Objects currenty in the node must be redistributed
            // in the sub-nodes if they don't touch the split mark.
            pLeftNode  = new CutNode(this);
            pRightNode = new CutNode(this);

            // These values are invalidated when we split
            MinOnSplitAxis = DBL_MAX;
            MaxOnSplitAxis = (-DBL_MAX);
            };

        void DeleteSubNodes()
            {
            delete pLeftNode;
            delete pRightNode;
            pLeftNode = 0;

            // These values are not valid anymore
            MinOnSplitAxis = DBL_MAX;
            MaxOnSplitAxis = (-DBL_MAX);
            MinOnOtherAxis = DBL_MAX;
            MaxOnOtherAxis = (-DBL_MAX);
            };

        bool IsLeaf() const {
            return pLeftNode == 0;
            };

        size_t GetLoad() const {
            return Objects.size();
            };

        size_t GetTreeLoad() const {
            return pLeftNode ? pLeftNode->GetTreeLoad() + pRightNode->GetTreeLoad() + Objects.size() :
                   Objects.size();
            };

        size_t GetTreeLoadWithout(CutNode* pi_pNode) const {
            return pi_pNode == pLeftNode ? pRightNode->GetTreeLoad() + Objects.size() :
                   pLeftNode->GetTreeLoad() + Objects.size();
            };

        void GatherAllObjects(typename HIDXIndexable<O>::List* pi_pObjects) const
            {
            if (pLeftNode)
                {
                pLeftNode->GatherAllObjects(pi_pObjects);
                pRightNode->GatherAllObjects(pi_pObjects);
                }

            pi_pObjects->insert(pi_pObjects->begin(), Objects.begin(), Objects.end());
            };

        void GatherAllObjects(ObjectList& pi_rObjects) const
            {
            if (pLeftNode)
                {
                pLeftNode->GatherAllObjects(pi_rObjects);
                pRightNode->GatherAllObjects(pi_rObjects);
                }

            // Add our objects
            typename HIDXIndexable<O>::List::const_iterator Itr(Objects.begin());
            while (Itr != Objects.end())
                {
                pi_rObjects.push_back((*Itr)->GetObject());
                ++Itr;
                }
            };

        // The list of objects cut at this level
        typename HIDXIndexable<O>::List Objects;

        // Min and Max values on the split axis.
        double  MinOnSplitAxis;
        double  MaxOnSplitAxis;

        double  MinOnOtherAxis;
        double  MaxOnOtherAxis;

        // Splitting point on the current split axis
        double  SplitPosition;

        // Two cut sub-nodes. Left is NULL if no sub-nodes
        CutNode* pLeftNode;
        CutNode* pRightNode;

        CutNode* pAncestorNode;
        };

    // Nodes of the tree
    struct Node
        {
        Node(bool pi_SplitHorizontally, Node* pi_pAncestor)
            {
            // No need to assign right node pointer, as we only
            // use the left one to know if there are sub-nodes.
            pLeftNode = 0;

            SplitHorizontally = pi_SplitHorizontally;

            pAncestorNode = pi_pAncestor;

            pObjects = new typename HIDXIndexable<O>::List;
            };

        ~Node()
            {
            if (pLeftNode)
                {
                // If left is not NULL, we have two sub-nodes and a cut tree (possibly)

                delete pLeftNode;
                delete pRightNode;

                delete pCut;
                }
            else
                {
                // No sub-nodes, so we contain an object list.
                delete pObjects;
                }
            };

        typename HIDXIndexable<O>::List* CreateSubNodes()
            {
            // We create the nodes. The next call to IsLeaf will return
            // false. We return the list of objects that were in the
            // node before the split. This list MUST be deleted by
            // the caller.
            pLeftNode  = new Node(!SplitHorizontally, this);
            pRightNode = new Node(!SplitHorizontally, this);

            typename HIDXIndexable<O>::List* pResult = pObjects;
            pCut = 0;

            return pResult;
            };

        void DeleteSubNodes(typename HIDXIndexable<O>::List* pi_pObjects)
            {
            delete pLeftNode;
            delete pRightNode;
            pLeftNode = 0;

            delete pCut;
            pCut = 0;

            pObjects = pi_pObjects;
            };

        bool IsLeaf() const {
            return pLeftNode == 0;
            };

        // Only on a leaf!
        size_t GetLoad() const {
            return pObjects->size();
            };

        // The complete load with all children
        size_t GetTreeLoad() const
            {
            if (pCut)
                {
                return pLeftNode ? pLeftNode->GetTreeLoad() + pRightNode->GetTreeLoad() + pCut->GetTreeLoad() :
                       pObjects->size();
                }
            else
                {
                return pLeftNode ? pLeftNode->GetTreeLoad() + pRightNode->GetTreeLoad() :
                       pObjects->size();
                }
            };

        // The load of the node plus all children on one side only.
        size_t GetTreeLoadWithout(Node* pi_pNode) const
            {
            if (pCut)
                {
                return pi_pNode == pLeftNode ? pRightNode->GetTreeLoad() + pCut->GetTreeLoad() :
                       pLeftNode->GetTreeLoad()  + pCut->GetTreeLoad();
                }
            else
                {
                return pi_pNode == pLeftNode ? pRightNode->GetTreeLoad() :
                       pLeftNode->GetTreeLoad();
                }
            };

        void GatherAllObjects(typename HIDXIndexable<O>::List* pi_pObjects) const
            {
            if (pLeftNode)
                {
                pLeftNode->GatherAllObjects(pi_pObjects);
                pRightNode->GatherAllObjects(pi_pObjects);

                if (pCut)
                    pCut->GatherAllObjects(pi_pObjects);
                }
            else
                {
                // Add our objects
                pi_pObjects->insert(pi_pObjects->begin(), pObjects->begin(), pObjects->end());
                }
            };

        void GatherAllObjects(ObjectList& pi_rObjects) const
            {
            if (pLeftNode)
                {
                pLeftNode->GatherAllObjects(pi_rObjects);
                pRightNode->GatherAllObjects(pi_rObjects);

                if (pCut)
                    pCut->GatherAllObjects(pi_rObjects);
                }
            else
                {
                // Add our objects
                typename HIDXIndexable<O>::List::const_iterator Itr(pObjects->begin());
                while (Itr != pObjects->end())
                    {
                    pi_rObjects.push_back((*Itr)->GetObject());
                    ++Itr;
                    }
                }

            };

        // The two sub-nodes. Left is NULL if no sub-nodes.
        Node*   pLeftNode;
        Node*   pRightNode;

        // Back pointer to ancestor. Used to gather objects
        // that interact with a given one.
        Node*   pAncestorNode;

        // Splitting point. For example, for a horizontal node,
        // this value will be its position on the Y axis.
        double SplitPosition;

        // Does this node split horizontally  or vertically
        bool   SplitHorizontally;

        union
            {
            // The cut tree for this node. Used if pLeftNode != NULL
            CutNode*                pCut;

            // The list of objects in the node, if it is a leaf
            // (pLeftNode == NULL)
            typename HIDXIndexable<O>::List* pObjects;
            };
        };


    enum RelativePosition
        {
        LEFT,
        RIGHT,
        CENTER
        };


    /////////////////////////////
    // Private methods
    /////////////////////////////

    // Copy ctor and assignment are disabled
    HGFHVVHSpatialIndex(const HGFHVVHSpatialIndex<O, SI>& pi_rObj);
    HGFHVVHSpatialIndex<O, SI>& operator=(const HGFHVVHSpatialIndex<O, SI>& pi_rObj);

    void            InsertInCutTree(const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
                                    bool                             pi_HorizontalCutNode,
                                    CutNode*                          pi_pCutNode,
                                    const HGF2DExtent&                pi_rObjectExtent);

    double         CalculateSplitPosition(const typename HIDXIndexable<O>::List& pi_rObjects,
                                           bool                         pi_NodeSplitsHorizontally) const;

    void            SetAttributeOfObject(const HFCPtr< HIDXIndexable<O> >& pi_rpObject,
                                         Node*                             pi_pNode);

    RelativePosition
    CalculateRelativePosition(bool                         pi_NodeSplitsHorizontally,
                              double                       pi_SplitPosition,
                              const HGF2DExtent&            pi_rObjectExtent) const;

    void            AdjustCutNodeBoundsAfterInsertion(CutNode*           pi_pNode,
                                                      bool              pi_HorizontalCutNode,
                                                      const HGF2DExtent& pi_rObjectExtent,
                                                      bool              pi_ObjectInsertedHere);

    void            AdjustCutNodeBoundsAfterRemoval(CutNode*           pi_pNode,
                                                    bool              pi_HorizontalCutNode,
                                                    const HGF2DExtent& pi_rObjectExtent,
                                                    bool              pi_ObjectInsertedHere);

    void            TryToMergeNodes(Node* pi_pNode);
    void            TryToMergeCutNodes(CutNode* pi_pNode);

    void            ValidateCutNodeBounds(CutNode* pi_pNode,
                                          bool    pi_HorizontalCutNode) const;

    void            FindObjectsInTree(Node*                   pi_pNode,
                                      const HGF2DExtent&      pi_rRegion,
                                      typename HIDXIndexable<O>::List& pi_rResult) const;

    void            FindObjectsInCutTree(CutNode*                pi_pNode,
                                         bool                   pi_HorizontalCutNode,
                                         const HGF2DExtent&      pi_rRegion,
                                         RelativePosition        pi_PositionOfRegionInHVNode,
                                         typename HIDXIndexable<O>::List& pi_rResult) const;

    void            TestObjectsInList(typename HIDXIndexable<O>::List& pi_rObjects,
                                      const HGF2DExtent&      pi_rRegion,
                                      typename HIDXIndexable<O>::List& pi_rResult) const;


    /////////////////////////////
    // Members
    /////////////////////////////


    // Pointer to the underlying index
    const SI*       m_pSubIndex;

    // The coordinate system to use internally
    HFCPtr<HGF2DCoordSys>
    m_pCoordSys;

    // The root of the HV/VH tree.
    Node*           m_pRoot;

    // List of objects that can't be indexed spatially
    // (i.e. Their extent is undefined)
    typename HIDXIndexable<O>::List
    m_UnindexedObjects;
    };

END_IMAGEPP_NAMESPACE

#include "HGFHVVHSpatialIndex.hpp"


