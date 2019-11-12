/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
//
#include <Geom/XYRangeTree.h>

using namespace Bentley::Geometry;

namespace Bentley {
  namespace Geometry {
class XYRangeTreeDump
{

int mIndent;
public:
XYRangeTreeDump (int indent)
    {
    mIndent = indent;
    }

void StartLine (int indent, char *pString = NULL)
    {
    printf ("\n");
    for (int i = 0; i < indent; i++)
        printf (" ");
    if (NULL != pString)
        printf ("%s", pString);
    }

void Attribute (int indent, char *pString, double value)
    {
    if (indent >= 0)
        printf ("\n");
    for (int i = 0; i < indent; i++)
        printf (" ");
    printf (" %s=\"%g\"", pString, value);
    }

void Dump (XYRangeTreeQueryP pNode, int indent)
    {
    char buffer [2000];
    if (NULL == pNode)
        {
        StartLine (indent, "<NullPtr/>");
        }
    else
        {
        XYRangeTreeRootP pRoot = pNode->AsRoot ();
        XYRangeTreeInteriorP pInterior = pNode->AsInterior ();
        XYRangeTreeLeafP pLeaf = pNode->AsLeaf ();
        if (NULL != pRoot)
            {
            StartLine (indent, "<Root>");
            Dump (pRoot->mChild, indent + mIndent);
            StartLine (indent, "</Root>");
            }
        else if (NULL != pInterior)
            {
            int parentId =
                       NULL != pInterior->mpParent
                    && NULL != pInterior->mpParent->AsInterior ()
                    ?  pInterior->mpParent->AsInterior ()->mInstanceId
                    :  -1;
            sprintf (buffer, "<Interior low=\"%g,%g\" high=\"%g,%g\" extent2=\"%g\" id=\"%d\" parentId=\"%d\">",
                        pInterior->mRange.low.x, pInterior->mRange.low.y,
                        pInterior->mRange.high.x, pInterior->mRange.high.y,
                        pInterior->mExtentSquared,
                        pInterior->mInstanceId, parentId);
            StartLine (indent, buffer);
            for (int i = 0; i < pInterior->mNumChild; i++)
                {
                Dump (pInterior->mChild[i], indent + mIndent);
                }
            StartLine (indent, "</Interior>");
            }
        else if (NULL != pLeaf)
            {
            sprintf (buffer, "<Leaf data=\"%d\" low=\"%g,%g\" high=\"%g,%g\"/>",
                        pLeaf->mData,
                        pLeaf->mRange.low.x, pLeaf->mRange.low.y,
                        pLeaf->mRange.high.x, pLeaf->mRange.high.y);
            StartLine (indent, buffer);
            }
        }
    if (indent == 0)
        printf ("\n");
    }
};



class XYRangeTreeKeyinPrinter : public XYRangeTreeHandler
{
public:
int mDepth;
XYRangeTreeKeyinPrinter (){mDepth = 0;}
void Box (XYRangeTreeQueryP pNode)
    {
    printf ("PLACE BLOCK\n");
    DRange2d range = pNode->Range ();
    printf ("xy=%g,%g\n", range.low.x, range.low.y);
    printf ("xy=%g,%g\n", range.high.x, range.high.y);
    }
virtual bool ShouldRecurseIntoSubtree       (XYRangeTreeRootP, XYRangeTreeInteriorP)
        {
        mDepth++;
        return true;
        }
virtual bool ShouldContinueAfterSubtree      (XYRangeTreeRootP, XYRangeTreeInteriorP pInterior)
        {
        Box (pInterior);
        mDepth--;
        return true;
        }
virtual bool ShouldContinueAfterLeaf         (XYRangeTreeRootP, XYRangeTreeInteriorP, XYRangeTreeLeafP pLeaf    )
        {
        Box (pLeaf);
        return true;
        }
};

};};



//  Eliminate ERROR warning
#undef          ERROR

