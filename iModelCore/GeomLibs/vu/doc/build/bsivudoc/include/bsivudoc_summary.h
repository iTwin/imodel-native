/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
+--------------------------------------------------------------------------------------*
//**
@doctext
@group "Overview of the Bentley Vertex Use Library"

<h2>The Bentley Vertex Use Library</h2>

<p>
This document provides a brief overview and specific functional documentation for internal developers who seek to leverage the
topological functionality in Bentley's Vertex Use Library.

<p>
The Bentley Vertex Use Library is independent of MicroStation, and depends only upon the Bentley Base Geometry library
(<CODE>bsibasegeom</CODE>).  Memory management is handled by default C <CODE>malloc</CODE>, etc., or optionally by user-provided functions
registered with ~mvumemfuncs_setFuncs.

<h3>Library Functionality</h3>

<p>
The Bentley Vertex Use Library provides functionality for building, querying and modifying a topology graph, which represents complete
adjacency information for an unlimited-size collection of vertices, edges and faces.  Examples of specific functionality include
triangulation, 2D boolean operations, and polygon decomposition into smaller convex polygons.  The term "Vertex Use"
refers to the multiple "uses" of a given vertex in a graph by its surrounding faces.

<h3>Library Source</h3>

<p>
The source code for the Bentley Vertex Use Library is housed in the <CODE>vu</CODE> subdirectory of the <CODE>geomlibs</CODE> CVS
module, a separate repository from the rest of the MicroStation source.  There are three subdirectories of <CODE>geomlibs\vu</CODE>:
<CODE>src</CODE>, <CODE>build</CODE> and <CODE>doc</CODE>.

<p>
The C/C++ source files are found in <CODE>geomlibs\vu\src</CODE>.  The master header file is <CODE>geomlibs\vu\src\include\vu.h</CODE>.

<p>
Makefiles are housed in <CODE>geomlibs\vu\build</CODE>.  Each subdirectory builds a different deliverable from the same source.

<p>
This help file can be found at <CODE>geomlibs\vu\doc\bsivudoc.chm</CODE>.


<h3>Building the Library</h3>

<p>
The Bentley makefile for building <CODE>bsivu.dll</CODE>, which MicroStation uses, is in <CODE>geomlibs\vu\build\bsivu</CODE>.
There is also a Bentley makefile for building <CODE>vu_static.lib</CODE>, a static version of the library, in
<CODE>geomlibs\vu\build\vu_static</CODE>.

<p>
To build one of these deliverables, you first need to checkout the <CODE>geomlibs</CODE>, <CODE>bsitools</CODE> and <CODE>util</CODE>
modules from CVS.  You will actually only need the <CODE>sharedmki</CODE> and <CODE>sharedinc</CODE> subdirectories of the
<CODE>util</CODE> module.  It is best to checkout all three modules directly under the same top-level source directory.  For CVS help, see
the <A href="http://cvsmgt/">CVS User Administration</A> page, or send e-mail to the <a href="mailto:CVS Administrators">CVS
Administrators</a>.

<p>
Next, open up a command shell and execute <CODE>geomlibs\common\geomlibs_env.cmd</CODE>, passing it the pathnames of your top-level
source directory, the desired output directory for objects and deliverables, and the directory of your compiler, all with trailing
backslashes.  This will set up the necessary environment variables, like <CODE>GEOMLIBS_ROOT</CODE> and <CODE>GEOMLIBS_OUT</CODE>.

<p>
Next, build the Bentley Base Geometry Library.  See <CODE>geomlibs\geom\doc\bsibasegeomdoc.chm</CODE>.

<p>
Finally, <CODE>bmake</CODE> either <CODE>geomlibs\vu\build\bsivu\bsivu.mke</CODE> or <CODE>geomlibs\vu\build\vu_static\vu_static.mke</CODE>.
*/

