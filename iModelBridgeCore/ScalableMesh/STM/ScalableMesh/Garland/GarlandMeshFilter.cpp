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

#include <ScalableMeshPCH.h>
#include "GarlandMeshFilter.h"

GarlandMeshFilter::GarlandMeshFilter()
{
    m_model = new Model();
    m_heap = new Heap();
    InitSettings();

    // For debug
    /*m_reachedTargetStop = false;
    m_unavailablePairStop = false;
    m_overErrorToleranceStop = false;
    m_loopStopCount = 0;*/
}

GarlandMeshFilter::~GarlandMeshFilter()
{
    delete m_model;
    delete m_heap;

    auto infoIter(m_pairInfoVector.begin());
    auto infoIterEnd(m_pairInfoVector.end());

    while (infoIter != infoIterEnd)
    {
        delete *infoIter;
        infoIter++;
    }
}

/*---------------------------------------------------------------------------------**//**
*   Initializes the parameters with predefined values.
+---------------+---------------+---------------+---------------+---------------+------*/
void GarlandMeshFilter::InitSettings()
{
    m_targetNumberVertices = 0;  
    m_errorTolerance = HUGE;
    m_usePlaneConstraint = true;
    m_useVertexConstraint = false;
    m_preserveBoundaries = false;
    m_preserveMeshQuality = false;
    m_constrainBoundaries = false;
    m_boundaryConstraintWeight = DEFAULT_BOUNDARY_CONSTRAINT_WEIGHT;
    m_weightByArea = false;
    m_placementPolicy = PLACE_OPTIMAL;
    m_pairSelectionTolerance = 0.0;
    m_constraintQuadricType = PLANE_CONSTRAINT;
}

/*---------------------------------------------------------------------------------**//**
*   Sets parameters, build the adjacency model and then remove degenerated faces
*   and unused vertices.
+---------------+---------------+---------------+---------------+---------------+------*/
void GarlandMeshFilter::EdgeCollapseInit(int verticeNumberTarget)
{
    m_targetNumberVertices = verticeNumberTarget;

    // The default value for the boundary constraint weight is 1.0 (means no constraint).
    SetBoundaryConstraint(100.0); 

    SetConstraintQuadricType(PLANE_CONSTRAINT);

    // HUGE means the error is not considered.
    m_errorTolerance = HUGE; 

    m_pairSelectionTolerance = 0.0;
    m_preserveMeshQuality = false;
    m_weightByArea = false;
    m_placementPolicy = PLACE_OPTIMAL;
}

void GarlandMeshFilter::AddPointToAdjacencyModel(double x, double y, double z)
{
    m_model->AddVertice(x, y, z);
}

void GarlandMeshFilter::AddFaceToAdjacencyModel(int32_t indexA, int32_t indexB, int32_t indexC)
{
    m_model->AddFace(indexA, indexB, indexC);
}

void GarlandMeshFilter::SetConstraintQuadricType(ConstraintQuadricType constraintQuadricType)
{
    if (constraintQuadricType == PLANE_CONSTRAINT)
    {
        m_usePlaneConstraint = true;
    }
    else if (constraintQuadricType == VERTEX_CONSTRAINT)
    {
        m_useVertexConstraint = true;
    }
}

void GarlandMeshFilter::SetBoundaryConstraint(double boundaryConstraintWeight)
{
    if (boundaryConstraintWeight > DEFAULT_BOUNDARY_CONSTRAINT_WEIGHT)
    {
        m_constrainBoundaries = true;
        m_boundaryConstraintWeight = boundaryConstraintWeight;  
    }
}

void GarlandMeshFilter::GetRemainingPoints(vector<DPoint3d>& points)
{
    return m_model->GetRemainingPoints(points);
}

void GarlandMeshFilter::GetRemainingFaceIndexes(vector<int32_t>& faceIndexes)
{
    return m_model->GetRemainingFaceIndexes(faceIndexes);
}

