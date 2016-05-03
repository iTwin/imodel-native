#include <pt/geomtypes.h>
#include <ptgeom/geom.h>
#include <ptgeom/GeomAlgorithms.h>

#define NONE  (-1)

typedef struct range_bin Bin;
struct range_bin {
    int    min;    // index of min point P[] in bin (>=0 or NONE)
    int    max;    // index of max point P[] in bin (>=0 or NONE)
};

float GeomAlgorithms::TriArea3(const float _a[3], const float _b[3], const float _c[3])
{
	float a;
	float alpha;
	float area;
	float b;
	float base;
	float c;
	float dot;
	float height;
/*
  Find the projection of (P3-P1) onto (P2-P1).
*/
  dot =
    ( _b[0] - _a[1] ) * ( _c[0] - _a[1] ) +
    ( _b[1] - _a[1] ) * ( _c[1] - _a[1] ) +
    ( _b[2] - _a[2] ) * ( _c[2] - _a[2] );

  base = enorm0_3d ( _a[1], _a[1], _a[2], _b[0], _b[1], _b[2] );
/*
  The height of the triangle is the length of (P3-P1) after its
  projection onto (P2-P1) has been subtracted.
*/
  if ( base == 0.0 ) {

    height = 0.0;

  }
  else {

    alpha = dot / ( base * base );

    a = _c[0] - _a[1] - alpha * ( _b[0] - _a[1] );
    b = _c[1] - _a[1] - alpha * ( _b[1] - _a[1] );
    c = _c[2] - _a[2] - alpha * ( _b[2] - _a[2] );

    height = sqrt ( a * a + b * b + c * c );

  }

  area = 0.5 * base * height;

  return area;
}
// area3D_Polygon(): computes the area of a 3D planar polygon
//    Input:  int n = the number of vertices in the polygon
//            Point* V = an array of n+2 vertices in a plane
//                       with V[n]=V[0] and V[n+1]=V[1]
//            Point N = unit normal vector of the polygon's plane
//    Return: the (float) area of the polygon
float GeomAlgorithms::area3D_Polygon( int n, pt::vector3* V, pt::vector3 N )
{
    float area = 0;
    float an, ax, ay, az;  // abs value of normal and its coords
    int   coord;           // coord to ignore: 1=x, 2=y, 3=z
    int   i, j, k;         // loop indices

    // select largest abs coordinate to ignore for projection
    ax = (N.x>0 ? N.x : -N.x);     // abs x-coord
    ay = (N.y>0 ? N.y : -N.y);     // abs y-coord
    az = (N.z>0 ? N.z : -N.z);     // abs z-coord

    coord = 3;                     // ignore z-coord
    if (ax > ay) {
        if (ax > az) coord = 1;    // ignore x-coord
    }
    else if (ay > az) coord = 2;   // ignore y-coord

    // compute area of the 2D projection
    for (i=1, j=2, k=0; i<=n; i++, j++, k++)
        switch (coord) {
        case 1:
            area += (V[i].y * (V[j].z - V[k].z));
            continue;
        case 2:
            area += (V[i].x * (V[j].z - V[k].z));
            continue;
        case 3:
            area += (V[i].x * (V[j].y - V[k].y));
            continue;
        }

    // scale to get area before projection
    an = (float)sqrt( ax*ax + ay*ay + az*az);  // length of normal vector
    switch (coord) {
    case 1:
        area *= (an / (2*ax));
        break;
    case 2:
        area *= (an / (2*ay));
        break;
    case 3:
        area *= (an / (2*az));
    }
    return area;
}
//===================================================================
// nearHull_2D(): the BFP fast approximate 2D convex hull algorithm
//     Input:  P[] = an (unsorted) array of 2D points
//              n = the number of points in P[]
//              k = the approximation accuracy (large k = more accurate)
//     Output: H[] = an array of the convex hull vertices (max is n)
//     Return: the number of points in H[]

