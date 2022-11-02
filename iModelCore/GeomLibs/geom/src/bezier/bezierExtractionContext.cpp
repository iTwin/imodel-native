/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
//static double s_unitWeightRelTol = 1.0e-8;

/*---------------------------------------------------------------------------------**//**
* Computes the new pole of given index by multiplying the old poles by the
* row of the knot insertion matrix (obtained via Oslo algorithm) corresponding
* to the new knot of the same index.
*
* Assumptions:
* 1) knot vectors are valid (nondecreasing & satisfying clamped/periodic cond's)
* 2) pOldKnots[mu] <= pNewKnots[index] < pOldKnots[mu + 1] for some mu
* 3) pOldKnots must be defined for indices [mu - degree..mu + degree]
* 4) pNewKnots must be defined for indices [index..index + degree]
* 5) pOldPoles must be defined for indices [mu - degree..mu] that are nonnegative
*
* @param pNewPole   <= desired new pole
* @param index      => 0-based idx of new pole/knot in [0..numNewKnots - order]
* @param pOldPoles  => old homogeneous pole vector
* @param pNewKnots  => new knot vector
* @param pOldKnots  => old knot vector
* @param mu         => 0-based index (see 2 above) or -1 to compute
* @param order      => curve order (= degree + 1)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void        computeInsertedPoleWithOslo
(
        DPoint4d    *pNewPole,
        int         index,
const   DPoint4d    *pOldPoles,
const   double      *pNewKnots,
const   double      *pOldKnots,
        int         mu,
        int         order
)
    {
    double      T[MAX_BEZIER_CURVE_ORDER];      /* first order entries used */
    double      tKnot, term1, d1, d2, beta;
    int         readIndex, writeIndex;          /* indices into T */
    int         degree = order - 1;
    int         j, k;

    /*
    Compute new pole by using Oslo algorithm for computing discrete B-splines,
    which are the entries in the knot insertion matrix.  The Oslo algorithm is
    the recursion relation for discrete B-splines (undefined terms/knots are
    zero):

        j = (0- or 1-based) index into old knots tau[j] and old poles c[j]
        i = (0- or 1-based) index into new knots t[i]   and new poles d[i]
        k > 0

        alpha_{j,k}(i) = omega_{j,k}(t[i+k-1]) * alpha_{j,k-1}(i) + (1 - omega_{j+1,k}(t[i+k-1])) * alpha_{j+1,k-1}(i),

        omega_{j,k}(x) = (tau[j+k-1] > tau[j]) ? (x - tau[j]) / (tau[j+k-1] - tau[j]) : 0

        alpha_{j,1}(i) = (tau[j] <= t[i] < tau[j+1]) ? 1 : 0,

    We can write the new poles in terms of the old poles:

        d[i] = sum_{j=mu-degree}^{mu} alpha_{j,order}(i) c[j], where tau[mu] <= t[i] < tau[mu+1]

    In the knot insertion case, old knots tau are a subset of new knots t.
    Note the correspondences in the code with Lyche:

        j,k,mu          (same)
        index           i
        pNewKnots       t
        pOldKnots       tau
        tKnot           t_{i+k-1}
        term1           1st term of alpha_{j,k}(i)
        d2*beta         2nd term of alpha_{j,k}(i)
        d1*beta         1st term of alpha_{j+1,k}(i)
        d1+d2           denominator of omega_{j,k}(x)
        d1              numerator of omega_{j+1,k}(t[i+k-1])
        d2              numerator of 1 - omega_{j,k}(t[i+k-1])

    T is used to compute the discrete B-spline triangle determined by the
    recursion relation, with each column of the triangle overwriting the
    previous as entries from the previous column are read (both read/write are
    done from bottom up).  Here's an exploded view, where each entry depends
    on the entries to the west and northwest (blank entries are zero):

            column 1            column 2            ...     column order
         +--------------------------------------------------------------------
    T[0] |  alpha_{mu,1}(i)     alpha_{mu,2}(i)     ...     alpha_{mu,order}(i)
    T[1] |                      alpha_{mu-1,2)(i)   ...     alpha_{mu-1,order}(i)
     .   |                                          .           .
     .   |                                           .          .
     .   |                                            .         .
    T[d] |                                                  alpha_{mu-degree,order}(i)

    Reference: T. Lyche, "Discrete B-Splines and Conversion Problems," in
    _Computation of Curves and Surfaces_, W. Dahmen et al. (eds.), 1990.
    */

    /* find mu such that tau[mu] <= t[i] < tau[mu + 1] */
    if (mu < 0)
        for (mu = 0; ; mu++)
            if  (
                pOldKnots[mu] <= pNewKnots[index] &&
                pNewKnots[index] < pOldKnots[mu + 1]
                )
                break;

    /* column #1 has one entry = alpha_{mu,1}(i) */
    T[0] = 1.0;

    /* write column numbers 2 through order */
    for (k = 2; k <= order; k++)
        {
        term1 = 0.0;
        writeIndex = k - 1;
        readIndex = writeIndex - 1;
        tKnot = pNewKnots[index + writeIndex];

        // write column #k: mu - j = writeIndex is the row of T written
        // (add 1 to algorithmic j index to account for first knot used only first time globally)
        for (j = mu - writeIndex; j < mu ; j++)
            {
            // omega_{j,k}(x) = 0
            if (j + 1 < 0 || (d1 = tKnot - pOldKnots[j + 1]) + (d2 = pOldKnots[j + k] - tKnot) <= 0.0)
                {
                term1 = 0.0;
                d1 = 0.0;       // for next iteration
                d2 = 1.0;
                }
            beta = T[readIndex--] / (d1 + d2);
            T[writeIndex--] = term1 + d2 * beta;
            term1 = d1 * beta;
            }

        /* first entry of column #k = a_{mu,k}(i) */
        T[0] = term1;
        }

    /*
    All nonzero entries of the ith row of the knot insertion matrix are
    now contained in the top order entries of T.  Multiply T from bottom
    up by the old poles with indices (mu - degree)..mu to compute the
    i_th new pole.
    */
    pNewPole->x = pNewPole->y = pNewPole->z = pNewPole->w = 0.0;
    for (j = mu - degree, k = degree; j <= mu; j++, k--)
        if (j >= 0)
            {
            pNewPole->x += T[k] * pOldPoles[j].x;
            pNewPole->y += T[k] * pOldPoles[j].y;
            pNewPole->z += T[k] * pOldPoles[j].z;
            pNewPole->w += T[k] * pOldPoles[j].w;
            }
    }