// To be used once the model has been built with adding vertices and faces.
// Initialization before the cleanup in the function CleanUpAdjacencyModel().
void GarlandMeshFilter::UpdateFaceAndVerticeCount()
{
    m_model->currentVertCount = m_model->InitialVerticeCount();
    m_model->currentEdgeCount = m_model->InitialEdgeCount();
    m_model->currentFaceCount = m_model->InitialFaceCount();
}

void GarlandMeshFilter::CleanUpAdjacencyModel()
{
    // Get rid of degenerate faces
    //-----------------------------------------------
    for (int i = 0; i < m_model->InitialFaceCount(); i++)  
    {
        if (!m_model->GetFace(i)->Plane().IsValid())
        {
            m_model->KillFace(m_model->GetFace(i));
        }
    }
    m_model->RemoveDegeneracy(m_model->AllFaces());

    // Get rid of unused vertices
    //-----------------------------------------------
    for (int i = 0; i<m_model->InitialVerticeCount(); i++)
    {
        if (m_model->GetVertex(i)->EdgeUses().length() == 0)
        {
            m_model->KillVertex(m_model->GetVertex(i));
        }          
    }
}

int GarlandMeshFilter::GetCurrentFaceCount()
{
    return m_model->currentFaceCount;
}

int GarlandMeshFilter::GetCurrentVertexCount()
{
    return m_model->currentVertCount;
}

int GarlandMeshFilter::GetInitialFaceCount()
{
    return m_model->InitialFaceCount();
}

int GarlandMeshFilter::GetInitialVertexCount()
{
    return m_model->InitialVerticeCount();
}

/*---------------------------------------------------------------------------------**//**
*   1- Distributes the constaints and build the heap.
*   2- Performs edge contraction until reaching the expected number of vertices or 
*      max error.
*   3- Returns the information defining the new simplified mesh.
+---------------+---------------+---------------+---------------+---------------+------*/
void GarlandMeshFilter::EdgeCollapseRun()
{     
    InitializeHeap();

    bool availablePairsInHeap = true;
    bool errorTolerated = true;
    double decimateMinError = -1;

    while (m_model->CurrentVerticeCount() > m_targetNumberVertices && errorTolerated && availablePairsInHeap)
        {            
            //m_loopStopCount++;
            pair_info *pair = GetPairForContraction();

            if (pair)
            {
                decimateMinError = pair->cost;
                if (decimateMinError < m_errorTolerance)
                {
                    DoContract(pair);
                } 
                else
                {
                    errorTolerated = false;
                }    
            }
            else
            {
                availablePairsInHeap = false;
            }
        }
    // For debug
    /*DefineSimplificationProcessStopCondition(m_model->currentVertCount, m_targetNumberVertices, 
                                               availablePairsInHeap, errorTolerated);*/
}

/*---------------------------------------------------------------------------------**//**
*   The remaining code was initially found in the file called "Decimate.cpp".
+---------------+---------------+---------------+---------------+---------------+------*/


/*---------------------------------------------------------------------------------**//**
*   Low-level routines for manipulating pairs.
+---------------+---------------+---------------+---------------+---------------+------*/
vert_info& GarlandMeshFilter::GetVertexInfo(Vertex *v)
{
    return m_verticeInfoArray(v->ValidID());
}

bool GarlandMeshFilter::CheckForPair(Vertex *v0, Vertex *v1)
{
    const pair_buffer& pairs = GetVertexInfo(v0).pairs;

    for (int i = 0; i<pairs.length(); i++)
    {
        if (pairs(i)->v0 == v1 || pairs(i)->v1 == v1)
        {
            return true;
        }
    }
    return false;
}

pair_info* GarlandMeshFilter::CreateNewPair(Vertex *v0, Vertex *v1)
{    
    vert_info& v0_info = GetVertexInfo(v0);
    vert_info& v1_info = GetVertexInfo(v1);

    pair_info *pair = new pair_info(v0, v1);

    m_pairInfoVector.push_back(pair);

    v0_info.pairs.add(pair);
    v1_info.pairs.add(pair);

    return pair;
}

