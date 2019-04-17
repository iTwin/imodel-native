/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <BeJsonCpp/BeJsonUtilities.h>
#include <bentley/bvector.h>
#include <Bentley/WString.h>


// This structure records navigation along a path into a json Value true.
struct JsonPathPosition {
Json::Value value;  // !!! ???? !!! Should this be pointer or reference?
// zero or one (but not both) of these is set to indicate how the path continues beyond this value...
size_t arrayIndex;
Utf8String propertyName;    // ??? can this be some other property reference that does not require a string copy?
};
class CompareJson
{
// QUESTION: should path positions be stored in 
bvector<JsonPathPosition> m_pathA;  // complete path from root of A to current leaf
bvector<JsonPathPosition> m_pathB;  // complete path from root of B to current leaf.
double m_relTol;
CompareJson (double relTol = 1.0e-12){
m_relTol = relTol;
}

// compare json trees below rootA and rootB.
//<ul>
//<li>objects are immediately different if they have different property sets
//<li>arrays are immediately different if they have different lengths
//<li>bools and strings are equal by direct comparison.
//<li>numbers are equal if fabs (a-b) < m_reltol * (1 + fabs (a) + fabs (b))
//</ul>
//After a false return, the paths to the point of difference in each object are preserved as m_pathA and m_pathB;
bool areAlmostEqual (Json::Value rootA, Json::Value rootB){
m_pathA.clear ();
m_pathB.clear ();
return false;
}

};