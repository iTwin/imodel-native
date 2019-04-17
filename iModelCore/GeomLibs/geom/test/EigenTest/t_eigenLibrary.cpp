/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
//
//
#include <testHarness.h>
#include <Geom/XYRangeTree.h>
#include <Eigen/Dense>

USING_NAMESPACE_BENTLEY_GEOMETRY_INTERNAL
using namespace Eigen;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     Earlin.Lutz  10/17
+---------------+---------------+---------------+---------------+---------------+------*/
TEST (Eigen, HelloWorld)
    {
    MatrixXd m(3,3);
    for (size_t i = 0; i < 3; i++)
        for (size_t j = 0; j < 3; j++)
            m(i,j) = i == j ? 1.0 : 0.01 * (i + j);
    printf ("(trace %#.17g\n", m(0,0) + m(1,1) + m(2,2));
    
    JacobiSVD<MatrixXd> svd(m, ComputeThinU | ComputeThinV);
    
    printf ("Singular values\n");
    for (size_t i = 0; i < 3; i++)
        printf (" %#.17g", svd.singularValues()(i));
    printf ("\n");
    }
    