void GarlandMeshFilter::DeletePair(pair_info *pair)
{
    vert_info& v0_info = GetVertexInfo(pair->v0);
    vert_info& v1_info = GetVertexInfo(pair->v1);

    v0_info.pairs.remove(v0_info.pairs.find(pair));
    v1_info.pairs.remove(v1_info.pairs.find(pair));

    if (pair->isInHeap())
        m_heap->kill(pair->getHeapPos());
}

/*---------------------------------------------------------------------------------**//**
*   Decimation process including:
*        - PairIsValid
*        - ComputePairInfoAndUpdateHeap
*        - DoContract
+---------------+---------------+---------------+---------------+---------------+------*/
bool GarlandMeshFilter::PairIsValid(Vertex *u, Vertex *v)
{
    return norm2(*u - *v) < m_proximityLimit;
}

int GarlandMeshFilter::PredictFace(Face& F, Vertex *v1, Vertex *v2, Vec3& vnew, Vec3& f1, Vec3& f2, Vec3& f3)
{
    int nmapped = 0;

    if (F.GetVertex(0) == v1 || F.GetVertex(0) == v2)
    {
        f1 = vnew;  nmapped++;
    }
    else f1 = *F.GetVertex(0);

    if (F.GetVertex(1) == v1 || F.GetVertex(1) == v2)
    {
        f2 = vnew;  nmapped++;
    }
    else f2 = *F.GetVertex(1);

    if (F.GetVertex(2) == v1 || F.GetVertex(2) == v2)
    {
        f3 = vnew;  nmapped++;
    }
    else f3 = *F.GetVertex(2);

    return nmapped;
}

double GarlandMeshFilter::PairMeshPenalty(Vertex* v1, Vertex* v2, Vec3& vnew)
{
    static face_buffer changed;
    changed.reset();

    m_model->ContractionRegion(v1, v2, changed);

    // double Nsum = 0;
    double Nmin = 0;

    for (int i = 0; i<changed.length(); i++)
    {
        Face& F = *changed(i);
        Vec3 f1, f2, f3;

        int nmapped = PredictFace(F, v1, v2, vnew, f1, f2, f3);

        // Only consider non-degenerate faces
        if (nmapped < 2)
        {
            Plane Pnew(f1, f2, f3);
            double delta = Pnew.Normal() * F.Plane().Normal();

            if (Nmin > delta) Nmin = delta;
        }
    }
    //return (-Nmin) * MESH_INVERSION_PENALTY;
    if (Nmin < 0.0)
        return MESH_INVERSION_PENALTY;
    else
        return 0.0;
}

void GarlandMeshFilter::ComputePairInfoAndUpdateHeap(pair_info *pair)
{
    Vertex *v0 = pair->v0;
    Vertex *v1 = pair->v1;

    vert_info& v0_info = GetVertexInfo(v0);
    vert_info& v1_info = GetVertexInfo(v1);

    Mat4 Q = v0_info.Q + v1_info.Q;
    double norm = v0_info.norm + v1_info.norm;

    pair->cost = Quadrix_PairTarget(Q, v0, v1, pair->candidate, m_placementPolicy, m_preserveBoundaries);

    if (m_weightByArea)
        pair->cost /= norm;

    if (m_preserveMeshQuality)
        pair->cost += PairMeshPenalty(v0, v1, pair->candidate);

    // NOTICE:  In the heap we use the negative cost.  That's because
    //          the heap is implemented as a MAX heap.
    if (pair->isInHeap())
    {
        m_heap->update(pair, (float)-pair->cost);
    }
    else
    {
        m_heap->insert(pair, (float)-pair->cost);
    }
}

