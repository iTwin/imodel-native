#pragma once

#define USEPREDICATES
#define IDL 0
#define _ITERATOR_DEBUG_LEVEL 0


template <typename _Index_type, typename _Function>
void parallel_for_ (_Index_type _First, _Index_type _Last, _Index_type _Step, const _Function& _Func)
    {
    for (; _First < _Last; _First += _Step) _Func (_First);
    }

template <typename _Index_type, typename _Function>
void parallel_for_each_ (_Index_type start, _Index_type end, const _Function& _Func)
    {
    for (_Index_type i = start; i != end; i++)
        _Func (*i);
    }

#include "vector"
const int numFacets = 4;
const int numEdges = 3;
typedef std::vector<DPoint3d> SMPointList;
typedef std::vector<int> SMPointIndexList;

#pragma region Helper Functions
#pragma endregion

struct Tetrahedron
    {
private:
    int m_adjTet[numFacets];
    public:
        int ptNums[numFacets];
        bool isActive;
        bool isChanged;
        bool special;
        char status;

    int GetAdjentTet (short pt) const
        {
        return m_adjTet[pt];
        }

    void SetAdjentTet (short pt, int value)
        {
        m_adjTet[pt] = value;
        }

    void DeleteTetrahedron ()
        {
        isActive = false;
        isChanged = false;
        special = false;
        status = 0;
        ptNums[0] = ptNums[1] = ptNums[2] = ptNums[3] = m_adjTet[0] =/* = m_adjTet[1] = m_adjTet[2] = m_adjTet[3] =*/ -1;
        }
    int GetPointIndex (int ptNum)  const
        {
        if (ptNums[0] == ptNum) return 0;
        if (ptNums[1] == ptNum) return 1;
        if (ptNums[2] == ptNum) return 2;
        if (ptNums[3] == ptNum) return 3;
        return -1;
        }

    static const int* GetDTetrahedron3dFacet (int facetNum)
        {
        static const int helper[][3] =
            {
                { 2, 1, 3 },
                { 3, 0, 2 },
                { 1, 0, 3 },
                { 0, 1, 2 }
            };
        return helper[facetNum];
        }
private:
    static void GetDTetrahedron3dFacet (int facetNum, int& a, int& b, int& c)
        {
        const int* helper = GetDTetrahedron3dFacet (facetNum);
        a = helper[0];
        b = helper[1];
        c = helper[2];
        }
            public:
        void AlignPt (int ptNum)
            {
            int ptIndex;
            for (ptIndex = 1; ptIndex < 3; ptIndex++)
                {
                if (ptNum == ptNums[ptIndex])
                    {
                    Tetrahedron t = *this;
                    int oldAdjTets[3];
                    oldAdjTets[0] = GetAdjentTet (2);
                    oldAdjTets[1] = GetAdjentTet (0);
                    oldAdjTets[2] = GetAdjentTet (1);

                    ptNums[0] = t.ptNums[ptIndex];
                    ptNums[1] = t.ptNums[(ptIndex + 1) % 3];
                    ptNums[2] = t.ptNums[(ptIndex + 2) % 3];

                    SetAdjentTet (2, oldAdjTets[(3 + ptIndex) % 3]);
                    SetAdjentTet (0, oldAdjTets[(4 + ptIndex) % 3]);
                    SetAdjentTet (1, oldAdjTets[(5 + ptIndex) % 3]);
#ifdef VALIDATION_CHECK
                    for (int f = 0; f < numFacets; f++)
                        {
                        int p[3];
                        GetFacePoints_ (f, p);
                        int a = t.GetFaceSide_ (p);
                        if (t.GetAdjentTet_(a) != GetAdjentTet_(f))
                            a = a;
                        }
#endif
                    return;
                    }
                }
            }
        Tetrahedron GetAdjustedTetForFace (int face) const // Around point 3.
            {
                const int ptHelper[numFacets][numFacets] =
                    {
                        { 2, 1, 3, 0 }, // 2
                        { 3, 0, 2, 1 }, // 3
                        { 1, 0, 3, 2 }, // 1
                        { 0, 1, 2, 3 }, // 0
                    };
                const int adjHelper[numFacets][numFacets] =
                    {
                        { 0, 3, 2, 1 }, // 2
                        { 1, 2, 3, 0 }, // 3
                        { 2, 3, 1, 0 }, // 1
                        { 3, 2, 0, 1 }, // 0
                    };

                Tetrahedron t;
                t.ptNums[0] = ptNums[ptHelper[face][0]];
                t.ptNums[1] = ptNums[ptHelper[face][1]];
                t.ptNums[2] = ptNums[ptHelper[face][2]];
                t.ptNums[3] = ptNums[ptHelper[face][3]];

                t.SetAdjentTet (3, GetAdjentTet (adjHelper[face][0])); // 1 0 3
                t.SetAdjentTet (2, GetAdjentTet (adjHelper[face][1])); // 0 1 2
                t.SetAdjentTet (0, GetAdjentTet (adjHelper[face][2])); // 3 0 2
                t.SetAdjentTet (1, GetAdjentTet (adjHelper[face][3])); // 2 1 3

#ifdef VALIDATECHECK
                for (int f = 0; f < numFacets; f++)
                    {
                    int p[3];
                    GetFacePoints (f, p);
                    int a = t.GetFaceSide (p);
                    if (t.GetAdjentTet(a) != GetAdjentTet(f))
                        a = a;
                    }
#endif
                return t;
                }
    void GetDTetrahedron3d (DTetrahedron3d& t, const SMPointList& pointList) const
        {
        t = DTetrahedron3d (pointList[ptNums[0]], pointList[ptNums[1]], pointList[ptNums[2]], pointList[ptNums[3]]);
        }

    template <class PL> void GetDTetrahedron3d (DTetrahedron3d& t, const PL& pointList) const
        {
        t = DTetrahedron3d (pointList[ptNums[0]], pointList[ptNums[1]], pointList[ptNums[2]], pointList[ptNums[3]]);
        }

    void GetFacePoints (int face, int facePts[3]) const
        {
        const int* a = GetDTetrahedron3dFacet (face);
        for (int i = 0; i < 3; i++)
            facePts[i] = ptNums[a[i]];
        }

    inline void ClearStatus ()
        {
        status = 0;
        }
    inline void SetFailedDelaunay (int vi)
        {
        status |= 1 << vi;
        }
    inline bool GetFailedDelaunay (int vi) const
        {
        return 0 != (status & (1 << vi));
        }
    inline void SetInternalOpp (int vi)
        {
        status |= 1 << (4 + vi);
        }
    inline void SetStatus(int vi)
        {
        status = vi;
        }
    inline bool GetInternalOpp (int vi) const
        {
        return 0 != (status & (1 << (4 + vi)));
        }

    // Returns 0 = same, 1 = same face, 2 = opposite face.
    static int IsFaceSame (int facePts1[3], int facePts2[3])
        {
        for (int i = 0; i < 3; i++)
            {
            if (facePts1[0] == facePts2[i])
                {
                if (facePts1[1] == facePts2[(i + 1) % 3] && facePts1[2] == facePts2[(i + 2) % 3])
                    return 1;
                if (facePts1[1] == facePts2[(i + 2) % 3] && facePts1[2] == facePts2[(i + 1) % 3])
                    return -1;
                }
            }
        return 0; // Not the same
        }

    int GetFaceSide (int findTet) const
        {
        for (int face = 0; face < numFacets; face++)
            {
            if (GetAdjentTet (face) == findTet)
                return face;
            }
        return -1;
        }
            int GetFaceSide (int testFacePts[3]) const
            {
            for (int face = 0; face < numFacets; face++)
                {
                int facePts[3];
                GetFacePoints (face, facePts);
                if (IsFaceSame (testFacePts, facePts) != 0)
                    return face;
                }
            return -1;
            }
        int GetFaceSide (int testFacePts[3], int test) const
            {
            for (int face = 0; face < numFacets; face++)
                {
                int facePts[3];
                GetFacePoints (face, facePts);
                if (IsFaceSame (testFacePts, facePts) == test)
                    return face;
                }
            return -1;
            }
        Tetrahedron ()
        {
        }

    Tetrahedron (int ptNum1, int ptNum2, int ptNum3, int ptNum4)
        {
        ptNums[0] = ptNum1;
        ptNums[1] = ptNum2;
        ptNums[2] = ptNum3;
        ptNums[3] = ptNum4;
        m_adjTet[0] = m_adjTet[1] = m_adjTet[2] = m_adjTet[3] = -1;
        isActive = true;
        isChanged = false;
        special = false;
        status = 0;
        }

    Tetrahedron (int ptNum1, int ptNum2, int ptNum3, int ptNum4, int adjTet1, int adjTet2, int adjTet3, int adjTet4)
        {
        ptNums[0] = ptNum1;
        ptNums[1] = ptNum2;
        ptNums[2] = ptNum3;
        ptNums[3] = ptNum4;
        SetAdjentTet(0, adjTet1);
        SetAdjentTet(1, adjTet2);
        SetAdjentTet(2, adjTet3);
        SetAdjentTet(3, adjTet4);
        isActive = true;
        isChanged = false;
        special = false;
        status = 0;
        }

     // Sets the points of a  tetrahedron
    inline void SetPointNums (int a, int b, int c, int d)
        {
        ptNums[0] = a;
        ptNums[1] = b;
        ptNums[2] = c;
        ptNums[3] = d;
        }
    inline void SetPointNums (const Tetrahedron& copyFrom)
        {
        ptNums[0] = copyFrom.ptNums[0];
        ptNums[1] = copyFrom.ptNums[1];
        ptNums[2] = copyFrom.ptNums[2];
        ptNums[3] = copyFrom.ptNums[3];
        }
    };