/**
@doctext
@group "Vertex Use Graphs"

<h2>Vertex Use Graphs</h2>

<p>
A Vertex Use (VU) Graph is a potentially large dynamic graph structure that represents all the adjacency information for a collection of
vertices, edges, and faces.
</p>

<h3>VU Graph Structure</h3>

<p>In geometric terms, there is a Vertex Use at each place where a geometric vertex is incident to---i.e., used by---a geometric face.
To visualize this, draw a small dot by the vertex in the corner of each face adjacent to the vertex.
Each dot is a Vertex Use.
Now draw an arrow from each VU to the VU succeeding it in counterclockwise procession around the face, and another arrow to the VU
succeeding it in counterclockwise procession around the vertex.
Do not bother with the backward (clockwise) arrows.
</p>

<p>The following are evident when this VU pointer structure is examined:

<ul>
<li>Following face successor (F) pointers produces a path around the face that moves along each edge to the successor vertex and
    eventually comes back to where you started.</li>

<li>Similarly, the vertex successor (V) pointers form a loop around the vertex.</li>

<li>For each edge, there are two VUs whose F pointers point "along" the edge.  These two pointers are on opposite sides of
        the edge and are oppositely oriented.</li>

<li>Similarly, there are two VUs whose V pointers "cross over" each edge.  These two pointers are on opposite ends of the edge
        and are oppositely oriented.</li>

<li>Starting at any VU, alternately following F and V pointers returns you to the starting node after a path of exactly 4 pointers.</li>

<li>Due to the previous observation, the <EM>predecessor</EM> of a VU in either its V or F loop can be found in exactly 3 steps:
        V then F then V gives the face predecessor, while F then V then F gives the vertex predecessor.
        (This is why you don't need clockwise (backward) pointers in the graph.)</li>
</ul>
</p>

<p>An individual VU represents many different things at different times:

<ul>
<li>The vertex</li>
<li>The face</li>
<li>The edge to its right in its face loop</li>
<li>The shell = connected component of the graph containing the VU's face, edge, and vertex</li>
</ul>

Keep this behavior in mind when looking at argument lists.
A single VU can be an adequate input argument for a procedure that does many things.
This chameleon-like property may be confusing at first, but it provides many benefits.
</p>

<h3>VU Data Types</h3>

<p>The VU library contains carefully planned code for many common operations on VU graphs: navigation, allocation, temporary arrays of nodes
during searches, primitive modifications that can be combined for a variety of mesh styles, etc.
</p>

<p>There are 3 opaque data types in the implementation:

<ul>
<li>~tVuSetP: pointer to the overall graph structure, aka the <I>graph</I>.</li>
<li>~tVuP: pointer to an individual VU, aka <I>node</I>.</li>
<li>~tVuArrayP: a rubber array of ~tVuP.</li>
</ul>
</p>

<p>A ~tVuSetP is a pointer to a header structure representing an entire graph of nodes.
Any routine that adds nodes to or traverses the graph takes a ~tVuSetP as an argument.
The set header is also the owner of a collection of borrowable bit masks and dynamically-resizable arrays of type ~tVuArrayP.
These greatly simplify memory allocation in meshing code.</p>

<p>Each ~tVuP knows its Cartesian coordinates and has a semantics mask.
Below is a sampling of queries to get data from a ~tVuP <CODE>nodeP</CODE>:

<ul>
<li>~mvu_fsucc (nodeP): returns the successor node around the face.  This simply follows the F pointer.</li>
<li>~mvu_vsucc (nodeP): returns the successor node around the vertex.  This simply follows the V pointer.</li>
<li>~mvu_fpred (nodeP): returns the predecessor node around the face.  This follows the V-F-V pointers.</li>
<li>~mvu_vpred (nodeP): returns the predecessor node around the vertex.  This follows the F-V-F pointers.</li>
<li>~mvu_edgeMate (nodeP): returns the node at the other end and other side of the same edge.  This follows the F-V pointers.</li>
<li>~mvu_getMask (nodeP, mask): return the logical AND of a mask with the node mask.</li>
<li>~mvu_setMask (nodeP, mask): turn on those bits in the node mask that are on in the given mask.</li>
<li>~mvu_clrMask (nodeP, mask): turn off those bits in the node mask that are on in the given mask.</li>
<li>~mvu_writeMask (nodeP, mask, value): execute ~mvu_setMask (nodeP, mask) if value is true; ~mvu_clrMask (nodeP, mask), if false.</li>
<li>~mvu_getX (nodeP): return the x-coordinate of the node.</li>
<li>~mvu_getY (nodeP): return the y-coordinate of the node.</li>
<li>~mvu_getXY (&x, &y, nodeP): return the xy-coordinates of the node.</li>
</ul>
</p>

<h3>VU Loops</h3>

<p>The following macros construct loop logic for the most vital navigation of a graph.
These macros are recommended for graph navigation because node loops are common and hand-coding them is extremely error-prone.
In each of the following, the variable <CODE>currP</CODE> is declared <EM>by the macro expansions</EM>.
That is, your code does not need to declare it, and you should not alter its value inside the loop.</p>

<p>To loop around the vertex at node <CODE>vertP</CODE>, visiting each node in the vertex loop once in CCW order:
<pre>
     VU_VERTEX_LOOP(currP, vertP)
         {
         // your loop body
         }
     VU_END_VERTEX_LOOP(currP, vertP)
</pre>
</p>

<p>To loop around the face at node <CODE>vertP</CODE>, visiting each node in the face loop once in CCW order:
<pre>
     VU_FACE_LOOP(currP, vertP)
         {
         // your loop body
         }
     VU_END_FACE_LOOP(currP, vertP)
</pre>
</p>

<p>To visit every node in the entire graph <CODE>graphP</CODE>:
<pre>
     VU_SET_LOOP(currP, graphP)
         {
         // your loop body
         }
     VU_END_SET_LOOP(currP, graphP)
</pre>
</p>

<h3>Constructing New Topology</h3>

<p>The following are functions to construct new topology:

<ul>
<li>~mvu_makePair (graphP, &node1P, &node2P): add two new nodes to the graph.  The nodes are connected to form an isolated edge with two
    distinct vertex loops.</li>
<li>~mvu_makeSling (graphP, &node1P, &node2P): add two new nodes to the graph.  The nodes are connected to form an isolated edge with both
    ends at the same vertex, i.e. a "sling".</li>
<li>~mvu_splitEdge (graphP, edgeP, &leftNodeP, &rightNodeP): insert a vertex within the edge.  Return the two nodes on the left and right
    sides as viewed from the given node.  The relevant masks are copied from <CODE>edgeP</CODE> to <CODE>leftNodeP</CODE> and from
    ~mvu_edgeMate (<CODE>edgeP</CODE>) to <CODE>rightNodeP</CODE>.</li>
<li>~mvu_join (graphP, startP, endP, &node1P, &node2P): create an edge that joins nodes <CODE>startP</CODE> and <CODE>endP</CODE>.</li>
</ul>
</p>

<p>The function ~mvu_vertexTwist (graphP, node1P, node2P) performs the following mysterious sounding but extraordinarily useful operation:

<ul>
<li>exchange the successors in the respective vertex loops, and</li>
<li>exchange the predecessors in the respective face loops.</li>
</ul>

This operator is, all by itself, sufficiently powerful to take any graph structure, pull all the edges out of their vertex loops and leave
the edges as isolated sticks, and then rebuild the graph.  This is true for any manifold topology, including objects with any number of
holes and any triangulation.
</p>
*/