void GarlandMeshFilter::DoContract(pair_info *pair)
{
    Vertex *v0 = pair->v0;  
    Vertex *v1 = pair->v1;

    vert_info& v0_info = GetVertexInfo(v0);
    vert_info& v1_info = GetVertexInfo(v1);
    Vec3 vnew = pair->candidate;
    int i;

    // Makes v0 be the new vertex
    v0_info.Q += v1_info.Q;
    v0_info.norm += v1_info.norm;

    // Performs the actual contraction
    static face_buffer changed;
    changed.reset();
    m_model->Contract(v0, v1, vnew, changed); 

    // Removes the pair that we just contracted
    DeletePair(pair);

    // Recalculates pairs associated with v0
    for (i = 0; i<v0_info.pairs.length(); i++)
    {
        pair_info *p = v0_info.pairs(i);
        ComputePairInfoAndUpdateHeap(p);
    }

    // Process pairs associated with now dead vertex

    static pair_buffer condemned(6); // collect condemned pairs for execution
    condemned.reset();

    for (i = 0; i<v1_info.pairs.length(); i++)
    {
        pair_info *p = v1_info.pairs(i);

        Vertex *u;
        if (p->v0 == v1)
        {
            u = p->v1;
        }
        else if (p->v1 == v1)
        {
            u = p->v0;
        }
        else
        {
            throw "Decimate.DoContract: Bogus pair.\n";
        }

        if (!CheckForPair(u, v0))
        {
            p->v0 = v0;
            p->v1 = u;
            v0_info.pairs.add(p);
            ComputePairInfoAndUpdateHeap(p);
        }
        else
            condemned.add(p);
    }

    for (i = 0; i < condemned.length(); i++)
    {
        DeletePair(condemned(i));
    }        
    v1_info.pairs.reset(); // safety precaution
}

/*---------------------------------------------------------------------------------**//**
*   External interface: setup and single step iteration.
+---------------+---------------+---------------+---------------+---------------+------*/

pair_info* GarlandMeshFilter::GetPairForContraction()
{
    heap_node *top;
    pair_info *pair;
    bool validPair = false;

    top = m_heap->extractTop();
    if (!top) return (pair_info *)NULL;
    pair = (pair_info*)top->obj;
    validPair = pair->isValid();

    while (!validPair)
    {
        DeletePair(pair);
        top = m_heap->extractTop();
        if (!top) return (pair_info *)NULL;
        pair = (pair_info *)top->obj;
        validPair = pair->isValid();
    }
    return pair;   
}

bool GarlandMeshFilter::EvalDecimateQuadric(Vertex *v, Mat4& Q)
{
    if (m_verticeInfoArray.length() > 0)
    {
        Q = m_verticeInfoArray(v->uniqID).Q;
        return true;
    }
    else
    {
        return false;
    } 
}

double GarlandMeshFilter::EvalDecimateError(Vertex *v)
{
    vert_info& info = GetVertexInfo(v);
    double err = Quadrix_EvaluateVertex(*v, info.Q);

    if (m_weightByArea)
    {
        err /= info.norm;
    }
    return err;
}

double GarlandMeshFilter::EvalDecimateMaxError()
{
    double max_err = 0;

    for (int i = 0; i < m_model->InitialVerticeCount(); i++)
    {
        if (m_model->GetVertex(i)->IsValid())
        {
            max_err = max(max_err, EvalDecimateError(m_model->GetVertex(i)));
        }
    }      
    return max_err;
}