#include        <sys/timeb.h>
#include        <windows.h>
#define TimerStackLimit 100
class TimerStack
{
private:
double mStack[100];
int mStackIndex;
int mNoisy;
public:

void SetNoisy (int n)
    {
    mNoisy = n;
    }

double GetSeconds ()
    {
    return (double)GetTickCount() * 0.001;
    }

TimerStack ()
    {
    mStackIndex = 0;
    mStack[0] = GetSeconds ();
    mNoisy = 1000;
    }

void Push ()
    {
    mStackIndex++;
    if (mStackIndex >= 0 && mStackIndex < TimerStackLimit)
        mStack[mStackIndex] = GetSeconds ();
    }
void Pop ()
    {
    mStackIndex--;
    }

double PeekAndAdvance ()
    {
    if (mStackIndex >= 0 && mStackIndex < TimerStackLimit)
        {
        double currentSeconds = GetSeconds ();
        double oldSeconds = mStack[mStackIndex];
        mStack[mStackIndex] = currentSeconds;
        return currentSeconds - oldSeconds;
        }
    return 0.0;
    }

void Show (char *pMessage, int id = INT_MIN)
    {
    double delta = PeekAndAdvance ();
    for (int i = 0; i < mStackIndex * 2; i++)
        printf ("  ");
    if (mNoisy < 1)
        return;
    if (id == INT_MIN)
        printf ("%s %g\n", pMessage, delta);
    else
        printf ("%s %d %g\n", pMessage, id, delta);
    }
};



#include <Geom/xyPointSearch.h>
#ifdef COMPILE_XYPointSearcherUtils
namespace Bentley { namespace Geometry {
class XYPointSearcherUtils
{
public:
static void DumpAsKeyins (XYPointSearcher &pointSearcher)
    {
    printf (" SORTED XYPointSearcher %d rows\n", pPointSearcher->rows.size ());
    for (unsigned int r = 0; r < pPointSearcher->rows.size (); r++)
        {
        printf (" Row %d i0=%d   i1=%d yMin=%g yMax=%g\n",
                    r,
                    pPointSearcher->rows[r].i0,
                    pPointSearcher->rows[r].i1,
                    pPointSearcher->rows[r].a0,
                    pPointSearcher->rows[r].a1
                    );

        unsigned int i0 = pPointSearcher->rows[r].i0;
        unsigned int i1 = pPointSearcher->rows[r].i1;

        printf ("PLACE SMARTLINE\n");
        for (unsigned int i = i0; i < i1; i++)
            {
            printf ("xy=%g,%g\n", pPointSearcher->points[i].x, pPointSearcher->points[i].y);
            }
        printf ("SELECT\n");
        }
    }
};
};};
#endif