struct TetrahedronList : std::vector < Tetrahedron >
    {
    private:
        typedef std::vector<Tetrahedron> base;
    public:

        void resize (int size)
            {
            base::resize (size);
            }
        void resize (int size, const Tetrahedron& t)
            {
            base::resize (size, t);
            }

        void push_back (const Tetrahedron& t)
            {
            base::push_back (t);
            }
    };
struct FaceEdgeStack
    {
    int val;
    FaceEdgeStack ()
        {
        val = 1;
        }

    void Clear ()
        {
        val = 1;
        }
    void Push (int face, int edge)
        {
        val = (val << 4) | face | (edge << 2);
        }

    bool IsEmpty ()
        {
        return val == 1;
        }
    int GetFace ()
        {
        return val & 3;
        }
    int GetEdge ()
        {
        return (val >> 2) & 3;
        }
    void Pop ()
        {
        val >>= 4;
        }
    };

struct FaceStack
    {
    int val;
    FaceStack ()
        {
        val = 1;
        }

    void Clear ()
        {
        val = 1;
        }
    void Push (int face)
        {
        val = (val << 2) | face;
        }

    bool IsEmpty ()
        {
        return val == 1;
        }
    int GetFace ()
        {
        return val & 3;
        }
    void Pop ()
        {
        val >>= 2;
        }
    };