/*---------------------------------------------------------------------------------**//**
* Finds the multiplicity of the given knot by looking ahead and behind in the
* knot sequence.  It is assumed that minMultIndex <= knotIndex <= maxMultIndex
* and that the knot sequence is nondecreasing.
*
* @param    pFirstMultIndex     <= index of first multiple found (or NULL)
* @param    pKnots              => full knot sequence
* @param    knotIndex           => index of knot to evaluate (must be valid)
* @param    minMultIndex        => smallest index of a multiple (must be valid)
* @param    maxMultIndex        => largest index of a multiple (must be valid)
* @param    knotTolerance       => absolute min distance separating unique knots
* @return multiplicity of the given knot (includes the knot itself)
* @see #createCoalescedLocalKnotVector
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     findKnotMultiplicity
(
        int     *pFirstMultIndex,
const   double  *pKnots,
        int     knotIndex,
        int     minMultIndex,
        int     maxMultIndex,
        double  knotTolerance

)
    {
    double  knot = pKnots[knotIndex];
    int     i, j;       /* knot indices just beyond/before multiple knot */

    /* check indices to right of knotIndex */
    for (i = knotIndex + 1; i <= maxMultIndex; i++)
        if (pKnots[i] - knot >= knotTolerance)
            break;

    /* check indices to left of knotIndex */
    for (j = knotIndex - 1; j >= minMultIndex; j--)
        if (knot - pKnots[j] >= knotTolerance)
            break;

    if (pFirstMultIndex)
        *pFirstMultIndex = j + 1;

    return i - j - 1;
    }