int main(int argc, char * argv[])
    {
    TimerStack timer = TimerStack ();
    static int sPrintKeyins = 0;
    static int sDumpEveryStep = 0;
    static int sMaxDump = 10000;
    bool bShowProbes = 0;
    bool bShowTree = 0;
    int numLeaf = 35;
    int maxDump = 10000;
    int numProbe = 0;
    int numPointProbe = 20;
    int probePrintStep = 1000;
    int treePrintStep  = 10000;
    double probeStep = 0.238;
    double dx = 1.0;
    double dy = 1.0;
    double leafStep = 0.1;
    double r = 100.0;
    double dr = 0.1;
    double drStep = 3.0 * leafStep;
    bool bKeyProbe = false;
    bool bKeyLeaf = false;
    bool bShowStats = true;
    bool bKeyRows = false;

    for (int i = 0; i < argc; i++)
        {
        int value;
        double rValue;
        if (1 == sscanf (argv[i], "n=%d", &value))
            numLeaf = value;
        if (1 == sscanf (argv[i], "maxDump=%d", &value))
            maxDump = value;
        if (1 == sscanf (argv[i], "p=%d", &value))
            numProbe = value;
        if (1 == sscanf (argv[i], "q=%d", &value))
            numPointProbe = value;

        if (1 == sscanf (argv[i], "r=%lf", &rValue))
            r = rValue;
        if (1 == sscanf (argv[i], "leafStep=%lf", &rValue))
            leafStep = rValue;

        if (1 == sscanf (argv[i], "probeStep=%lf", &rValue))
            probeStep = rValue;

        if (1 == sscanf (argv[i], "drStep=%lf", &rValue))
            drStep = rValue;
        if (1 == sscanf (argv[i], "dr=%lf", &rValue))
            dr = rValue;


        if (0 == strcmp(argv[i], "showProbes=true"))
            bShowProbes = true;
        if (0 == strcmp(argv[i], "showProbes=false"))
            bShowProbes = false;

        if (0 == strcmp(argv[i], "showTree=true"))
            bShowTree = true;
        if (0 == strcmp(argv[i], "showTree=false"))
            bShowTree = false;

        if (0 == strcmp(argv[i], "keyProbe=true"))
            bKeyProbe = true;
        if (0 == strcmp(argv[i], "keyProbe=false"))
            bKeyProbe = false;

        if (0 == strcmp(argv[i], "keyLeaf=true"))
            bKeyLeaf = true;
        if (0 == strcmp(argv[i], "keyLeaf=false"))
            bKeyLeaf = false;

        if (0 == strcmp(argv[i], "showStats=true"))
            bShowStats = true;
        if (0 == strcmp(argv[i], "showStats=false"))
            bShowStats = false;

        if (0 == strcmp(argv[i], "keyRows=true"))
            bKeyRows = true;
        if (0 == strcmp(argv[i], "keyRows=false"))
            bKeyRows = false;
        }

    if (!bShowStats)
        timer.SetNoisy (0);
    //struct tm tm0;
    //unsigned time0 = getsystime(tm0);

    XYPointSearcherP pPointSearcher = XYPointSearcher::Allocate ();
    XYRangeTreeRootP pTree = XYRangeTreeRoot::Allocate ();
    XYRangeTreeDump dumper = XYRangeTreeDump (2);
    DRange2d range;

    timer.Show ("Start Tree Construction");
    timer.Push ();
    for (int i = 0; i < numLeaf; i++)
        {
        double theta = i * leafStep;
        double beta  = i * drStep;
        double ri = r * (1.0 + dr * cos (beta));
        double x = ri * cos (theta);
        double y = ri * sin (theta);
        range.initFrom (x, y, x + dx, y + dy);
        pTree->Add ((void*)i, range);
        pPointSearcher->AddPoint (x, y, (void*)i);
        if (sDumpEveryStep)
            dumper.Dump (pTree, 0);
        if (i > 0 && (i % treePrintStep) == 0)
            timer.Show  ("Tree Construction", i);
        }
    timer.Pop ();
    timer.Show  ("Tree Construction", numLeaf);

    XYRangeTreeClosestLowPointSearcher searcher = XYRangeTreeClosestLowPointSearcher ();
    double minProbeDistance =  DBL_MAX;
    double maxProbeDistance = -DBL_MAX;

    timer.Show  ("Start RangeProbes");
    timer.Push ();
    for (int i = 0; i < numProbe; i++)
        {
        double theta = i * probeStep;
        DPoint2d xy;
        xy.x = (r * cos (theta));
        xy.y = (r * sin (theta));
        searcher.Setup (xy, 2 * r);
        pTree->Traverse (searcher);
        double a = sqrt (searcher.mMinDist2);

        if (a < minProbeDistance)
            minProbeDistance = a;
        if (a > maxProbeDistance)
            maxProbeDistance = a;

        if (bKeyProbe)
            {
            printf ("PLACE LINE\n");
            printf ("xy=%g,%g\n", xy.x, xy.y);
            printf ("xy=%g,%g\n", searcher.mClosestPoint.x, searcher.mClosestPoint.y);
            }
        if (bShowProbes)
            {
            dumper.StartLine (0, "<Probe>");
                dumper.Attribute (4,  " x", xy.x);
                dumper.Attribute (-1, " y", xy.y);
                dumper.Attribute ( 4, "xx",       searcher.mClosestPoint.x);
                dumper.Attribute (-1, "yy",       searcher.mClosestPoint.y);
                dumper.Attribute (-1, "minDist",  sqrt (searcher.mMinDist2));
                dumper.Attribute ( 4, "Itrue",    searcher.mInteriorTrue);
                dumper.Attribute (-1, "Ifalse",   searcher.mInteriorFalse);
                dumper.Attribute (-1, "leaf",     searcher.mLeaf);
                dumper.Attribute (-1, "reduce",   searcher.mReduceDistance);
            dumper.StartLine (0, "</Probe>");
            }
        if (i > 0 && (i % probePrintStep) == 0)
            timer.Show  ("Probe ", i);
        }
    timer.Pop ();
    timer.Show  ("RangeProbes", numProbe);

#ifdef COMPILE_XYPointSearcherUtils
    if (bKeyRows)
        {
        XYPointSearcherUtils::DumpAsKeyins (pointSearcher);
        }
#endif
    timer.Show  ("Start PointProbes");
    timer.Push ();
    for (int i = 0; i < numPointProbe; i++)
        {
        double theta = i * probeStep;
        DPoint2d xy;
        xy.x = (r * cos (theta));
        xy.y = (r * sin (theta));

        DPoint2d uv = {0,0};
        XYPointSearcherTagType uvTag = (XYPointSearcherTagType)i;
        pPointSearcher->ClosestPoint (xy.x, xy.y, uv.x, uv.y, uvTag);

        double a = hypot (xy.x - uv.x, xy.y - uv.y);

        if (a < minProbeDistance)
            minProbeDistance = a;
        if (a > maxProbeDistance)
            maxProbeDistance = a;

        if (bKeyProbe)
            {
            printf ("PLACE LINE\n");
            printf ("xy=%g,%g\n", xy.x, xy.y);
            printf ("xy=%g,%g\n", uv.x, uv.y);
            }
        if (bShowProbes)
            {
            dumper.StartLine (0, "<PointProbe>");
                dumper.Attribute (4,  " x", xy.x);
                dumper.Attribute (-1, " y", xy.y);
                dumper.Attribute ( 4, "xA", uv.x);
                dumper.Attribute (-1, "yA", uv.y);
                dumper.Attribute (-1, "minDistA",  a);
            dumper.StartLine (0, "</PointProbe>");
            }
        if (i > 0 && (i % probePrintStep) == 0)
            timer.Show  ("Probe ", i);
        }
    timer.Pop ();
    timer.Show  ("PointProbes", numPointProbe);

    if (bShowStats)
        {
        dumper.StartLine (0, "<PointProbeStats>");
        dumper.Attribute (4, "numPointProbe", numPointProbe);
        dumper.Attribute (4, "probeStep", probeStep);
        dumper.Attribute (4, "minProbeDistance", minProbeDistance);
        dumper.Attribute (4, "maxProbeDistance", maxProbeDistance);
        dumper.StartLine (0, "</PointProbeStats>");
        }

    if (numLeaf <= maxDump && bShowTree)
        dumper.Dump (pTree, 0);

    XYRangeTreeCounter counter = XYRangeTreeCounter();
    pTree->Traverse (counter);
    if (bShowStats)
        {
        dumper.StartLine (0, "<TreeStats>");
        dumper.Attribute (4, "leafStep", leafStep);

        dumper.Attribute (4, "MAX_TREE_CHILD", MAX_TREE_CHILD);
        dumper.Attribute (4, "maxDepth", counter.mMaxDepth);
        dumper.Attribute (4, "numLeaf",  counter.mNumLeaf);
        dumper.Attribute (4, "numInterior", counter.mNumInterior);

        double a;
        bsiTrig_safeDivide (&a, counter.mNumLeaf, counter.mNumInterior, 0.0);
        dumper.Attribute (4, "leafPerInterior",  a);

        dumper.Attribute (4, "mNumFringe",  counter.mNumFringe);
        bsiTrig_safeDivide (&a, counter.mNumLeaf, counter.mNumFringe, 0.0);
        dumper.Attribute (4, "leafPerFringe",  a);

        dumper.Attribute (4, "minLeafExtent",  counter.mMinFringeExtent);
        dumper.Attribute (4, "maxLeafExtent",  counter.mMaxFringeExtent);

        dumper.StartLine(0, "</TreeStats>");
        }

    if (bKeyLeaf)
        {
        XYRangeTreeKeyinPrinter keyinPrinter = XYRangeTreeKeyinPrinter();
        pTree->Traverse (keyinPrinter);
        }
    delete pPointSearcher;
    //return getExitStatus();
    return 0;
    }


