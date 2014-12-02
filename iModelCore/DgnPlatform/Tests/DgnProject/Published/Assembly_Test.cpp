/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/Assembly_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_SQLITE 

#if defined (NEEDS_WORK_DGNITEM)
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ElementId addLineToAssembly (DgnModelR model, ElementId assemblyId = ElementId())
    {
    EditElementHandle line;
    DSegment3d  segment;
    
    segment.point[0] = DPoint3d::FromXYZ (0.0, 0.0, 0.0); 
    segment.point[1] = DPoint3d::FromXYZ (100.0, 100.0, 100.0);
    EXPECT_EQ (SUCCESS, LineHandler::CreateLineElement (line, NULL, segment, model.Is3d(), model));

    line.GetElementDescrP()->SetAssemblyId(assemblyId);
    EXPECT_EQ (SUCCESS, line.AddToModel());

    return line.GetElementId();
    }

//=======================================================================================
// For testing AssemblyIterators
// @bsiclass                                                    Keith.Bentley   05/13
//=======================================================================================
struct ExpectedIds : bvector<ElementId> 
{
    void Add (ExpectedIds const& other) {insert(end(), other.begin(), other.end());}
    void Add (ElementId id) {push_back(id);}
    void Drop (ElementId id){erase (std::find (begin(), end(), id));}

    /*---------------------------------------------------------------------------------**//**
    * Test an AssemblyIterator against this expected result (a vector of ElementId's). Each expected
    * member should be found exactly once. When the iteration is finished, the expected vector should be empty.
    * @bsimethod                                    Keith.Bentley                   05/13
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TestIter (AssemblyIterator const& iterator, ElementId aId)
        {
        for (auto it : iterator)
            {
            ASSERT_TRUE (it.GetElementRef()->IsAssembledBy(aId));

            auto found = std::find (begin(), end(), it.GetElementId());
            ASSERT_TRUE (found != end()); // it should be here
            erase (found);                // remove it so we can tell we got each entry once and only once
            }

        ASSERT_EQ (0, size());
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(Assemblies, Changes)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"3dMetricGeneral.idgndb");
    DgnProjectR project = *tdm.GetDgnProjectP();
    DgnModelR model = *tdm.GetDgnModelP();

    // create a line (not in an assembly)
    ElementId line1 = addLineToAssembly(model);
    ASSERT_TRUE (line1.IsValid());

    // create another line (not in an assembly)
    ElementId line2 = addLineToAssembly(model);
    ASSERT_TRUE (line2.IsValid());

    // relate a few elements to line2 as an assembly
    ExpectedIds line2Members;
    for (int i=0; i<3; ++i)
        line2Members.Add(addLineToAssembly (model, line2));

    // create a line to be used as subassembly of line2
    ElementId line3 = addLineToAssembly (model, line2);
    line2Members.Add(line3);

    // relate a few elements to line3 as a sub-assembly
    ExpectedIds line3Members;
    for (int i=0; i<4; ++i)
        line3Members.Add(addLineToAssembly (model, line3));

    // now create 3-level deep assemblies off of line3members
    ElementId e3_0_1 = addLineToAssembly (model, line3Members[0]);
    ElementId e3_1_1 = addLineToAssembly (model, line3Members[1]);
    ElementId e3_1_2 = addLineToAssembly (model, line3Members[1]);
    ElementId e3_2_1 = addLineToAssembly (model, line3Members[2]);
    ElementId e3_3_1 = addLineToAssembly (model, line3Members[3]);

    ExpectedIds line3SubMember; // save these ids for testing below
    line3SubMember.Add(e3_0_1);
    line3SubMember.Add(e3_1_1);
    line3SubMember.Add(e3_1_2);
    line3SubMember.Add(e3_2_1);
    line3SubMember.Add(e3_3_1);

    // make sure that "IsAssembledBy" works successfully
    ASSERT_TRUE (project.Models().GetElementById(e3_3_1)->IsAssembledBy(line2));

    // this should return false because it is not assembled by line1 (nothing is at this point).
    ASSERT_FALSE (project.Models().GetElementById(e3_3_1)->IsAssembledBy(line1));

    // this should return false because we're looking for an invalid assemblyId
    ASSERT_FALSE (project.Models().GetElementById(e3_3_1)->IsAssembledBy(ElementId()));

    project.SaveChanges(); // just so we can verify what's happened in another process for debugging

    // now try some iterators. First try a nest=0 iterator over line2. That should only find line2Members
    PersistentElementRefPtr line2El = project.Models().GetElementById(line2);
    ExpectedIds expected(line2Members); // these are the elements that should be in line2 with no nesting
    expected.TestIter (line2El->MakeAssemblyIterator(0), line2);

    // now try a 1 level deep iterator. That should find line2Members plus line3Members (but not line3SubMembers)
    expected = line2Members;
    expected.Add(line3Members);
    expected.TestIter (project.MakeAssemblyIterator(line2,1), line2);

    // now try a infinite depth iterator. That should find line2Members, plus line3Members, plus line3SubMembers
    expected = line2Members;
    expected.Add(line3Members);
    expected.Add(line3SubMember);
    expected.TestIter (project.MakeAssemblyIterator(line2), line2);

    // attempt to create a circular assembly (should fail)
    EditElementHandle eeh (line3, model);
    eeh.GetElementDescrP()->SetAssemblyId(e3_2_1);
    StatusInt stat = eeh.ReplaceInModel(eeh.GetElementRef());
    ASSERT_EQ (stat, DGNMODEL_STATUS_CircularDependency);

    // attempt to create an assembly reference to an element that doesn't exist (should fail)
    eeh.GetElementDescrP()->SetAssemblyId(ElementId (99999999));
    stat = eeh.ReplaceInModel(eeh.GetElementRef());
    ASSERT_EQ (stat, DGNMODEL_STATUS_BadAssemblyId);

    // attempt to delete an element that has assembly references (should fail)
    stat = eeh.DeleteFromModel();
    ASSERT_EQ (stat, DGNMODEL_STATUS_ForeignKeyConstraint);

    // re-establish the element handle to point to line3 and try to clear its assembly id
    stat = eeh.FindById(line3, &model);
    ASSERT_EQ (stat, SUCCESS);
    eeh.GetElementDescrP()->SetAssemblyId(ElementId());
    stat = eeh.ReplaceInModel(eeh.GetElementRef());
    ASSERT_EQ (stat, DGNMODEL_STATUS_Success);

    // make sure that line2 now only has its original members (we dropped line 3 above)
    expected = line2Members;
    expected.Drop(line3);
    expected.TestIter (project.MakeAssemblyIterator(line2), line2);

    // this should not find anything. Line1 doesn't have any assembly references
    expected.TestIter (project.MakeAssemblyIterator(line1), line1);

    // now change line3 so it references line1 as its assembly
    eeh.GetElementDescrP()->SetAssemblyId(line1);
    stat = eeh.ReplaceInModel(eeh.GetElementRef());
    ASSERT_EQ (stat, DGNMODEL_STATUS_Success);

    project.SaveChanges(); // just so we can verify what's happened in another process for debugging

    // now iterating over line1's members, we shoud find line3, line3Members and line3SubMembers.
    expected = line3Members;
    expected.Add(line3);
    expected.Add(line3SubMember);
    expected.TestIter (project.MakeAssemblyIterator(line1), line1);
    }

#endif
