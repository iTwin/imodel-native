/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/GeoCoords/Transformation.hpp $
|    $RCSfile: Transformation.hpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/10/20 18:47:45 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*__PUBLISH_SECTION_START__*/


/*---------------------------------------------------------------------------------**//**
* @description     
* @bsiclass                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct TransfoMatrix::RowProxy
    {
private:
    friend struct                       TransfoMatrix;
    double*                             m_rowP;
    explicit                            RowProxy                           (double*                     rowP)
        :   m_rowP(rowP) 
        {
        }

public:
    double                              operator[]                         (size_t                      idx) const
        { 
        assert(idx < 4); 
        return m_rowP[idx];
        }
    double&                             operator[]                         (size_t                      idx)
        { 
        assert(idx < 4); 
        return m_rowP[idx];
        }
    };

inline const TransfoMatrix& TransfoMatrix::GetIdentity ()
    {
    static const double IDENTITY_PARAMS[3][4] = {{1.0, 0.0, 0.0, 0.0},
                                                 {0.0, 1.0, 0.0, 0.0},
                                                 {0.0, 0.0, 1.0, 0.0}};
    static const TransfoMatrix IDENTITY(IDENTITY_PARAMS);
    return IDENTITY;
    }

inline TransfoMatrix::TransfoMatrix ()
    {
    memcpy(m_parameters[0], GetIdentity().m_parameters[0], 12 * sizeof(double));
    }

inline TransfoMatrix::TransfoMatrix (const double parameters[][4])
    {
    memcpy(m_parameters[0], parameters[0], 12 * sizeof(double));
    }


inline TransfoMatrix::TransfoMatrix    (double r0c0, double r0c1, double r0c2, double r0c3,
                                        double r1c0, double r1c1, double r1c2, double r1c3,
                                        double r2c0, double r2c1, double r2c2, double r2c3)
    {
        const double parameters[3][4] = {{r0c0, r0c1, r0c2, r0c3},
                                         {r1c0, r1c1, r1c2, r1c3},
                                         {r2c0, r2c1, r2c2, r2c3}};
    memcpy(m_parameters[0], parameters[0], 12 * sizeof(double));
    }

inline TransfoMatrix::CRowProxy TransfoMatrix::operator[] (size_t idx) const
    {
    assert(idx < 3);
    return m_parameters[idx];
    }

inline TransfoMatrix::RowProxy TransfoMatrix::operator[] (size_t idx)
    {
    assert(idx < 3);
    return RowProxy(m_parameters[idx]);
    }


inline DPoint3d    operator*   (const TransfoMatrix&    lhs,
                                const DPoint3d&         rhs)
    {
    DPoint3d result =  {lhs[0][0]*rhs.x + lhs[0][1]*rhs.y + lhs[0][2]*rhs.z + lhs[0][3],
                        lhs[1][0]*rhs.x + lhs[1][1]*rhs.y + lhs[1][2]*rhs.z + lhs[1][3], 
                        lhs[2][0]*rhs.x + lhs[2][1]*rhs.y + lhs[2][2]*rhs.z + lhs[2][3]};

    return result;
    }