/**
@doctext
@group "VU Coordinates"

<h2>VU Coordinates</h2>

<p>These functions query, modify, and compare coordinates of nodes.</p>

<p>Each node is associated with a 3D coordinate, though most of the time, only the xy-coordinates (aka uv-coordinates) are used.
The function documentation should make this clear where necessary.</p>
*/

/**
@doctext
@group "VU Node Masks"

<h2>VU Node Masks</h2>

<p>Each node in the graph has an integer ~tVuMask field whose 32 bits are accessible as "mask bits".
Callers can read and write the mask bits to indicate special properties of the node.
VU graph searches depend heavily on this mask field.

<p>Certain constants are predefined for common interpretations of a graph.  The most universal of these are VU_BOUNDARY_EDGE and
VU_EXTERIOR_EDGE.

<p>The following constants are defined for use in spherical or cylindrical settings where there are poles and seams:
VU_NULL_EDGE, VU_NULL_FACE and VU_SEAM_EDGE.

<p>The following constants are defined for use in parametric or B-spline settings: VU_U_SLICE, VU_V_SLICE, VU_KNOT_EDGE, VU_RULE_EDGE,
VU_GRID_EDGE.

<p>The following constants are defined for use as generic surface properties: VU_SILHOUETTE_EDGE, VU_SILHOUETTE_VERTEX, VU_SECTION_EDGE,
VU_DISCONTINUITY_EDGE.
*/

/**
@doctext
@group "VU Marked Edge Sets"

<h2>VU Marked Edge Sets</h2>

<p>A marked edge set is a way of tracking edges so that:

<ol>
<li>It is easy to randomly select one marked edge for processing, and</li>
<li>It is easy to tell whether a given edge is in the set.</li>
</ol>

<p>In comparison, a ~tVuArrayP provides (1) but not (2); mask bits provide (2) but not (1).
The marked edge set works by maintaining both properties: when an edge is added to the set,
both of its VU nodes are marked, and the pointer is added to the array.

<p>When an algorithm decides to remove an edge from the set (but does not know where the node
pointer is in the array), the set functions remove the masks from the nodes but let the pointer
stay in the array.  When a request is made to take a pointer from the array, the nodes addressed
by the pointer are checked for the mask; if not present, the the edge must have been removed
previously, and the pointer can be removed from the array.
*/

/**
@doctext
@group "VU Node Arrays"

<h2>VU Node Arrays</h2>

<p>A ~tVuArrayP is an opaque pointer to the header structure for a
dynamically allocated array of ~tVuP pointers to vertex uses.

<p>The array may be directly allocated and deallocated by calling ~mvu_arrayNew and ~mvu_arrayFree.
However, in most cases an application borrows and returns a cached array via ~mvu_grabArray and ~mvu_returnArray.
*/

