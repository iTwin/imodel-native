/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ForeignFormat/NonPublished/XGraphics_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#ifdef WIP_XGRAPHICS_MERGE

#include "ForeignFormatTests.h"
#include <DgnPlatform/ForeignFormat/DgnProjectImporter.h>
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_FOREIGNFORMAT
USING_NAMESPACE_BENTLEY_DGNPLATFORM
struct XGraphicsTests: public testing::Test
    {
    public:
        ScopedDgnHost m_autoDgnHost;
        DgnProjectPtr m_project;    
        DgnModelP m_modelP;
        bvector <Utf8CP> m_modelNames;
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Prepares test data file
        //---------------------------------------------------------------------------------------
        void PrepareDefaultModel()
            {
            DgnDbTestDgnManager tdm(L"XGraphicsElements.idgndb", __FILE__, OPENMODE_READWRITE);
            //ScopedDgnHost autoDgnHost;
            m_project = tdm.GetDgnProjectP();
            DgnModels& modelTable =  m_project->Models();
            DgnModelId id = modelTable.QueryModelId("Default");
            m_modelP =  m_project->Models().GetModelById (id);
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        //---------------------------------------------------------------------------------------
        void loadModel(Utf8CP name)
            {
            DgnModels& modelTable =  m_project->Models();
            DgnModelId id = modelTable.QueryModelId(name);
            m_modelP =  m_project->Models().GetModelById (id);
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Adds models that contain test data
        //---------------------------------------------------------------------------------------
        void SetUp()
            {
            m_modelNames.push_back("Splines");
            PrepareDefaultModel();
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     03/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool isXGraphicsSymbol (ElementRefP ref)
            {
            return XAttributeHandle (ref, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_SymbolTransform), 0).IsValid();
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     03/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool hasXGraphics (ElementRefP ref)
            {
            return XAttributeHandle (ref, XAttributeHandlerId (XATTRIBUTEID_XGraphics, XGraphicsMinorId_Data), 0).IsValid();
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     03/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        BentleyStatus createXGraphicsElement (EditElementHandleR newElement, ElementHandleCR originalElement, XGraphicsContainerR container)
            {
            ExtendedElementHandler::InitializeElement (newElement, &originalElement, originalElement.GetDgnModelP(), true, false);
            if (SUCCESS != container.AddToElement (newElement) || SUCCESS != newElement.GetDisplayHandler()->ValidateElementRange (newElement, false))
                return ERROR;
            if (isInvalidRange (newElement.GetElementCP()->hdr.dhdr.range))
                return ERROR;
            Display_attribute dispAttr;
            if (mdlElement_displayAttributePresent (originalElement.GetElementCP(), TRANSPARENCY_ATTRIBUTE, &dispAttr))
                {
                // Mimic DisplayHandler.cpp's handling of adding this attribute.
                ScopedArray <byte>  elmData (newElement.GetElementCP()->Size() + 2*sizeof (dispAttr));
                DgnElementP          tmpElement = (DgnElementP) elmData.GetData();
                newElement.GetElementCP()->CopyTo (*tmpElement);
                mdlElement_displayAttributeAdd (tmpElement, &dispAttr);
                newElement.ReplaceElement (tmpElement);
                }
            return SUCCESS;
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     04/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        static bool isInvalidRange (ScanRangeCR range)
            {
            // Some XGraphics streams contains operations with invalid ranges (2009_1182A-3DFP02I in Hospital).
            // These end up not being drawn in MicroStation - just throw them away when optimizing.
            return range.xlowlim > range.high.x || range.low.y > range.high.y || range.low.z > range.high.z;
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     04/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        ECInstanceKey ecKeyFromElement (ElementHandleCR eh)
            {
            if (NULL == eh.GetElementDescrCP())
                return ECInstanceKey();

            return ECInstanceKey (eh.GetElementDescrCP()->GetECClassId(), eh.GetElementDescrCP()->GetECInstanceId());
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   MattGooding     04/13
        // Method copied from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool setECKeyForElement (OptimizedElementMap& map, ElementHandleCR eh)
            {
            bool wasSuccess = map.insert (make_bpair (eh.GetElementDescrCP()->GetElementRef(), make_bpair (ecKeyFromElement (eh), ElementAgenda()))).second;
            BeAssert (wasSuccess);
            return wasSuccess;
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Checks if data buffer has required format that should be: 
        // XGraphicsHeader + (operationCode + operationSize + operationData) x manyTimes
        //---------------------------------------------------------------------------------------
        bool       IsValidBuffer (byte * buffer, size_t size)
            {
            byte *pEnd, *pData;
            for (pData = buffer + sizeof(XGraphicsHeader), pEnd = buffer + size; pData < pEnd;)
                {
                UInt16  opCode;
                UInt32  opSize;
                if (SUCCESS != XGraphicsOperations::GetOperation (opCode, opSize, pData, pEnd))
                    return false;
                pData += (opSize - sizeof(opSize));
                }
            return pEnd == pData;
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Method mimicking method DropXGraphicsSymbols rutine from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool DropXGraphicsSymbols (EditElementHandleR eeh)
            {
            XGraphicsContainer container;
            //Extract data from element to container 
            if (SUCCESS != container.ExtractFromElement (eeh))
                return false;
            container.SetElement (eeh.GetElementDescrCP()->GetElementRef());
            //Optimize container by dropping XGraphics
            EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container data buffer does not met format requirements";
            if (SUCCESS != container.Optimize (XGRAPHIC_OptimizeOptions_RemoveSymbols))
                return false;
            EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container data buffer does not met format requirements";
            EXPECT_EQ(0, container.GetSymbolCount())<<"Failed to drop symbols";
            //Save changes
            container.SetUseCache (true);
            StatusInt replaced = container.ReplaceOnElement (eeh);
            EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container data buffer does not met format requirements";
            //Verify changes
            if(replaced == SUCCESS)
                {
                //Check that all XGraphics symbols were dropped
                XGraphicsContainer emptyContainer;
                EXPECT_EQ(SUCCESS, emptyContainer.ExtractFromElement(eeh));
                EXPECT_TRUE(IsValidBuffer(emptyContainer.GetData(), emptyContainer.GetDataSize()))<<"Container data buffer does not met format requirements";
                EXPECT_EQ(0, emptyContainer.GetSymbolCount())<<"Element should not contain symbols as they were dropped.";
                }
            return (replaced == SUCCESS);
            }
        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Method mimicking FlattenTransforms rutine from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool FlattenTransforms (ElementAgendaR elements)
            {
            bool wasModified = false;
            ElementAgenda results;
            for (auto& iter : elements)
                {
                XGraphicsContainer container;
                if (SUCCESS != container.ExtractFromElement (iter) || SUCCESS != container.FlattenTransforms ())
                    continue;
                EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container's data buffer does not met requirements";
                if (SUCCESS == container.ReplaceOnElement (iter))
                    wasModified = true;
                EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container data buffer does not met format requirements";
                }
            return wasModified;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Julija Suboc     07/13
        // Method mimicking SplitLArgeMeshes rutine from DgnProjectOptimize.cpp file
        //---------------------------------------------------------------------------------------
        bool SplitLargeMeshes (ElementAgendaR elements)
            {
            bool wasModified = false;
            ElementAgenda results;
            for (auto& iter : elements)
                {
                // If the element isn't a mesh or doesn't contain XGraphics, nothing to do.
                XGraphicsContainer container; //, containerToVerify;
                if (SUCCESS != container.ExtractFromElement (iter) &&
                    (MESH_HEADER_ELM != iter.GetElementType() || SUCCESS != container.CreateFromElement (iter)))
                    {
                    results.Insert (iter);
                    continue;
                    }
                //containerToVerify.ExtractFromElement(iter);
                EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container's data buffer does not met requirements";
                iter.GetElementType();
                bvector <XGraphicsContainer> splitMeshes;
                if (SUCCESS != container.SplitMeshesByFaceCount (splitMeshes, 30))
                    {
                    results.Insert (iter) ;
                    continue;
                    }
                wasModified = true;
                for (auto& meshIter : splitMeshes)
                    {
                    EXPECT_TRUE(IsValidBuffer(meshIter.GetData(), meshIter.GetDataSize()))<<"Container's data buffer does not met requirements";
                    EditElementHandle eeh;
                    if (SUCCESS == createXGraphicsElement (eeh, iter, meshIter))
                        results.Insert (eeh);
                    }
                if (MESH_HEADER_ELM != iter.GetElementType() && !container.IsEmpty())
                    {
                    container.ReplaceOnElement (iter);
                    EXPECT_TRUE(IsValidBuffer(container.GetData(), container.GetDataSize()))<<"Container's data buffer does not met requirements";
                    // This is no longer the same element it was in the original file - don't maintain ID.
                    iter.GetElementDescrP()->el.ehdr.uniqueId = 0;
                    results.Insert (iter);
                    }
            }
            EXPECT_TRUE(elements.size()<= results.size())<<"Result should contain the same amount of elements or more.";
            elements = results;
            return wasModified;
            }
   };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     06/13
//---------------------------------------------------------------------------------------
TEST_F(XGraphicsTests, DropXGraphicsSymbol)
    {
     while(!m_modelNames.empty())
        {
        Utf8CP modelName = m_modelNames.back();
        m_modelNames.pop_back();
        loadModel(modelName);
        GraphicElementRefList* list = m_modelP->GetGraphicElementsP();
        if (NULL != list)
            //Try to pass every graphic element in model 
            for each (PersistentElementRefP elm in *m_modelP->GetGraphicElementsP())
                {
                ElementHandle eh(elm);
                ASSERT_TRUE(eh.IsValid())<<"Failed to create element handle.";
                ElementAgenda agenda;
                bool isXGraphics = isXGraphicsSymbol(elm);
                if (isXGraphics)
                    continue;
                if (!elm->IsGraphics() || SHAREDCELL_DEF_ELM == elm->GetElementType()) 
                    continue;
                EditElementHandle eeh;
                eeh.Duplicate(eh);
                int type = elm->GetElementType();
                if (type != MESH_HEADER_ELM && type != SHARED_CELL_ELM && type != CELL_HEADER_ELM && hasXGraphics (elm))
                    {
                     // If this is a non-complex element referring to XGraphics symbols, DropXGraphicsSymbols will remove those references.
                    if(DropXGraphicsSymbols (eeh))
                        //When trying to remove references for second time DropXGraphics should return false  
                        EXPECT_FALSE(DropXGraphicsSymbols (eeh))<<"Some elements still contain XGraphics symbols.";
                    }
                }
         }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     06/13
//
//---------------------------------------------------------------------------------------
TEST_F(XGraphicsTests, SplitLargeMeshes)
    {
    while(!m_modelNames.empty())
        {
        Utf8CP modelName = m_modelNames.back();
        m_modelNames.pop_back();
        loadModel(modelName);
        GraphicElementRefList* list = m_modelP->GetGraphicElementsP();
        if (NULL != list)
            //Try to pass every graphic element in model
            for each (PersistentElementRefP elm in *m_modelP->GetGraphicElementsP())
                {
                ElementHandle eh(elm);
                ElementRefP originalRef = eh.GetElementRef();
                ASSERT_TRUE(eh.IsValid())<<"Failed to create element handle.";
                ElementAgenda agenda;
                if (isXGraphicsSymbol (elm) || !elm->IsGraphics() || SHAREDCELL_DEF_ELM == elm->GetElementType())
                    continue;
                EditElementHandle eeh;
                eeh.Duplicate(eh);
                OptimizedElementMap results;
                bool wasModified = false;
                 switch (eeh.GetElementType())
                    {
                    case MESH_HEADER_ELM:
                        {
                        setECKeyForElement (results, eeh);
                        results[originalRef].second.Insert (eeh);
                        break;
                        }
                    case SHARED_CELL_ELM:
                    case CELL_HEADER_ELM:
                        continue;
                    default:
                        {
                        if (!hasXGraphics (originalRef))
                            continue; 
                        wasModified |= DropXGraphicsSymbols (eeh);
                        setECKeyForElement (results, eeh);
                        results[originalRef].second.Insert (eeh);
                        break;
                        }
                    }
             for (auto& agendaIter : results)
                {
                    if(SplitLargeMeshes (agendaIter.second.second))
                        EXPECT_FALSE(SplitLargeMeshes (agendaIter.second.second))<<"There are still some meshes with face count bigger than required.";
                }
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija Suboc     06/13
//
//---------------------------------------------------------------------------------------
TEST_F(XGraphicsTests, FlattenTransforms)
    {
    while(!m_modelNames.empty())
        {
        Utf8CP modelName = m_modelNames.back();
        m_modelNames.pop_back();
        loadModel(modelName);
        GraphicElementRefList* list = m_modelP->GetGraphicElementsP();
        if (NULL != list)
            //Try to pass every graphic element in model
            for each (PersistentElementRefP elm in *m_modelP->GetGraphicElementsP())
                {
                ElementHandle eh(elm);
                ElementRefP originalRef = eh.GetElementRef();
                ASSERT_TRUE(eh.IsValid())<<"Failed to create element handle.";
                ElementAgenda agenda;
                // No optimizations performed for these types.
                if (isXGraphicsSymbol (elm) || !elm->IsGraphics() || SHAREDCELL_DEF_ELM == elm->GetElementType())
                    continue;
                EditElementHandle eeh;
                eeh.Duplicate(eh);
                OptimizedElementMap results;
                bool wasModified = false;
                 switch (eeh.GetElementType())
                    {
                    case MESH_HEADER_ELM:
                        {
                        setECKeyForElement (results, eeh);
                        results[originalRef].second.Insert (eeh);
                        break;
                        }
                    case SHARED_CELL_ELM:
                    case CELL_HEADER_ELM:
                        continue;
                    default:
                        {
                        if (!hasXGraphics (originalRef))
                            continue;
                        wasModified |= DropXGraphicsSymbols (eeh);
                        setECKeyForElement (results, eeh);
                        results[originalRef].second.Insert (eeh);
                        break;
                        }
                    }
             for (auto& agendaIter : results)
                    if(FlattenTransforms (agendaIter.second.second))
                        EXPECT_FALSE(FlattenTransforms (agendaIter.second.second));
            }
        }
    }

#endif