struct FindInsertPointHelper
    {
    DPoint3d center;
    double distanceSquared;
    };

struct FlipPointHelper
    {
    int tetras[2];
    };
struct FlipPointHelperList : std::vector < FlipPointHelper >
    {
    FlipPointHelperList (int size)
        {
        FlipPointHelper l;
        l.tetras[0] = -1;
        resize (size, l);
        }
    bool HasFlipped (int tn)
        {
        return this->operator[](tn).tetras[0] != -1;
        //            return this->find (tn) != this->end ();
        }

    void AddFlipped (int tn, int t1, int t2)
        {
        this->operator[](tn).tetras[0] = t1;
        this->operator[](tn).tetras[1] = t2;
        }
    inline const int* GetFlipped (int tn) const
        {
        const int* t = &this->operator[](tn).tetras[0];
        if (t[0] == -1)
            return nullptr;
        return t;
        }
    };
struct FlipInformation
    {
    enum FlipType
        {
        FlipNone,
        Flip23,
        Flip32,
        Flip44,
        };
    unsigned int info;
    //FlipInformation ()
    //    {
    //    info = 0;
    //    //flipType = FlipNone;
    //    }
    void Init (FlipType type, int flipFace, int faceEdge)
        {
        info = type | (flipFace << 2) | (faceEdge << 4);
        }
    int GetFlipType ()  const
        {
        return info & 3;
        }
    int GetFlipFace () const
        {
        return (info >> 2) & 3;
        }
    int GetFlipEdge () const
        {
        return (info >> 4) & 3;
        }

    // The things below can be calculated, flipTNum can if we have one of the FlipInformation per activeTetrahedron.
    int flipTNum;
    int otherTNum;
    int flip44TNum;
    };

//// MeshData

struct PointOnEdge
    {
    int ptNum;
    int tetraNum;
    int face;
    PointOnEdge ()
        {
        ptNum = -1;
        }
    PointOnEdge (int ptNum, int tetraNum, int face)
        {
        this->ptNum = ptNum;
        this->tetraNum = tetraNum;
        this->face = face;
        }
    };


struct MeshData
    {
    DRange3d m_extents;
    TetrahedronList m_tetrahedrons; // The Tetrahedrons.
    SMPointList m_points;           // The Points.
    int m_infPtNum;
    int m_ignorePtsAfterNum;

    inline bool IsInfPoint (int ptNum) const
        {
        return ptNum == m_infPtNum;
        }

    inline bool IgnorePoint (int ptNum) const
        {
        return (IsInfPoint (ptNum) || (m_ignorePtsAfterNum != -1 && ptNum >= m_ignorePtsAfterNum));
        }

#pragma region Helper Functions
    int WalkToTetrahedronWithPoint (int startTetrahedron, DPoint3dCR pt) const;
    int GetNextExitTetrahedronAlongRay (int startTetrahedron, int prevTet, DPoint3dCR startPt, DVec3dCR normal) const;
    void GetTetrahedronsAroundPoint (int tetraIndx, int ptNum, std::vector<int>& tetrahedrons) const;
    void CollectLinkedPoints (int tetraIndx, std::vector<int>& linkedPoints, int ptNum) const;
    void CollectLinkedPoints (int tetraIndx, bvector<PointOnEdge>& linkedPoints, int ptNum) const;
    void FindPointsAroundEdge (int tetraIndx, int ptA, int ptB, bvector<PointOnEdge>& otherPt) const;
    };