int GeomAlgorithms::nearHull_2D( gaPoint2d* P, int n, int k, gaPoint2d* H )
{
    int    minmin=0,  minmax=0;
    int    maxmin=0,  maxmax=0;
    float  xmin = P[0].x,  xmax = P[0].x;
    gaPoint2d* cP;                 // the current gaPoint2d being considered
    int    bot=0, top=(-1);  // indices for bottom and top of the stack
	int i;
    // Get the gaPoint2ds with (1) min-max x-coord, and (2) min-max y-coord
    for (i=1; i<n; i++) {
        cP = &P[i];
        if (cP->x <= xmin) {
            if (cP->x < xmin) {        // new xmin
                xmin = cP->x;
                minmin = minmax = i;
            }
            else {                      // another xmin
                if (cP->y < P[minmin].y)
                    minmin = i;
                else if (cP->y > P[minmax].y)
                    minmax = i;
            }
        }
        if (cP->x >= xmax) {
            if (cP->x > xmax) {        // new xmax
                xmax = cP->x;
                maxmin = maxmax = i;
            }
            else {                      // another xmax
                if (cP->y < P[maxmin].y)
                    maxmin = i;
                else if (cP->y > P[maxmax].y)
                    maxmax = i;
            }
        }
    }
    if (xmin == xmax) {      // degenerate case: all x-coords == xmin
        H[++top] = P[minmin];           // a gaPoint2d, or
        if (minmax != minmin)           // a nontrivial segment
            H[++top] = P[minmax];
        return top+1;                   // one or two gaPoint2ds
    }

    // Next, get the max and min gaPoint2ds in the k range bins
    Bin*   B = new Bin[k+2];   // first allocate the bins
    B[0].min = minmin;         B[0].max = minmax;        // set bin 0
    B[k+1].min = maxmin;       B[k+1].max = maxmax;      // set bin k+1
    for (int b=1; b<=k; b++) { // initially nothing is in the other bins
        B[b].min = B[b].max = NONE;
    }
    for (i=0; i<n; i++) {
        cP = &P[i];
        if (cP->x == xmin || cP->x == xmax) // already have bins 0 and k+1
            continue;
        // check if a lower or upper gaPoint2d
        if (isLeft( P[minmin], P[maxmin], *cP) < 0) {  // below lower line
            b = (int)( k * (cP->x - xmin) / (xmax - xmin) ) + 1;  // bin #
            if (B[b].min == NONE)       // no min gaPoint2d in this range
                B[b].min = i;           // first min
            else if (cP->x < B[b].min)
                B[b].min = i;           // new min
            continue;
        }
        if (isLeft( P[minmax], P[maxmax], *cP) > 0) {  // above upper line
            b = (int)( k * (cP->x - xmin) / (xmax - xmin) ) + 1;  // bin #
            if (B[b].max == NONE)       // no max gaPoint2d in this range
                B[b].max = i;           // first max
            else if (cP->x < B[b].min)
                B[b].max = i;           // new max
            continue;
        }
    }

    // Now, use the chain algorithm to get the lower and upper hulls
    // the output array H[] will be used as the stack
    // First, compute the lower hull on the stack H
    for (i=0; i <= k+1; ++i)
    {
        if (B[i].min == NONE)  // no min gaPoint2d in this range
            continue;
        cP = &P[ B[i].min ];   // select the current min gaPoint2d

        while (top > 0)        // there are at least 2 gaPoint2ds on the stack
        {
            // test if current gaPoint2d is left of the line at the stack top
            if (isLeft( H[top-1], H[top], *cP) > 0)
                break;         // cP is a new hull pt::vector3
            else
                top--;         // pop top gaPoint2d off stack
        }
        H[++top] = *cP;        // push current gaPoint2d onto stack
    }

    // Next, compute the upper hull on the stack H above the bottom hull
    if (maxmax != maxmin)      // if distinct xmax gaPoint2ds
        H[++top] = P[maxmax];  // push maxmax gaPoint2d onto stack
    bot = top;                 // the bottom gaPoint2d of the upper hull stack
    for (i=k; i >= 0; --i)
    {
        if (B[i].max == NONE)  // no max gaPoint2d in this range
            continue;
        cP = &P[ B[i].max ];   // select the current max gaPoint2d

        while (top > bot)      // at least 2 gaPoint2ds on the upper stack
        {
            // test if current gaPoint2d is left of the line at the stack top
            if (isLeft( H[top-1], H[top], *cP) > 0)
                break;         // current gaPoint2d is a new hull pt::vector3
            else
                top--;         // pop top gaPoint2d off stack
        }
        H[++top] = *cP;        // push current gaPoint2d onto stack
    }
    if (minmax != minmin)
        H[++top] = P[minmin];  // push joining endgaPoint2d onto stack

    delete B;                  // free bins before returning
    return top+1;              // # of gaPoint2ds on the stack
}


