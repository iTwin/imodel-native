/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

static int s_trinomial [5][5][5] =
	{
	{   // DEGREE 0
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 1
            {1,1,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 2
            {1,2,1,0,0},
            {2,2,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 3
            {1,3,3,1,0},
            {3,6,3,0,0},
            {3,3,0,0,0},
            {1,0,0,0,0},
            {0,0,0,0,0},
        },
        {   // DEGREE 4
            {1,4,6,4,1},
            {4,12,12,4,0},  
            {6,12,6,0,0},
            {4,4,0,0,0},
            {1,0,0,0,0},
        },
	};

DPoint3d AddScaled (DPoint3dCP points, double *scales, size_t n)
	{
	DPoint3d xyz;
	xyz.Zero ();
	for (size_t i = 0; i < n; i++)
		{
		xyz.x += points[i].x * scales[i];
		xyz.y += points[i].y * scales[i];
		xyz.z += points[i].z * scales[i];
		}
	return xyz;	
	}
double BinomialCoefficientSafe (int n, int i)
	{
	if (i>-1 && i<n+1)
		{
		return s_trinomial[n][0][i];
		}
	else
		return 0.0;
	}

static int s_numPoints_crv [MAX_BEZIER_CURVE_ORDER_ForStruct + 1] = {0, 1, 2, 3, 4, 5};
int BezierCurveDPoint3d::GetOrder () const { return m_order;}
int BezierCurveDPoint3d::GetDegree () const { return m_order - 1;}
int BezierCurveDPoint3d::GetNumberPoints () const { return m_order; }

void BezierCurveDPoint3d::SetOrder (int order)
	{
	m_order = order;
	}

BezierCurveDPoint3d::BezierCurveDPoint3d ()
        : m_order (0)
    {
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (DPoint3dCR xyz0, DPoint3dCR xyz1, DPoint3dCR xyz2)
: m_order (2)
    {
    m_controlPoints[0] = xyz0;
    m_controlPoints[1] = xyz1;
    m_controlPoints[2] = xyz2;
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (int order)
	: m_order (order)
	{
	}

BezierCurveDPoint3d::BezierCurveDPoint3d (DPoint3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i] = points[i];
		}
    m_order = size;
	}

BezierCurveDPoint3d::BezierCurveDPoint3d (DVec3d *points, int size)
	{
	for (int i=0; i<size; i++)
		{
		m_controlPoints[i].x = points[i].x;
		m_controlPoints[i].y = points[i].y;
		m_controlPoints[i].z = points[i].z;
		}
    m_order = size;
	}

int BezierCurveDPoint3d::SetControlPoints (bvector<DPoint3d> const &controlPoints)
    {
    int order = (int) controlPoints.size ();
    m_order = order;
    for (int numPoints = s_numPoints_crv[order], i = 0; i < numPoints; i++)
        m_controlPoints[i] = controlPoints[i];
    return order;
    }

BezierCurveDPoint3d::BezierCurveDPoint3d (bvector<DPoint3d> const &controlPoints)
    {
    SetControlPoints (controlPoints);
    }

DPoint3d BezierCurveDPoint3d::GetPole (int i) const
	{
	return m_controlPoints[i];
	}

DPoint3d BezierCurveDPoint3d::GetPoleSafe (int i) const
	{
	if (i>-1 && i<m_order)
		{
		return m_controlPoints[i];
		}
	else
		return DPoint3d::From (0,0,0);
	}

void BezierCurveDPoint3d::SetPole (int i, DPoint3dCR xyz)
	{
	m_controlPoints[i] = xyz;
	}

BezierCurveDPoint3d BezierCurveDPoint3d::RaiseOrder () const
	{
	DPoint3d pnt, orig;
	orig = DPoint3d::From (0,0,0);
	BezierCurveDPoint3d newcurve;
	int ord = m_order;
	newcurve.m_order = ord+1;

	for (int i=0; i<ord+1; i++)
		{
		pnt = DPoint3d::FromSumOf (orig, GetPoleSafe (i-1), ((double)i)/ord, GetPoleSafe (i), (ord-(double)i)/ord);
		newcurve.SetPole (i, pnt);
		}

	return newcurve;
	}

BezierCurveDPoint3d BezierCurveDPoint3d::RaiseOrderTo (int order) const
	{
	BezierCurveDPoint3d newcurve[6];
	newcurve[m_order].m_order = m_order;

	for (int i=0; i<m_order; i++)
		{
		newcurve[m_order].m_controlPoints[i] = this->m_controlPoints[i];
		}

	for (int j=m_order; j<order; j++)
		{
		newcurve[j+1].m_order = j+1;
		newcurve[j+1] = newcurve[j].RaiseOrder ();
		}

	return newcurve[order];
	}

void BezierCurveDPoint3d::Multiply (TransformCR transform)
	{
	transform.Multiply (m_controlPoints, m_order);
	}

BezierCurveDPoint3d BezierCurveDPoint3d::FromMultiply (TransformCR transform, BezierCurveDPoint3dCR curve)
	{
	BezierCurveDPoint3d newCurve = curve;
	newCurve.Multiply (transform);
	return newCurve;
	}

DPoint3d BezierCurveDPoint3d::EvaluateDirect (double u) const
	{
	if (m_order>0)
		{
		double basfunc[5];
		ComputeBasisFunctions (u, basfunc);
		return AddScaled (m_controlPoints, basfunc, m_order);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}
	}

void BezierCurveDPoint3d::EvaluateDirect (double u, DPoint3dR xyz, DVec3dR dXdu) const
	{
	if (m_order>0)
		{
		double basfunc[5], basfuncu[5];
		ComputeBasisFunctions (u, basfunc, basfuncu);
		xyz = AddScaled (m_controlPoints, basfunc, m_order);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, m_order));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctions (double u, double *values) const
	{
	double v = 1.0-u;

	if (m_order == 2)
		{
		values[0] = v;
		values[1] = u;
		}
	else if (m_order == 3)
		{
		values[0] = v*v;
		values[1] = 2*u*v;
		values[2] = u*u; 
		}
	else if (m_order == 4)
		{
		double uu = u*u;
		double vv = v*v;

		values[0] = vv*v;
		values[1] = 3*vv*u;
		values[2] = 3*uu*v;
		values[3] = uu*u;
		}
	else if (m_order == 5)
		{
		double uu = u*u;
		double uuu = uu*u;
		double vv = v*v;
		double vvv = vv*v;

		values[0] = vvv*v;
		values[1] = 4*vvv*u;
		values[2] = 6*uu*vv;
		values[3] = 4*uuu*v;
		values[4] = uuu*u;
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctions (double u, double *values, double *ddu) const
	{
	double v = 1.0-u;

	if (m_order == 2)
		{
		values[0] = v;
		values[1] = u;

		ddu[0] = -1;
		ddu[1] = 1;
		}
	else if (m_order == 3)
		{
		values[0] = v*v;
		values[1] = 2*u*v;
		values[2] = u*u; 

		ddu[0] = -2*v;
		ddu[1] = 2*v - 2*u;
		ddu[2] = 2*u;
		}
	else if (m_order == 4)
		{
		double uu = u*u;
		double vv = v*v;
		double sxuv = 6*u*v;

		values[0] = vv*v;
		values[1] = 3*vv*u;
		values[2] = 3*uu*v;
		values[3] = uu*u;

		ddu[0] = -3*vv;
		ddu[1] = 3*vv - sxuv;
		ddu[2] = sxuv - 3*uu;
		ddu[3] = 3*uu;
		}
	else if (m_order == 5)
		{
		double uu = u*u;
		double uuu = uu*u;
		double vv = v*v;
		double vvv = vv*v;
		double twuuv = 12*uu*v;
		double twuvv = 12*u*vv;

		values[0] = vvv*v;
		values[1] = 4*vvv*u;
		values[2] = 6*uu*vv;
		values[3] = 4*uuu*v;
		values[4] = uuu*u;

		ddu[0] = -4*vvv;
		ddu[1] = 4*vvv - twuvv;
		ddu[2] = twuvv - twuuv;
		ddu[3] = twuuv - 4*uuu;
		ddu[4] = 4*uuu;
		}
	}

DPoint3d BezierCurveDPoint3d::EvaluateDirectCompact (double u) const
	{
	if (m_order>0)
		{
		double basfunc[5];
		ComputeBasisFunctionsCompact (u, basfunc);
		return AddScaled (m_controlPoints, basfunc, m_order);
		}
	else
		{
		return DPoint3d::From (0,0,0);
		}	
	}

void BezierCurveDPoint3d::EvaluateDirectCompact (double u, DPoint3dR xyz, DVec3dR dXdu) const
	{
	if (m_order>0)
		{
		double basfunc[5], basfuncu[5];
		ComputeBasisFunctionsCompact (u, basfunc, basfuncu);
		xyz = AddScaled (m_controlPoints, basfunc, m_order);
		dXdu = DVec3d::From (AddScaled (m_controlPoints, basfuncu, m_order));
		}
	else
		{
		xyz = DPoint3d::From (0,0,0);
		dXdu = DVec3d::FromStartEnd (DPoint3d::From (0,0,0), DPoint3d::From (0,0,0));
		}
	}

void BezierCurveDPoint3d::ComputeBasisFunctionsCompact (double u, double *values) const
        { 
		if (m_order>0)
			{
			double v = 1.0 - u;
			int n = m_order - 1;
			double valPower [2][5];

			valPower[0][0] =
				valPower[1][0] = 1;
	
			for (int p=0; p<n; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				}

			for (int i=0; i<m_order; i++)
				{
				values[i] = s_trinomial[n][0][i]*valPower[0][i]*valPower[1][n-i];
				}

				}
			else
				{
				}
        }

void BezierCurveDPoint3d::ComputeBasisFunctionsCompact (double u, double *values, double *ddu) const
        {
		if (m_order>0)
			{
			double v = 1.0 - u;
			int n = m_order - 1;
			double valPower [2][5];

			valPower[0][0] =
				valPower[1][0] = 1;
	
			for (int p=0; p<n; p++)
				{
				valPower[0][p+1]= valPower[0][p]*u;
				valPower[1][p+1]= valPower[1][p]*v;
				}

			for (int i=0; i<m_order; i++)
				{
				values[i] = s_trinomial[n][0][i]*valPower[0][i]*valPower[1][n-i];
				ddu[i] = n*(BinomialCoefficientSafe(n-1,i-1)*valPower[0][i-1]*valPower[1][n-i]-BinomialCoefficientSafe(n-1,i)*valPower[0][i]*valPower[1][n-1-i]);
				}

				}
			else
				{
				}
        }

void BezierCurveDPoint3d::InplaceDeCasteljau (double u)
	{
	double scales[2];
	DPoint3d points[2];
	scales[0] = u;
	scales[1] = 1.0 - u;

	for (int k=0;k<m_order-1;k++)
		{
		points[0] = m_controlPoints[k];
		points[1] = m_controlPoints[k+1];
		m_controlPoints[k] = AddScaled (points, scales, 2);
		}
	m_order--;
	}

void BezierCurveDPoint3d::NoOp (double u)
	{
	}

int BezierCurveDPoint3d::GetFacetCount (double chordTolerance, double angleTolerance, double maxEdgeLength)
	{
	DPoint4d inp [5];
	int np = GetNumberPoints ();

	for (int i=0; i<np; i++)
		{
		inp[i].x = m_controlPoints[i].x;
		inp[i].y = m_controlPoints[i].y;
		inp[i].z = m_controlPoints[i].z;
		inp[i].w = 1.0;
		}
	return bsiBezierDPoint4d_estimateEdgeCount (inp, np, chordTolerance, angleTolerance, maxEdgeLength, true);
	}