// Creates the heap and initializes attributes 
// used to order the vertex in the heap.
// m_pairSelectionTolerance has to be set at this point.
void GarlandMeshFilter::InitializeHeap()
{
    int i, j;
    m_verticeInfoArray.init(m_model->InitialVerticeCount());

    // Distributing shape constraints.
    if (m_useVertexConstraint)
        for (i = 0; i<m_model->InitialVerticeCount(); i++)
        {
        Vertex *v = m_model->GetVertex(i);
                if (v->IsValid())
                        GetVertexInfo(v).Q = Quadrix_GetVertexConstraint(*v);
        }

    for (i = 0; i<m_model->InitialFaceCount(); i++)
        if (m_model->GetFace(i)->IsValid())
        {
            if (m_usePlaneConstraint)
            {
                Mat4 Q = Quadrix_GetPlaneConstraint(*m_model->GetFace(i));
                double norm = 0.0;

                if (m_weightByArea)
                {
                    norm = m_model->GetFace(i)->Area();
                    Q *= norm;
                }
                for (j = 0; j<3; j++)
                {
                    GetVertexInfo(m_model->GetFace(i)->GetVertex(j)).Q += Q;
                    GetVertexInfo(m_model->GetFace(i)->GetVertex(j)).norm += norm;
                }
            }
        }

    if (m_constrainBoundaries)
    {
        // Accumulates discontinuity constraints.
        for (i = 0; i<m_model->InitialEdgeCount(); i++)
            if (m_model->GetEdge(i)->IsValid() && Quadrix_CheckForDiscontinuity(m_model->GetEdge(i)))
            {
                Mat4 B = Quadrix_GetDiscontinuityConstraint(m_model->GetEdge(i));
                double norm = 0.0;

                if (m_weightByArea)
                {
                    norm = norm2(*m_model->GetEdge(i)->Org() - *m_model->GetEdge(i)->Dest());
                    B *= norm;
                }
                B *= m_boundaryConstraintWeight;
                GetVertexInfo(m_model->GetEdge(i)->Org()).Q += B;
                GetVertexInfo(m_model->GetEdge(i)->Org()).norm += norm;
                GetVertexInfo(m_model->GetEdge(i)->Dest()).Q += B;
                GetVertexInfo(m_model->GetEdge(i)->Dest()).norm += norm;
            }
    }

    // Sets the size of the heap.
    m_heap->resize(m_model->currentEdgeCount);
    int pair_count = 0;

    // Collectes pairs [edges].
    for (i = 0; i<m_model->InitialEdgeCount(); i++)
        if (m_model->GetEdge(i)->IsValid())
        {
            pair_info *pair = CreateNewPair(m_model->GetEdge(i)->Org(), m_model->GetEdge(i)->Dest());
            ComputePairInfoAndUpdateHeap(pair);
            pair_count++;
        }

    // The next 2 if are about creating pairs based on vertices 
    // that are not initially connected by an edge.
    //-------------------------------------------------------------
    if (m_pairSelectionTolerance<0)
    {
        // Auto-limit at 5% of model radius.
        m_pairSelectionTolerance = m_model->bounds.radius * 0.05;
    }
    m_proximityLimit = m_pairSelectionTolerance * m_pairSelectionTolerance;
    if (m_proximityLimit > 0)
    {
        // Collecting pairs based on the limit.
        ProximityGrid grid(m_model->bounds.minVec, m_model->bounds.maxVec, m_pairSelectionTolerance);
        for (i = 0; i<m_model->InitialVerticeCount(); i++)
            grid.AddPoint(m_model->GetVertex(i));

        buffer<Vec3 *> nearby(32);
        for (i = 0; i<m_model->InitialVerticeCount(); i++)
        {
            nearby.reset();
            grid.ProximalPoints(m_model->GetVertex(i), nearby);

            for (j = 0; j<nearby.length(); j++)
            {
                Vertex *v1 = m_model->GetVertex(i);
                Vertex *v2 = (Vertex *)nearby(j);

                if (v1->IsValid() && v2->IsValid())
                {
                    if (!CheckForPair(v1, v2))
                    {
                        pair_info *pair = CreateNewPair(v1, v2);
                        ComputePairInfoAndUpdateHeap(pair);
                        pair_count++;
                    }
                }
            }
        }
    }
    else
    {
        // Ignoring non-edge pairs [limit=0].
    }
}

/*---------------------------------------------------------------------------------**//**
*   Save the current model as an obj file.
+---------------+---------------+---------------+---------------+---------------+------*/
void GarlandMeshFilter::OutputModelToFile(string outputFilepath)
{
    m_model->OutputToFile(outputFilepath);
}