/**
@doctext
@group "VU Node Heap"

<h2>VU Node Heap</h2>

<p>These functions manage ~tVuP in a ~tVuArrayP in "heap" manner, maintaining the "min" node for quick removal.
Comparisons are by ~mvu_compareLexicalUV, i.e. the heap facilitates bottom-to-top geometric sweeps, with secondary left-to-right ordering.

<p>The array is an implicit binary tree.  Entry 0 is the root.  The left and right children of entry i are 2i+1 and 2i+2.
*/

/**
@doctext
@group "VU Meshing"

<h2>VU Meshing</h2>

<p>These functions perform typical meshing operations on a graph, e.g., graph slicing, face and edge splitting, and triangulation of a
monotone face.

<p>The algorithms assume that all VU nodes have the same coordinate space, hence coordinates can be copied blindly to opposite sides of
edges (i.e., there are no seams).
*/

/**
@doctext
@group "VU Graph Stack"

<h2>VU Graph Stack</h2>

<p>The entire node set for a VU graph at any instant can be pushed onto a stack managed by that graph.
Immediately after the push, normal traversal operations will appear to be dealing with an empty graph.
Stacked node sets can later be popped back into the current graph.

<p>After a push, the application can build new structures in the "current" graph, then pop the stacked nodes
back into use, and do operations to combine the now-consolidated nodes.

<h3>Performing Area Booleans with a Graph Stack</h3>

<p>A VU graph stack can be used to perform area boolean operations on the graph.

<p>The overall structure of a boolean operation between two graphs A and B is:

<ol>
<li>Load graph A.</li>
<li>Do all connect and markup operations so that VU_BOUNDARY_EDGE and VU_EXTERIOR_EDGE masks are consistent.
    The following table indicates the particulars of the usual markup sequence:
    <table rows="6" columns="3" border="1">
    <tr>
    <th>Function</th>
    <th>Action Performed</th>
    <th>Key Result</th>
    </tr>
    <tr>
    <td>~mvu_mergeLoops</td>
    <td>Find intersections among edges, sort by angle around vertices</td>
    <td>Graph has all adjacency information for proper subdivision of the plane, but exterior masks are not yet valid</td>
    </tr>
    <tr>
    <td>~mvu_regularizeGraph</td>
    <td>Add bridge edges to holes, split each face into an up-edge chain and a down-edge chain</td>
    <td>Graph is ready for parity and triangulation</td>
    </tr>
    <tr>
    <td>~mvu_markAlternatingExteriorBoundaries</td>
    <td>Apply parity rules to identify exterior/interior faces</td>
    <td>Graph has valid exterior masks</td>
    </tr>
    <tr>
    <td>~mvu_splitMonotoneFacesToEdgeLimit</td>
    <td>Triangulate all faces (send maxEdge = 3)</td>
    <td><i>* Optional</i></td>
    </tr>
    <tr>
    <td>~mvu_flipTrianglesToImproveQuadraticAspectRatio</td>
    <td>Improve triangle shapes</td>
    <td><i>* Optional</i></td>
    </tr>
    </table>
    * Unless there is specific need (e.g., display) to have triangles, it is common to skip the last two steps above.
    Triangulation is more commonly applied <em>after</em> the boolean operation.
</li>
<li>Push graph A onto the graph stack.</li>
<li>Load graph B.</li>
<li>Do all connect and markup operations as were performed on graph A.</li>
<li>Pop the graph stack.  At this point, the graph contains all the edges from both A and B, but the intersections and shared vertices
    are <em>not</em> known.</li>
<li>Apply a stack operation function which merges the combined graph and revises the exterior masks to reflect the properties of the
    boolean composite.
    The folowing table lists the available callbacks for boolean stack operations:
    <table rows="5" columns="2" border="1">
    <tr>
    <th>Callback</th>
    <th>Boolop</th>
    </tr>
    <tr>
    <td>~mvu_andLoops</td>
    <td>A intersect B
    </tr>
    <tr>
    <td>~mvu_orLoops</td>
    <td>A union B
    </tr>
    <tr>
    <td>~mvu_xorLoops</td>
    <td>A xor B
    </tr>
    <tr>
    <td>~mvu_andComplementLoops</td>
    <td>A minus B
    </tr>
    </table>
</li>
<li>The previous step has the possibly undesirable effect of not leaving the graph in as "clean" a state as it was after the regularization
    step.  Hence, it is often necessary to invoke the regularization, alternating boundary and triangulation steps again.</li>
<li>Apply application specific modifications, such as decouple holes from surrounding faces.</li>
<li>Traverse the graph face-by-face, emitting application specific output.</li>
</ol>

<p>It's tempting to try to reduce this sequence to a single function call.  Unfortunately, experience has shown that there are innumerable
variations of the particulars at each step.  On the input side, the amount of preparation can vary widely depending on the data source.  On
the output side, the formats and preferred structure of faces varies wildly.
*/