/*---------------------------------------------------------------------------------**//**
* Shifts invalid knot vector to enforce knot assumptions where possible.  Pole
* vector is similarly shifted to parallel the knot vector.
*
* @param    ppPoles         <=> B-spline poles
* @param    pNumPoles       <=> # B-spline poles
* @param    ppKnots         <=> full B-spline curve knots
* @param    pNumKnots       <=> # B-spline knots
* @param    relTolerance    => relative minimal distance separating unique knots/pole coords
* @param    order           => B-spline curve order (= degree + 1)
* @param    pbClosed        <=> true for closed curve, false for open
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    repointInvalidKnots
(
DPoint4d    **ppPoles,
int         *pNumPoles,
double      **ppKnots,
int         *pNumKnots,
double      relTolerance,
int         order,
bool        *pbClosed
)
    {
    double      knotTol, endKnotTol, *pK = *ppKnots;
    int         mult0, mult1, iFirst0, iFirst1, nInterior0, nInterior1;
    int         n = *pNumKnots;
    int         iInner0 = order, iInner1 = n - order - 1;   /* in (0,1) if norm'd */

    // null knot vector => uniform (OK)
    if (!pK)
        return;

    /* scale relative tolerance by knot interval size */
    knotTol = relTolerance * (pK[iInner1 + 1] - pK[iInner0 - 1]);

    // Check exterior knots for multiplicities > order.  Use larger tolerance to detect IGDS-era slop in "closed" curves.
    endKnotTol = knotTol * RELATIVE_BSPLINE_EXT_KNOT_TOLERANCE / RELATIVE_BSPLINE_KNOT_TOLERANCE;
    mult0 = findKnotMultiplicity (&iFirst0, pK, iInner0 - 1, 0, n - 1, endKnotTol);
    nInterior0 = (iFirst0 + mult0) - order;
    mult1 = findKnotMultiplicity (&iFirst1, pK, iInner1 + 1, 0, n - 1, endKnotTol);
    nInterior1 = (n - iFirst1) - order;

    // Special kludge to open up periodic knot sequences produced by bspcurv_closeCurve and
    //   bspconv_computeCurveFromArc---like this lovely vector for a 7-pole geometrically closed,
    //   nonperiodic rational quadratic B-spline circle: {-1/3, 0,0,0, 1/3,1/3, 2/3,2/3, 1,1,1, 4/3}.
    // NOTE: Equivalent code in mdlBspline_knotsShouldBeOpened* and bspcurv_closeCurve.
    if (*pbClosed
        && mult0 == order
        && pK[iInner0 - 1] == 0.0   // normalized knots
        && nInterior0 == order / 2)
        {
        DPoint4d*   pP = *ppPoles;
        int         lastPoleIndex = *pNumPoles - 1;

        // use V9 tolerances (fc_epsilon, fc_p0001) from bspcurv_closeCurve
        double  tol = DoubleOps::LargestCoordinate (pP, *(pP + lastPoleIndex)) * 0.00001;
        if (tol > 1.0)
            tol = 1.0;

        if (bsiDPoint4d_pointEqualMixedTolerance (pP, pP + lastPoleIndex, tol, 0.0001))
            {
            *ppKnots += nInterior0;
            *pNumKnots -= order - 1;
            *pbClosed = false;
            }
        }

    // if open start/end knots over-clamped, we can repoint past the irrelevant knots/poles
    else if (!*pbClosed && (mult0 >= order || mult1 >= order))
        {
        *ppKnots += nInterior0;
        *ppPoles += nInterior0;
        *pNumKnots -= nInterior0 + nInterior1;
        *pNumPoles -= nInterior0 + nInterior1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute the new B-spline knot vector from the given range of original B-spline
* knots, coalescing multiple knots to the first knot in the group.
* <P>
* If [firstConstIndex,lastConstIndex] defines a subinterval which intersects
* [firstIndex,lastIndex], then local knots corresponding to the original knots
* inside this subinterval are set to the original B-spline knot with index
* constKnotIndex.  (This feature is useful for setting the first/last knot
* multiplicities, which you may not want to coalesce to the first knot in the
* group).  Passing negative values for all 3 "const" params will always bypass
* the constant subinterval logic.
*
* @param    pNewKnots           <= local, coalesced B-spline knots
* @param    pKnots              => full Bsp knot sequence
* @param    firstIndex          => first Bsp knot to check (must be valid index)
* @param    lastIndex           => last Bsp knot to check (must be valid index)
* @param    firstConstIndex     => first Bsp knot to set to const value
* @param    lastConstIndex      => last Bsp knot to set to const value
* @param    constKnotIndex      => valid idx of knot for const indices, if any
* @param    knotTolerance       => absolute min distance separating unique knots
* @param    order               => B-spline curve order (= degree + 1)
* @see #extractNextBezierFromBspline
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    createCoalescedLocalKnotVector
(
        double  *pNewKnots,
const   double  *pKnots,
        int     firstIndex,
        int     lastIndex,
        int     firstConstIndex,
        int     lastConstIndex,
        int     constKnotIndex,
        double  knotTolerance,
        int     order
)
    {
    double  dt;
    int     mult;
    int     i, j;

    for (i = firstIndex; i <= lastIndex; i++)
        {
        if (lastConstIndex < i || i < firstConstIndex)
            {
            mult = findKnotMultiplicity (NULL, pKnots, i, i, lastIndex, knotTolerance);

            /* coalesce multiple knots to the first knot in the group */
            dt = pKnots[i];
            for (j = 0; j < mult; j++)
                *pNewKnots++ = dt;

            /* advance to index of last multiple written */
            i += mult - 1;
            }

        /*
        Knots inside [firstConstIndex, lastConstIndex] are assumed to be
        pre-coalesced to the knot at constKnotIndex.
        */
        else
            *pNewKnots++ = pKnots[constKnotIndex];
        }
    }

/*---------------------------------------------------------------------------------**//**
* Creates the local knot/pole vectors and local B-spline knot index needed to
* compute the first pole of the first Bezier segment of the given B-spline curve.
*
* @param pContext       <=> local and global curve data
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    initLocalKnotsAndPoles
(
ExtractContext  *pContext
)
    {
    /* curve info */
    const DPoint4d  *pPoles = pContext->pPoles;
    int             numPoles = pContext->numPoles;
    const double    *pKnots = pContext->pKnots;
    int             numKnots = pContext->numKnots;
    double          knotTolerance = pContext->knotTolerance;
    int             order = pContext->order;
    bool            bClosed = pContext->bClosed;

    /* local curve info */
    DPoint4d    *pP = pContext->pLocalBspPoles;
    double      *pBezK = pContext->pLocalBezKnots;
    double      *pBspK = pContext->pLocalBspKnots;

    int     degree = order - 1;
    int     mu;
    int     i;

    /* Comment notation:
        d = degree, m = order, p = numPoles, mult = multiplicity, n = numKnots,
        k = original coalesced B-spline knot */

    /*
    In general the local knots are:
        Bsp: idx [mu - d, mu + d]; coalesce non-startKnots,
            indices < 0 have value 0 (closed curves).
        Bez: m knots, starting with k[mu] if closed, and k[0] if open,
            followed by 2d - 1 copies of k[d].
    In general the local poles are:
        Bsp: idx [mu - d, mu]; indices < 0 have the zero pole (closed curves).
    */

    /* knot sequence given (possibly nonuniform) */
    if (pKnots)
        {
        double  startKnot, endKnot;
        int     startKnotIndex, endKnotIndex;

        /* closed (possibly nonuniform) */
        if (bClosed)
            {
            int mult, start0;

            startKnotIndex = degree;
            endKnotIndex = numKnots - order;
            startKnot = pKnots[startKnotIndex];
            endKnot = pKnots[endKnotIndex];

            // look for dups of start/endKnot
            pContext->mult = mult = findKnotMultiplicity (&start0, pKnots, startKnotIndex, 0, endKnotIndex - 1, knotTolerance);
            pContext->finalMult = findKnotMultiplicity (&pContext->finalMu, pKnots, endKnotIndex, startKnotIndex + 1, numKnots - 1, knotTolerance);

            mu = pContext->mu = start0 - 1; // mu is the index of the knot before startKnot
            --pContext->finalMu;            // finalMu is the index of the knot before endKnot

            // handle (over)saturated start knot like open curve
            // Note: we haven't pointed past excess start knots, so start local Bsp poles/knots at mu - d
            if (mult >= order)
                {
                mu = pContext->mu = start0 + mult - 1;  // index of last startKnot multiple (>= degree)

                /* Bsp knots: already coalesced startKnot = [0, mu] */
                createCoalescedLocalKnotVector
                    (
                    pBspK, pKnots, mu - degree, mu + degree,
                    0, mu, startKnotIndex, knotTolerance, order
                    );

                /* Bez knots */
                for (i = 0; i < 2 * degree; i++)
                    pBezK[i] = startKnot;

                /* Bsp poles */
                memcpy (pP, &pPoles[mu - degree], order * sizeof (DPoint4d));
                }
            else
                {
                DPoint4d    zeroPole = {0.0, 0.0, 0.0, 1.0};

                /* Bsp knots: startKnot = [mu + 1, mu + mult] (d - mu zero pads) */
                for (i = 0; i < degree - mu; i++)
                    pBspK[i] = 0.0;
                createCoalescedLocalKnotVector
                    (
                    &pBspK[degree - mu], pKnots, 0, mu + degree,
                    mu + 1, mu + mult, startKnotIndex, knotTolerance,
                    order
                    );

                /* Bez knots */
                pBezK[0] = pKnots[mu];
                for (i = 1; i < 2 * degree; i++)
                    pBezK[i] = startKnot;

                /* Bsp poles (d - mu zero pads) */
                for (i = 0; i < degree - mu; i++)
                    pP[i] = zeroPole;
                memcpy (&pP[i], pPoles, (mu + 1) * sizeof (DPoint4d));
                }
            }

        /* open (possibly nonuniform) */
        else
            {
            startKnotIndex = 0;
            endKnotIndex = numKnots - order;
            startKnot = pKnots[startKnotIndex];
            endKnot = pKnots[endKnotIndex];

            // ASSUME: start/end knots of open B-spline curve are clamped
            pContext->mult = pContext->finalMult = order;

            // in repointInvalidKnots, we've repointed past excess start knots and truncated excess end knots
            mu = pContext->mu = degree;
            pContext->finalMu = endKnotIndex - 1;

            /* Bsp knots: already coalesced startKnot = [0, mu] */
            createCoalescedLocalKnotVector
                (
                pBspK, pKnots, 0, mu + degree,
                0, mu, startKnotIndex, knotTolerance, order
                );

            /* Bez knots */
            for (i = 0; i < 2 * degree; i++)
                pBezK[i] = startKnot;

            /* Bsp poles */
            memcpy (pP, pPoles, order * sizeof (DPoint4d));
            }
        }

    /* knot sequence not given => uniform, ignore n */
    else
        {
        double delta;

        /* closed uniform */
        if (bClosed)
            {
            DPoint4d    zeroPole = {0.0, 0.0, 0.0, 1.0};

            pContext->mult = pContext->finalMult = 1;

            delta = 1.0 / (double) numPoles;
            mu = pContext->mu = degree - 1;
            pContext->finalMu = numPoles + degree - 1;

            /* Bsp knots: values {-d/p, ..., (d-1)/p} (one zero pad) */
            pBspK[0] = 0.0;
            pBspK[1] = -degree * delta;
            for (i = 2; i <= mu + order; i++)
                pBspK[i] = pBspK[i - 1] + delta;

            /*
            Bez knots: values {-1/p, 0 mult d}
            (add 1 to mu due to Bsp knot zero pad affecting Bsp knot indexing)
            */
            pBezK[0] = pBspK[mu + 1];
            for (i = 1; i < 2 * degree; i++)
                pBezK[i] = pBspK[mu + 2];

            /* Bsp poles (one zero pad) */
            pP[0] = zeroPole;
            memcpy (&pP[1], pPoles, degree * sizeof (DPoint4d));
            }

        /* open uniform */
        else
            {
            pContext->mult = pContext->finalMult = order;

            delta = 1.0 / (double) (numPoles - degree);
            mu = pContext->mu = degree;
            pContext->finalMu = numPoles - 1;

            /* Bsp knots: values {0 mult m, ..., d/(p-d)} */
            memset (pBspK, 0, order * sizeof (double));
            for (i = order; i < mu + order; i++)
                pBspK[i] = pBspK[i - 1] + delta;

            /* Bez knots: values {0 mult m} */
            memset (pBezK, 0, 2 * degree * sizeof (double));

            /* Bsp poles */
            memcpy (pP, pPoles, order * sizeof (DPoint4d));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Shifts the local knot/pole vectors that were used to compute the first pole of
* a Bezier segment, to prepare for calculating the next degree Bezier poles of
* this segment.
*
* @param pContext       <=> local and global curve data
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    shiftLocalKnotsAndPoles
(
ExtractContext  *pContext
)
    {
    /* curve info */
    const DPoint4d  *pPoles = pContext->pPoles;
    int             numPoles = pContext->numPoles;
    const double    *pKnots = pContext->pKnots;
    double          knotTolerance = pContext->knotTolerance;
    int             order = pContext->order;
    bool            bClosed = pContext->bClosed;

    /* local curve info */
    DPoint4d    *pP = pContext->pLocalBspPoles;
    double      *pBezK = pContext->pLocalBezKnots;
    double      *pBspK = pContext->pLocalBspKnots;
    int         prevMu = pContext->mu;
    int         preproc = !pContext->status;    /* nonzero if this is 1st call */

    int         degree = order - 1;
    int         mu;                             /* new mu */
    int         muDiff;                         /* mu - prevMu */
    int         l;                              /* local index */
    int         m;                              /* master index */

    /* Comment notation:
        d = degree, m = order, p = numPoles, mult = multiplicity,
        k = original coalesced B-spline knot */

    // Compute next mu ( = highest idx s.t. pKnots[mu] <= next Bezier knot = next B-spline interior knot)
    if (preproc && (!bClosed || pContext->mu >= degree))   // preproc open curve or preproc closed curve with (over)saturated start knot
        muDiff = 0;
    else if (!pKnots)   // closed uniform or not preproc open uniform curve
        muDiff = 1;
    else                // closed non-uniform curve (with unsaturated start knot) or not preproc open non-uniform curve
        {
        // prevMu is its own local index iff preprocessing
        int     localMuPlus1 = (preproc) ? prevMu + 1 : order;

        // count local knots until next mu (muDiff >= 1)
        for (muDiff = 1; muDiff < degree; muDiff++)
            if (pBspK[localMuPlus1 + muDiff] != pBspK[localMuPlus1])
                break;

        // if we've exhausted the local vector, check for mult >= order in full knot vector
        if (muDiff == degree)
            muDiff = findKnotMultiplicity (NULL, pKnots, prevMu + 1, prevMu + 1, pContext->finalMu, knotTolerance);
        }
    mu = pContext->mu += muDiff;

    // set knot multiplicity of the end knot of this segment
    if (muDiff > 0)
        pContext->mult = muDiff;

    /* B-spline knots:
    Shift left l local indices [muDiff .. (pKnots) ? 2d - 1 : 2d].
    Then if pKnots,
        add coalesced knots reading from original index mu - d + l onward
        until 2d + 1 total knots in place;
    otherwise,
        add knotGap to the local knot at index 2d
        (if open, do this only if mu + d - 2 <= finalMu).
    The result is the subinterval
        if pKnots:  [mu - d .. mu + d] (coalesced)
    or, if closed:  [(mu - 2d) / p .. mu / p]
    or (if open):   [max{0, (mu - 2d) / (p - d)} mult max{1, 2d - mu + 1} ..
                        min{1, mu / (p - d)} mult max{1, mu + d - finalMu}].
    */
    if (muDiff > 0)
        {
        /* shift one more knot if we can just compute 'em */
        l = (pKnots)
                ? 2 * degree - muDiff
                : 2 * degree - muDiff + 1;

        if (l > 0)
            memcpy (pBspK, &pBspK[muDiff], l * sizeof (double));
        else
            l = 0;

        if (pKnots)
            createCoalescedLocalKnotVector
                (
                &pBspK[l], pKnots, mu - degree + l, mu + degree,
                -1, -1, -1, knotTolerance, order
                );

        /* max one more knot to adjust if uniform */
        else if (bClosed || mu + degree <= pContext->finalMu + 1)
            pBspK[l] += (bClosed)
                            ? 1.0 / (double) numPoles
                            : 1.0 / (double) (numPoles - degree);
        }

    /* Bez knots:
    Shift left local indices [d..2d-1], and then add the next knot with mult d.
    The result is the subinterval [k[mu] mult d..k[mu + 1] mult d].
    */
    memcpy (pBezK, &pBezK[degree], degree * sizeof (double));
    for (l = degree; l < 2 * degree; l++)
        pBezK[l] = pBspK[order];

    /* Bsp poles:
    Shift left l local indices [muDiff..d], and then add poles reading from
    original index mu - d + l onward, until m poles are in place, wrapping
    around if necessary.
    The result is the (wrapped) subinterval [mu - d, mu].
    */
    l = order - muDiff;

    if (l > 0)
       memcpy (pP, &pP[muDiff], l * sizeof (DPoint4d));
    else
        l = 0;

    for (m = mu - degree + l; l < order; l++, m++)
        pP[l] = (m >= numPoles) ? pPoles[m - numPoles] : pPoles[m];

    // the Bezier pole isn't shared across an oversaturated interior knot; reset the "shared" pole to the interpolated pole
    if (muDiff > degree)
        pContext->sharedPole = pP[0];
    }

/*----------------------------------------------------------------------+
|                                                                       |
| This magic number ensures that the shared input struct for            |
| bsiBezierDPoint4d_extractNextBezierFromBspline has been initialized  |
|                                                                       |
+----------------------------------------------------------------------*/
#define EXTRACT_NEXT_BEZ_FLAG 0xDA499

/*---------------------------------------------------------------------------------**//**
* Fills the context fields corresponding to local data.
*
* @param pContext       <=> local and global curve data
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    initContext
(
ExtractContext  *pContext
)
    {
    int degree = pContext->order - 1;

    /* setup starting local knots/poles and first mu */
    initLocalKnotsAndPoles (pContext);

    /*
    Find first pole of first Bezier segment.  Local mu = global mu.
    Start poles and Bsp knots off at first true pole/knot (not zero padding);
    all subsequent local arrays will be full of true poles/knots, since the
    mu after firstMu must be >= degree, and Oslo takes poles mu - degree..mu
    and Bsp knots mu - degree..mu + degree.
    */
    computeInsertedPoleWithOslo
        (
        &pContext->sharedPole, 0,
        &pContext->pLocalBspPoles[degree - pContext->mu],
        pContext->pLocalBezKnots,
        &pContext->pLocalBspKnots[degree - pContext->mu],
        pContext->mu, pContext->order
        );

    /*
    Shift knots/poles/mu for calculating next d Bez poles.  Status flag 0 means
    shifter performs differently on this first call; thereafter, status gets a
    nonzero magic number signifying the context has been initialized.
    */
    pContext->status = 0;
    shiftLocalKnotsAndPoles (pContext);
    pContext->status = EXTRACT_NEXT_BEZ_FLAG;
    }

/*---------------------------------------------------------------------------------**//**
* Initializes the data structure shared between successive calls to
* <A HREF="#extractNextBezierFromBspline">extractNextBezierFromBspline</A>,
* which can be used in a loop to extract Bezier segments corresponding to the
* given B-spline curve.
* <P>
* The knot sequence, if given, is assumed to be valid, i.e., it is
* <UL><LI> nondecreasing,
*     <LI> has no knot of multiplicity greater than order,and
*     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
*          the first and last order knots.
* </UL>
* The periodicity condition requires that the i_th knot interval equal the
* (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
* knot intervals due to multiplicities).
* The clamped condition requires the first and last knots to have multiplicity
* order.
* <P>
* Two knots in a given sequence are considered to be equal if the distance
* between them does not exceed the given relative tolerance.  If pKnots is null,
* then a uniform knot sequence is assumed and both numKnots and knotTolerance
* are ignored.
*
* @param pContext           <= local and global curve data initialized
* @param pPoles             => B-spline curve homogeneous poles
* @param numPoles           => # B-spline curve poles
* @param pKnots             => full B-spline curve knots; null for uniform
* @param numKnots           => # B-spline knots
* @param knotTolerance      => relative minimal distance separating unique knots
* @param order              => B-spline curve order (= degree + 1)
* @param bClosed            => true for closed curve, false for open
* @return false iff order out of range or invalid input
* @see #extractNextBezierFromBspline
* @see #extractNextBezierFromBsplineEnd
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineInit
(
        ExtractContext  *pContext,
const   DPoint4d        *pPoles,
        int             numPoles,
const   double          *pKnots,
        int             numKnots,
        double          knotTolerance,
        int             order,
        bool            bClosed
)
    {
    if (!pContext || !pPoles || order > MAX_BEZIER_CURVE_ORDER || order < 2)
        return false;

    // recover from invalid knot sequences
    repointInvalidKnots ((DPoint4d **) &pPoles, &numPoles, (double **) &pKnots, &numKnots, knotTolerance, order, &bClosed);

    /* copy (ptrs to) curve info into Context */
    pContext->pPoles = pPoles;
    pContext->numPoles = numPoles;
    pContext->pKnots = pKnots;
    pContext->numKnots = numKnots;
    pContext->knotTolerance = knotTolerance;
    if (pKnots)
        pContext->knotTolerance *= pKnots[numKnots - order] - pKnots[order - 1];
    pContext->order = order;
    pContext->bClosed = bClosed;

    /* fill in rest of context */
    initContext (pContext);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Dummy function for symmetry.
*
* @see #extractNextBezierFromBsplineInit
* @see #extractNextBezierFromBspline
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    bsiBezierDPoint4d_extractNextBezierFromBsplineEnd
(
ExtractContext  *pContext
)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description Outputs the homogeneous poles of the next Bezier segment of the given
*   B-spline curve, as determined by the local curve data in pContext.
*   The number of poles output equals the order of the curve.
* @remarks To process all Bezier segments of a B-spline, call this method in a loop until
*   false is returned.  Sandwich the loop by a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
*   beforehand (to initialize the context), and a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
*   afterwards.
*
* @param pBezierPoles       <= order homogeneous poles of next Bez segment
* @param pContext           <=> local and global curve data
* @return true iff a Bezier segment was extracted
* @see #extractNextBezierFromBsplineInit
* @see #extractNextBezierFromBsplineEnd
* @see #convertBsplineToBeziers
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBspline
(
        DPoint4d        *pBezierPoles,
        ExtractContext  *pContext
)
    {
    return bsiBezierDPoint4d_extractNextBezierFromBsplineExt2 (pBezierPoles, NULL, NULL, NULL, NULL, pContext);
    }

/*---------------------------------------------------------------------------------**//**
* @description Outputs the homogeneous poles of the next Bezier segment of the given
*   B-spline curve, as determined by the local curve data in pContext.
*   The number of poles output equals the order of the curve.
* @remarks Optionally outputs the Bezier segment's knot span: the original B-spline
*   knots (parameters) between which the Bezier segment lies.
* @remarks To process all Bezier segments of a B-spline, call this method in a loop until
*   false is returned.  Sandwich the loop by a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
*   beforehand (to initialize the context), and a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
*   afterwards.
*
* @param pBezierPoles       <= order homogeneous poles of next Bez segment
* @param pStartKnot         <= B-spline param at which next Bez segment starts (or NULL)
* @param pEndKnot           <= B-spline param at which next Bez segment ends (or NULL)
* @param pContext           <=> local and global curve data
* @return true iff a Bezier segment was extracted
* @see #extractNextBezierFromBsplineInit
* @see #extractNextBezierFromBsplineEnd
* @see #convertBsplineToBeziers
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineExt
(
        DPoint4d        *pBezierPoles,
        double          *pStartKnot,
        double          *pEndKnot,
        ExtractContext  *pContext
)
    {
    return bsiBezierDPoint4d_extractNextBezierFromBsplineExt2 (pBezierPoles, pStartKnot, pEndKnot, NULL, NULL, pContext);
    }

/*---------------------------------------------------------------------------------**//**
* @description Outputs the homogeneous poles of the next Bezier segment of the given
*   B-spline curve, as determined by the local curve data in pContext.
*   The number of poles output equals the order of the curve.
* @remarks Optionally outputs the Bezier segment's knot span: the original B-spline
*   knots (parameters) between which the Bezier segment lies, and their multiplicities.
* @remarks To process all Bezier segments of a B-spline, call this method in a loop until
*   false is returned.  Sandwich the loop by a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineInit</A>,
*   beforehand (to initialize the context), and a call to
*   <A HREF="#extractNextBezierFromBsplineInit">extractNextBezierFromBsplineEnd</A>,
*   afterwards.
*
* @param pBezierPoles       <= order homogeneous poles of next Bez segment
* @param pStartKnot         <= B-spline param at which next Bez segment starts (or NULL)
* @param pEndKnot           <= B-spline param at which next Bez segment ends (or NULL)
* @param pStartKnotMult     <= multiplicity of pStartKnot (or NULL)
* @param pEndKnotMult       <= multiplicity of pEndKnot (or NULL)
* @param pContext           <=> local and global curve data
* @return true iff a Bezier segment was extracted
* @see #extractNextBezierFromBsplineInit
* @see #extractNextBezierFromBsplineEnd
* @see #convertBsplineToBeziers
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool        bsiBezierDPoint4d_extractNextBezierFromBsplineExt2
(
        DPoint4d        *pBezierPoles,
        double          *pStartKnot,
        double          *pEndKnot,
        int             *pStartKnotMult,
        int             *pEndKnotMult,
        ExtractContext  *pContext
)
    {
    /* local curve info */
    DPoint4d    *pP = pContext->pLocalBspPoles;
    double      *pBezK = pContext->pLocalBezKnots;
    double      *pBspK = pContext->pLocalBspKnots;
    int         order = pContext->order;

    DPoint4d    *pBez = pBezierPoles;
    int         degree = order - 1;
    int         i;

    /*
    Exit if previous call computed the last Bezier segment, or the shared
    struct has not been initialized.
    */
    if (pContext->status != EXTRACT_NEXT_BEZ_FLAG)
        return false;

    // shared pole and start knot multiplicity were computed in previous call
    *pBez++ = pContext->sharedPole;
    if (pStartKnotMult)
        *pStartKnotMult = pContext->mult;

    /*
    Compute last degree poles of this Bezier segment.
    The local value of mu remains constant ( = degree).
    */
    for (i = 0; i < degree; i++)
        computeInsertedPoleWithOslo
            (pBez++, i, pP, pBezK, pBspK, degree, order);

    /*
    The B-spline parameter range of the j_th Bezier segment is found in the
    local Bezier's knot sequence: {k[mu] mult d, k[mu+1] mult d}, where
    k is the coalesced B-spline knot vector, mu is the global mu stored in
    pContext, and k[mu] is the left knot in the j_th non-degenerate B-spline
    knot interval.
    */
    if (pStartKnot)
        *pStartKnot = pBezK[0];
    if (pEndKnot)
        *pEndKnot = pBezK[degree];

    if (pContext->mu == pContext->finalMu)
        {
        /* just computed the last segment of the spline */
        pContext->status = 0;
        if (pEndKnotMult)
            *pEndKnotMult = pContext->finalMult;
        }
    else if (pContext->mu > pContext->finalMu)
        {
        /* error state if we ever get here... */
        pContext->status = ERROR;
        return false;
        }
    else
        {
        /* last pole is first pole of next Bezier segment */
        pContext->sharedPole = *--pBez;

        /* shift knots/poles/mu for calculating next degree Bez poles */
        shiftLocalKnotsAndPoles (pContext);

        if (pEndKnotMult)
            *pEndKnotMult = pContext->mult;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* Output the homogeneous poles of the Bezier spline corresponding to the given
* B-spline curve.  The input/output curves have the same order.
* <P>
* The knot sequence, if given, is assumed to be valid, i.e., it is
* <UL><LI> nondecreasing,
*     <LI> has no knot of multiplicity greater than order,and
*     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
*          the first and last order knots.
* </UL>
* The periodicity condition requires that the i_th knot interval equal the
* (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
* knot intervals due to multiplicities).
* The clamped condition requires the first and last knots to have multiplicity
* order.
* <P>
* Two knots in a given sequence are considered to be equal if the distance
* between them does not exceed the given relative tolerance.  If pKnots is null,
* then a uniform knot sequence is assumed and both numKnots and knotTolerance
* are ignored.
* <P>
* The number N of Bezier poles output has the sharp upper bound
* <UL>
* <LI> <CODE>N <= 1 + (order - 1) * (numKnots - 2 * order + 1)</CODE>,
*      if bShare is true
* <LI> <CODE>N <= order * (numKnots - 2 * order + 1)</CODE>, if bShare is false
* </UL>
* In particular, if the curve has uniform knot sequence,
* <UL>
* <LI> <CODE>N = 1 + (order - 1) * numPoles</CODE>,
*      if bShare and bClosed are true
* <LI> <CODE>N = 1 + (order - 1) * (numPoles - order + 1)</CODE>,
*      if bShare is true and bClosed is false
* <LI> <CODE>N = order * (numPoles - order + 1)</CODE>,
*      if bShare and bClosed are false
* <LI> <CODE>N = order * numPoles</CODE>, if bShare is false and bClosed is true
* </UL>
* <P>
* Optionally outputs each Bezier segment's knot (parameter) span in the B-spline
* curve.  The number M of parameters returned, and has the sharp upper bound
* <UL>
* <LI> <CODE>M <= numKnots - 2 * order + 2</CODE>, if bShare is true
* <LI> <CODE>M <= 2 * (numKnots - 2 * order + 1)</CODE>, if bShare is false
* </UL>
* <P>
* Note: for open uniform B-spline curves, it is faster to call
* <A HREF="#convertOpenUniformBsplineToBeziers">convertOpenUniformBsplineToBeziers</A>.
*
* @param pBezierPoles   <= array of homogeneous poles of Bezier spline (must hold N entries)
* @param pParameters    <= array of B-spline parameter spans of the Bezier segments (must hold M entries) (or NULL)
* @param pPoles         => array of homogeneous poles of B-spline curve
* @param numPoles       => number of poles of B-spline curve
* @param pKnots         => full knot sequence of B-spline curve (null for uniform)
* @param numKnots       => size of full knot sequence
* @param knotTolerance  => relative minimal distance separating unique knots
* @param order          => B-spline/Bezier spline curve order (= degree + 1)
* @param bClosed        => true for closed curve, false for open
* @param bShare         => true if each Bezier segment end point/parameter is to appear only once in output
* @return number of Bezier segments output or 0 if order out of range.
* @see #convertOpenUniformBsplineToBeziers
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_convertBsplineToBeziersExt
(
        DPoint4d*   pBezierPoles,
        double*     pParameters,
const   DPoint4d*   pPoles,
        int         numPoles,
const   double*     pKnots,
        int         numKnots,
        double      knotTolerance,
        int         order,
        bool        bClosed,
        bool        bShare
)
    {
    /*
    The actual number N of Bezier poles output is given by
        N = (bShare) ? 1 + d * (u - 1) : m * (u - 1),
    where d = degree and u = # unique knots in the closed normalized
    parameter interval (i.e., [0,1] --- not counting first/last d knots).
    */

    ExtractContext  Context;
    DPoint4d        *pBez = pBezierPoles;
    double          param0, param1, *pParam = pParameters;
    int             numBezSegments = 0;
    int             degree = order - 1;

    if (bsiBezierDPoint4d_extractNextBezierFromBsplineInit (&Context, pPoles, numPoles, pKnots, numKnots, knotTolerance, order, bClosed))
        {
        while (bsiBezierDPoint4d_extractNextBezierFromBsplineExt (pBez, &param0, &param1, &Context))
            {
            if (pParam)
                {
                if (!bShare || 0 == numBezSegments)
                    *pParam++ = param0;
                *pParam++ = param1;
                }

            pBez += bShare ? degree : order;
            numBezSegments++;
            }

        bsiBezierDPoint4d_extractNextBezierFromBsplineEnd (&Context);
        }

    return numBezSegments;
    }


/*---------------------------------------------------------------------------------**//**
* Output the homogeneous poles of the Bezier spline corresponding to the given
* B-spline curve.  The input/output curves have the same order.
* <P>
* The knot sequence, if given, is assumed to be valid, i.e., it is
* <UL><LI> nondecreasing,
*     <LI> has no knot of multiplicity greater than order,and
*     <LI> satisfies periodicity (if closed) or clamped (if open) conditions on
*          the first and last order knots.
* </UL>
* The periodicity condition requires that the i_th knot interval equal the
* (numPoles + i)_th knot interval, for i = 0, ..., 2*order - 3 (counting null
* knot intervals due to multiplicities).
* The clamped condition requires the first and last knots to have multiplicity
* order.
* <P>
* Two knots in a given sequence are considered to be equal if the distance
* between them does not exceed the given relative tolerance.  If pKnots is null,
* then a uniform knot sequence is assumed and both numKnots and knotTolerance
* are ignored.
* <P>
* The number N of Bezier poles output has the sharp upper bound
* <UL>
* <LI> <CODE>N <= 1 + (order - 1) * (numKnots - 2 * order + 1)</CODE>,
*      if bShare is true
* <LI> <CODE>N <= order * (numKnots - 2 * order + 1)</CODE>, if bShare is false
* </UL>
* In particular, if the curve has uniform knot sequence,
* <UL>
* <LI> <CODE>N = 1 + (order - 1) * numPoles</CODE>,
*      if bShare and bClosed are true
* <LI> <CODE>N = 1 + (order - 1) * (numPoles - order + 1)</CODE>,
*      if bShare is true and bClosed is false
* <LI> <CODE>N = order * (numPoles - order + 1)</CODE>,
*      if bShare and bClosed are false
* <LI> <CODE>N = order * numPoles</CODE>, if bShare is false and bClosed is true
* </UL>
* <P>
* Note: for open uniform B-spline curves, it is faster to call
* <A HREF="#convertOpenUniformBsplineToBeziers">convertOpenUniformBsplineToBeziers</A>.
*
* @param pBezierPoles   <= array of homogeneous poles of Bezier spline
* @param pPoles         => array of homogeneous poles of B-spline curve
* @param numPoles       => number of poles of B-spline curve
* @param pKnots         => full knot sequence of B-spline curve (null for uniform)
* @param numKnots       => size of full knot sequence
* @param knotTolerance  => relative minimal distance separating unique knots
* @param order          => B-spline/Bezier spline curve order (= degree + 1)
* @param bClosed        => true for closed curve, false for open
* @param bShare         => true if each Bezier segment end point is to appear only once in output
* @return number of Bezier segments output or 0 if order out of range.
* @see #convertOpenUniformBsplineToBeziers
* @see #convertBsplineToBeziersExt
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int         bsiBezierDPoint4d_convertBsplineToBeziers
(
        DPoint4d    *pBezierPoles,
const   DPoint4d    *pPoles,
        int         numPoles,
const   double      *pKnots,
        int         numKnots,
        double      knotTolerance,
        int         order,
        bool        bClosed,
        bool        bShare
)
    {
    return bsiBezierDPoint4d_convertBsplineToBeziersExt (pBezierPoles, NULL, pPoles, numPoles, pKnots, numKnots, knotTolerance, order,
                                                          bClosed, bShare);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