/*---------------------------------------------------------------------------------**//**
*   Save a mesh passed as parameter to an obj file.
+---------------+---------------+---------------+---------------+---------------+------*/
void GarlandMeshFilter::OutputMeshToFile(string fileName,
                                         size_t nbVertices, const DPoint3d* vertices,
                                         size_t nbFaceIndexes, const int32_t* faceIndexes)
{      
    size_t faceCount = nbFaceIndexes / 3; 
    ofstream out;
    out.open(fileName);

    if (out)
    {
        out << "# OBJ file format with extension .obj" << endl;
        out << "# vertex count = " << nbVertices << endl;
        out << "# face count = " << faceCount << endl;
    
        // Vertices
        for (size_t i = 0; i < nbVertices; i++)
        {
            out << "v " << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
        }
        // Faces
        for (size_t j = 0; j < nbFaceIndexes; j += 3)
        {
            if ((j + 2) <  nbFaceIndexes)
            {
                out << "f " << faceIndexes[j] << " " << faceIndexes[j+1] << " " << faceIndexes[j+2] << endl;
            }
        }
        out.close();
    }
}

/*---------------------------------------------------------------------------------**//**
*   Create a report about faces and points and their validity in the current model.
+---------------+---------------+---------------+---------------+---------------+------*/
string GarlandMeshFilter::AnalyseModelFacesAndVertices()
{
    int currentFaceCount = 0;
    int invalidFaceCount = 0;
    int invalidVerticeCount = 0;
    int validVerticeCount = 0;
    int totalFaceCount = 0;
    int totalVerticeCount = 0;
    float percentageFaceInvalid = 0.0;
    float percentageVerticeInvalid = 0.0;
    string s = "";
  
    // Face Analysis
    //------------------
    s += "\n----------------------------------------------------------------\n";
    s += "Faces: \n";
    s += "----------------------------------------------------------------\n\n";

    for (int i = 0; i < m_model->InitialFaceCount(); i++)
    {
        totalFaceCount++;
        if (!m_model->GetFace(i)->Plane().IsValid()) // Get invalid faces
        {       
            invalidFaceCount++;     
            Vec3 normal = m_model->GetFace(i)->Plane().Normal();
            Vertex* v1 = m_model->GetFace(i)->GetVertex(0);
            Vertex* v2 = m_model->GetFace(i)->GetVertex(1);
            Vertex* v3 = m_model->GetFace(i)->GetVertex(2);
                
            s += "F (INVALID) index: "; s += to_string(i+1); 
            s += " Normal: "; s += StrNorm(normal);
            s += " V indexes: "; s += StrVertIdx(v1, v2, v3);
            s += " V1 ("; s += StrValid(v1->IsValid()); s += "): "; s += StrCoord(v1);
            s += " V2 ("; s += StrValid(v2->IsValid()); s += "): "; s += StrCoord(v2);
            s += " V3 ("; s += StrValid(v3->IsValid()); s += "): "; s += StrCoord(v3); s += "\n";
        }
        else 
        {
            /*currentFaceCount++;  
            Vertex* v1 = m_model->GetFace(i)->GetVertex(0);
            Vertex* v2 = m_model->GetFace(i)->GetVertex(1);
            Vertex* v3 = m_model->GetFace(i)->GetVertex(2);

            s += "F (VALID) index: "; s += to_string(i + 1);
            s += " V indexes: "; s += StrVertIdx(v1,v2, v3);  
            s += " V1 ("; s += StrValid(v1->IsValid()); s += "): "; s += StrCoord(v1);
            s += " V2 ("; s += StrValid(v2->IsValid()); s += "): "; s += StrCoord(v2);
            s += " V3 ("; s += StrValid(v3->IsValid()); s += "): "; s += StrCoord(v3); s += "\n";*/
        }
    }

    // Vertex Analysis
    //------------------
    s += "\n----------------------------------------------------------------\n";
    s += "Vertices: \n";
    s += "----------------------------------------------------------------\n\n";

    for (int i = 0; i < m_model->InitialVerticeCount(); i++)
    {
        totalVerticeCount++;
        if (m_model->GetVertex(i)->EdgeUses().length() == 0) // Get invalid vertices
        {
            invalidVerticeCount++;
            Vertex* v = m_model->GetVertex(i);
            s += "V (INVALID) index: "; s += StrCoord(v); s += "\n";
        }
        else
        {
            /*validVerticeCount++;
            s += "V (VALID) index: "; s += to_string(i+1); s += "\n";*/
        }
    }

    // Summary
    //------------------
    s += "\n----------------------------------------------------------------\n";
    s += "Summary: \n";
    s += "----------------------------------------------------------------\n\n";

    percentageFaceInvalid = (float) invalidFaceCount * 100 / totalFaceCount;
    percentageVerticeInvalid = (float) invalidVerticeCount * 100 / totalVerticeCount;

    s += "Invalid Face Count: "; s += to_string(invalidFaceCount); s += "\n";
    s += "Valid Face Count: "; s += to_string(currentFaceCount); s += "\n";
    s += "Total Face Count: "; s += to_string(totalFaceCount); s += "\n";
    s += "Percentage invalid: "; s += to_string(percentageFaceInvalid); s += "\n";
    s += "\n" ;
    s += "Invalid Vertice Count: "; s += to_string(invalidVerticeCount); s += "\n";
    s += "Valid Vertice Count: "; s += to_string(validVerticeCount); s += "\n";
    s += "Total Vertice Count: "; s += to_string(totalVerticeCount); s += "\n";
    s += "Percentage invalid: "; s += to_string(percentageVerticeInvalid); s += "\n";

    return s;
}

