/*--------------------------------------------------------------------------------------+
|
|   Code partially derived from Michael Garland's demo application called "QSlim" 
|   (version 1.0) which intends to demonstrate an algorithm of mesh simplification based 
|   on Garland and Heckbert(1997) "Surface Simplification Using Quadric Error Metrics".
|   The code of the demo is said to be in the public domain.
|   See: http://www.cs.cmu.edu/afs/cs/Web/People/garland/quadrics/qslim10.html
|
|   $Revision: 1.0 $
|       $Date: 2014/09/17 $
|     $Author: Christian.Cote $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <iostream>
#include <fstream>
#include "Model.h"
#include "Quadrics.h"
#include "TypesAndDefinitions.h"
#include "Buffer.h"
#include "Heap.h"
#include "ProximityGrid.h"

class pair_info : public Heapable
{
    public:
        Vertex *v0, *v1;
        Vec3 candidate;
        double cost;
        pair_info(Vertex* a, Vertex* b) { v0 = a; v1 = b; cost = HUGE; }
        bool isValid() { return v0->IsValid() && v1->IsValid(); }
};

typedef buffer<pair_info *> pair_buffer;

class vert_info
{
    public:
        pair_buffer pairs;
        Mat4 Q;
        double norm;
        vert_info() : Q(Mat4::zero) { pairs.init(2); norm = 0.0; }
};

const double DEFAULT_BOUNDARY_CONSTRAINT_WEIGHT = 1.0;

class GarlandMeshFilter
{
    private:

        #define MESH_INVERSION_PENALTY 1e9

        // Adjacency model (representing a mesh) on
        // which is applied the edge collapse treatment:
        //------------------------------------------------
        Model* m_model;

        // Heap used as priority queue 
        // for storing pairs of vertices:
        //------------------------------------------------
        Heap* m_heap;

        // Used during the decimation process:
        //------------------------------------------------
        array<vert_info> m_verticeInfoArray;
        std::vector<pair_info*> m_pairInfoVector;

        // Distance threshold defining pair of points not previously connected:
        double m_proximityLimit; 

        // Decimation parameters:
        //------------------------------------------------
        int m_targetNumberVertices;  
        double m_errorTolerance;
        bool m_usePlaneConstraint;
        bool m_useVertexConstraint;
        bool m_preserveBoundaries;
        bool m_preserveMeshQuality;
        bool m_constrainBoundaries;
        double m_boundaryConstraintWeight;
        bool m_weightByArea;
        OptimalPlacementPolicyType m_placementPolicy;
        double m_pairSelectionTolerance;
        ConstraintQuadricType m_constraintQuadricType;

        // Debug - To produce analyses about input meshes:
        //------------------------------------------------
        string m_report;
        string m_reportName;
        int m_loopStopCount;
        bool m_reachedTargetStop;        
        bool m_unavailablePairStop;    
        bool m_overErrorToleranceStop;
        
        // Decimation functions:
        //------------------------------------------------
        bool PairIsValid(Vertex* u, Vertex* v);
        int PredictFace(Face& F, Vertex* v1, Vertex* v2, Vec3& vnew, Vec3& f1, Vec3& f2, Vec3& f3);
        double PairMeshPenalty(Vertex *v1, Vertex *v2, Vec3& vnew);
        void ComputePairInfoAndUpdateHeap(pair_info *pair);
        void DoContract(pair_info *pair);

        bool EvalDecimateQuadric(Vertex* v, Mat4& Q);
        double EvalDecimateMaxError();
        double EvalDecimateError(Vertex *);
        pair_info * GetPairForContraction();
        void InitializeHeap();

        // Low-level routines for manipulating pairs:
        //------------------------------------------------
        vert_info& GetVertexInfo(Vertex *v);
        bool CheckForPair(Vertex *v0, Vertex *v1);
        pair_info* CreateNewPair(Vertex *v0, Vertex *v1);
        void DeletePair(pair_info *pair);

        // Other functions:
        //------------------------------------------------
        void InitSettings();
        void SetConstraintQuadricType(ConstraintQuadricType constraintQuadricType);
        void SetBoundaryConstraint(double boundaryConstraintWeight);

    public:

        GarlandMeshFilter();
        ~GarlandMeshFilter();

        void EdgeCollapseInit(int verticeNumberTarget);
        void EdgeCollapseRun();

        void AddPointToAdjacencyModel(double x, double y, double z);
        void AddFaceToAdjacencyModel(int32_t indexA, int32_t indexB, int32_t indexC);

        int GetInitialFaceCount();
        int GetInitialVertexCount();
        void UpdateFaceAndVerticeCount();
        void CleanUpAdjacencyModel();

        // Used after the cleanup of the model:
        int GetCurrentFaceCount();    
        int GetCurrentVertexCount();

        void GetRemainingPoints(vector<DPoint3d>& points);
        void GetRemainingFaceIndexes(vector<int32_t>& faceIndexes);

        void OutputModelToFile(string outputFilepath);
        void OutputMeshToFile(string fileName, size_t nbVertices, const DPoint3d* vertices, 
                              size_t nbFaceIndexes, const int32_t* faceIndexes);

        // For debug / analysis:  
        void AnalyseFacesAndVerticesAndSaveItToFile(string meshName, string reportName);
        void AnalyseCurrentFacesAndVertices();
        string AnalyseModelFacesAndVertices();
        string StrValid(bool validity);
        string StrCoord(Vertex* v1);
        string StrVertIdx(Vertex* v1, Vertex* v2, Vertex* v3);
        string StrNorm(Vec3& normal);
        string FormatReasonForProcessTermination();
        void SaveCurrentFaceAndVertexAnalysis(string meshName, string reportName);
        void SaveFaceAndVertexAnalysis(string meshName, string reportName, string report);   
        void SaveReasonForProcessTermination(string meshName, string fileName);
        string GetReasonForProcessTermination();
        void DefineSimplificationProcessStopCondition(int currrentVertexCount, int targetNbVertices,
                                                      bool availablePairsInHeap, bool errorTolerated);
        string OutputHeapNodeInfo(heap_node& n, int nodeIndex);
        void DumpHeap(string fileName);
    };