string GarlandMeshFilter::StrValid(bool validity)
{
    if (validity) 
    { 
        return "VALID"; 
    }
    else 
    { 
        return "INVALID"; 
    }
}

string GarlandMeshFilter::StrCoord(Vertex* v)
{
    string x = to_string((*v)[X]); string y = to_string((*v)[Y]); string z = to_string((*v)[Z]);
    string s = "x: "; s += x; s += " y: "; s += y; s += " z: "; s += z; 
    return s;
}

string GarlandMeshFilter::StrVertIdx(Vertex* v1, Vertex* v2, Vertex* v3)
{
    string v1id = to_string(v1->uniqID);    
    string v2id = to_string(v2->uniqID);    
    string v3id = to_string(v3->uniqID);
    string s = v1id; s += " "; s += v2id; s += " "; s += v3id;
    return s;
}

string GarlandMeshFilter::StrNorm(Vec3& normal)
{
    string nX = to_string(normal[X]);       
    string nY = to_string(normal[Y]);       
    string nZ = to_string(normal[Z]);
    string s = "x: "; s += nX; s += " y: "; s += nY; s += " z: "; s += nZ;
    return s;
}

void GarlandMeshFilter::AnalyseCurrentFacesAndVertices()
{
    m_report = AnalyseModelFacesAndVertices();
}

void GarlandMeshFilter::SaveFaceAndVertexAnalysis(string meshName, string reportName, string report)
{
    string s;
    s += "################################################################\n";
    s += "#\n";
    s += "# Analysis of faces and vertices. \n";
    s += "# Input mesh: "; s += meshName; s += "\n";
    s += "#\n";
    s += "################################################################\n";

    string finalReport = s;
    finalReport += report;

    ofstream out;
    out.open(reportName);

    if (out)
    {
        out << finalReport;
        out.close();
    }
}

//  Assumes that m_report contains already a report.
void GarlandMeshFilter::SaveCurrentFaceAndVertexAnalysis(string meshName, string reportName)
{
    SaveFaceAndVertexAnalysis(meshName, reportName, m_report);
}

void GarlandMeshFilter::AnalyseFacesAndVerticesAndSaveItToFile(string meshName, string reportName)
{
    string report = AnalyseModelFacesAndVertices();
    SaveFaceAndVertexAnalysis(meshName, reportName, report);
    m_report = "";
}

string GarlandMeshFilter::GetReasonForProcessTermination()
{
    string reason = "Unknown stop condition.";
    if (m_reachedTargetStop)
    {
        reason = "The process reached the targeted vertex count.";
    }
    else if (m_unavailablePairStop)
    {
        reason = "The process stopped before the end because of not finding available pairs.";
    }
    else if (m_overErrorToleranceStop)
    {
        reason = "The process stopped before the end in overpassing the error tolerance.";
    }
    return reason;
}

void GarlandMeshFilter::SaveReasonForProcessTermination(string meshName, string fileName)
{
    string reason = GetReasonForProcessTermination();
    string s;
    s += "################################################################\n";
    s += "#\n";
    s += "# Analysis of Process Termination. \n";
    s += "# Simplified mesh: "; s += meshName; s += "\n";
    s += "#\n";
    s += "################################################################\n\n";

    s += reason;
    int discrepancy = abs(m_targetNumberVertices - m_model->CurrentVerticeCount());

    s += "\nStopped at iteration number: "; s += to_string(m_loopStopCount);
    s += "\n\nVertex count at stop: "; s += to_string(m_model->CurrentVerticeCount());
    s += "\nExpected vertex count at stop:"; s += to_string(m_targetNumberVertices);
    s += "\nDifference between expectation and real vertex count: "; s += to_string(discrepancy); s += " vertices";
    s += "\n\nFace count at stop: "; s += to_string(m_model->CurrentFaceCount());

    ofstream out;
    out.open(fileName);
    if (out)
    {
        out << s;
        out.close();
    }
}

void GarlandMeshFilter::DefineSimplificationProcessStopCondition(int currrentVertexCount, int targetNbVertices,
                                                                bool availablePairsInHeap, bool errorTolerated)
{   
    if (!availablePairsInHeap)
    {
        m_unavailablePairStop = true;
    }
    else if (!errorTolerated)
    {
        m_overErrorToleranceStop = true;
    }
    else
    {
        m_reachedTargetStop = true;
    }
}

void GarlandMeshFilter::DumpHeap(string fileName)
{
    string heapInfo = "";
    for (int i = 0; i < m_heap->size; i++)
    {
        heapInfo += OutputHeapNodeInfo(m_heap->ref(i), i);
    }
    ofstream out;
    out.open(fileName);
    if (out)
    {
        out << heapInfo;
        out.close();
    }
}

string GarlandMeshFilter::OutputHeapNodeInfo(heap_node& n, int nodeIndex)
{
    pair_info * pair = (pair_info *)n.obj;
    Vec3 c = pair->candidate;
    double cost = pair->cost;
    int position = pair->getHeapPos();
    bool inHeap = (pair->isInHeap()) ? true : false;
    bool isValid = pair->isValid();
    Vertex* v0 = pair->v0;
    Vertex* v1 = pair->v1;
    string s;

    s += "\n//-------------------------------------------------------------------------------------\n";
    s += "// Pair "; s += to_string(nodeIndex); s += "\n";
    s += "//-------------------------------------------------------------------------------------\n";
    s += "Position: "; s += to_string(position); s += " Cost: "; s += to_string(cost); s += "\n";   
    s += "Validity: "; StrValid(isValid);  s += " PairInHeap: "; s += to_string(inHeap); s += "\n";
    s += "Vertex 0 (id: "; s += to_string(v0->uniqID); s += "): "; 
    s += StrCoord(v0); s += " validity: "; s += to_string(v0->IsValid());  s += "\n";
    s += "Vertex 1 (id: "; s += to_string(v1->uniqID); s += "): "; 
    s += StrCoord(v1); s += " validity: "; s += to_string(v1->IsValid());  s += "\n";
    return s;
}





