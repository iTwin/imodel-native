/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/DTMElementHandlerManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
/*__PUBLISH_SECTION_START__*/
#pragma once

#include <TerrainModel\ElementHandler\DTMDataRef.h>
#include <TerrainModel\ElementHandler\IRasterTextureSourceManager.h>

//#include "DgnPlatform\DgnPlatform.h"
//#d:\BSI\Topaz\src\DgnPlatform\PublicAPI\DgnPlatform\DgnPlatform.h

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct IMultiResolutionGridManagerCreator;                                                                  
typedef RefCountedPtr<IMultiResolutionGridManagerCreator> IMultiResolutionGridManagerCreatorPtr;

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//=======================================================================================
// @bsistruct                                            Daryl.Holmwood     08/2011
//=======================================================================================

/*__PUBLISH_SECTION_END__*/
class DTMElementIterator
    {
    private:
        bvector<ElementHandle> m_elements;
        unsigned int m_index;

        void ScanModel(DgnModelRefP modelRef);
    public:
        DTMElementIterator(DgnModelRefP modelRef, bool includeRef);
        ElementHandle Current()
            {
            return m_elements[m_index];
            }
        bool MoveNext()
            {
            m_index++;
            return m_index < m_elements.size();
            }
        void Reset()
            {
            m_index = -1;
            }
    };


/*__PUBLISH_SECTION_START__*/

/*=================================================================================**//**
* @addtogroup DTMElement
This is the handler to manage DTMElement creation and retrieval
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class DTMElementHandlerManager
    {
/*__PUBLISH_CLASS_VIRTUAL__*/
    public:
        struct IDTMIsFriendModelExtension
            {
            virtual bool IsFriendModel (ElementHandleCR element, DgnModelRefP modelRef) = 0;
            };
/*__PUBLISH_SECTION_END__*/
        struct IDTMElementPowerPlatformExtension
            {
            virtual DgnPlatform::DgnTextStylePtr GetActiveStyle (DgnFileR file) = 0;
            virtual void EnsureSharedCellDefinitionExists (WCharCP cellName, DgnModelRefP modelRef) = 0;
            };
    private:
    static int s_mrDTMactivationRefCount;
    static IDTMElementPowerPlatformExtension* s_DTMElementPowerPlatformExtension;
    static IDTMIsFriendModelExtension*        s_DTMIsFriendModel;
    static IMultiResolutionGridManagerCreatorPtr s_multiResolutionGridManagerCreator;
    static IRasterTextureSourceManager*          s_rasterTextureSourceManagerP;
    static bool                                  s_isDrawForAnimation;
    
    DTMElementHandlerManager() {}

public:
    DTMELEMENT_EXPORT static void Initialize (IDTMElementPowerPlatformExtension* value);

    DTMELEMENT_EXPORT static IDTMElementPowerPlatformExtension* GetDTMElementPowerPlatformExtension ()
        {
        return s_DTMElementPowerPlatformExtension;
        }

    static int GetMrDTMActivationRefCount()
                {    
                return s_mrDTMactivationRefCount;
                }        

    static IMultiResolutionGridManagerCreatorPtr GetMultiResolutionGridManagerCreator()
                {
                return s_multiResolutionGridManagerCreator;
                }

    static IRasterTextureSourceManager* GetRasterTextureSourceManager()
                {
                return s_rasterTextureSourceManagerP;
                }
   
    static bool IsFriendModel (ElementHandleCR element, DgnModelRefP modelRef)
        {
        if (s_DTMIsFriendModel != nullptr)
            return s_DTMIsFriendModel->IsFriendModel (element, modelRef);
        return false;
        }

/*__PUBLISH_SECTION_START__*/
    DTMELEMENT_EXPORT static void SetIsFriendModelExtension (IDTMIsFriendModelExtension* value);

    //! Test if the ElementHandle is a DTMElement
    //! @param[in] element the Element Handle to test
    //! @return if the DTM is an Element
    public: DTMELEMENT_EXPORT static bool IsDTMElement(ElementHandleCR element);
    //! Gets the DTMDataRef from an ElementHandle
    //! @param[out] the DTMDataRef
    //! @param[in] element the Element Handle to test
    //! @return Errorcode
    public: DTMELEMENT_EXPORT static StatusInt GetDTMDataRef(RefCountedPtr<DTMDataRef>& outRef, ElementHandleCR element);

    //public: DTMELEMENT_EXPORT static void ScheduleFromDtmFile(EditElementHandleR editHandle, const WChar* fileName);
    public: DTMELEMENT_EXPORT static void ScheduleFromMrDtmFile(DgnModelRefP dgnModelRefP, EditElementHandleR editHandle, const DgnDocumentPtr& mrdtmDocumentPtr, bool inCreation);
    public: DTMELEMENT_EXPORT static void ActivateMrDTM();
    public: DTMELEMENT_EXPORT static void DeactivateMrDTM();    
    public: DTMELEMENT_EXPORT static StatusInt ConvertMrDTMtoSingleResDTM(EditElementHandleR outputEh, EditElementHandleCR inputEh, long maxNbPointsInDTM, DgnModelRefP modelRef);    


    public: DTMELEMENT_EXPORT static void SetMultiResolutionGridManagerCreator(IMultiResolutionGridManagerCreatorPtr multiResolutionGridManagerCreator);   

    public: DTMELEMENT_EXPORT static void SetRasterTextureSourceManager(IRasterTextureSourceManager* rasterTextureSourceManagerP);   

    public: DTMELEMENT_EXPORT static bool IsDrawForAnimation();    
    public: DTMELEMENT_EXPORT static void SetDrawForAnimation(bool isDrawForAnimation);   
                                

    //! Creates an editHandle from the dtm, using current transform is one exists.
    //! @param editHandle IN OUT this is the editHandle which will be set to the DTMElement, it will use the Model set in the EditHandle.
    //! @param storageTransformation IN the transformation from UORS to storage units.
    //! @param dtm IN the DTM to add.
    //! @param disposeDTM IN release the DTM memory and assocate with the DTMElement (see remarks)
    //! @returns SUCCESS if all went ok, otherwise ERROR.
    //! @remarks If you aren't going to use the DTM after adding then set disposeDTM.
    //! @note the editHandle needs to be added to the Model.
    public: DTMELEMENT_EXPORT static StatusInt ScheduleFromDtm (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, TransformCR storageTransformation, DgnModelRefR modelRef, bool disposeDTM = false);

    //! Creates an editHandle from the dtm in UORs.
    //! @param editHandle IN OUT this is the editHandle which will be set to the DTMElement, it will use the Model set in the EditHandle.
    //! @param dtm IN the DTM to add.
    //! @param disposeDTM IN release the DTM memory and assocate with the DTMElement (see remarks)
    //! @remarks If you aren't going to use the DTM after adding then set disposeDTM.
    //! @note the editHandle needs to be added to the Model.
    public: DTMELEMENT_EXPORT static StatusInt ScheduleFromDtmDirect (EditElementHandleR editHandle, ElementHandleCP templateElement, BcDTMR dtm, DgnModelRefR modelRef, bool disposeDTM = false);

    //! Replaces the DTM in an DTM Element with a new DTM.
    //! @param editHandle IN the DTM Element to replace
    //! @param dtm IN the DTM to replace it with.
    //! @param disposeDTM IN release the DTM memory and assocate with the DTMElement (see remarks)
    //! @return status, SUCCESS if all went to plan otherwise ERROR.
    //! @remarks If you aren't going to use the DTM after adding then set disposeDTM.
    public: DTMELEMENT_EXPORT static StatusInt ScheduleReplaceDtm (EditElementHandleR editHandle, BcDTMR dtm, bool disposeDTM = false);

    public: DTMELEMENT_EXPORT static void GetStorageToUORMatrix (Transform& trsf, DgnModelRefP model, ElementHandleCR element, bool withExaggeration = true);
    public: DTMELEMENT_EXPORT static void GetStorageToUORMatrix (Transform& trsf, ElementHandleCR element);
    public: DTMELEMENT_EXPORT static void SetStorageToUORMatrix (const Transform& trsf, EditElementHandleR element);

    //! Set the Name of the DTM Element
    //! @param editHandle IN the element to set the name for.
    //! @param name IN the name to set.
    public: DTMELEMENT_EXPORT static void SetName (EditElementHandleR editHandle, WCharCP name);

    //! Set the Name of the DTM Element
    //! @param elRef IN the element to set the name for.
    //! @param name IN the name to set.
    public: DTMELEMENT_EXPORT static void SetName (ElementRefP elRef, WCharCP name);

    //! Get the Name of the DTM Element
    //! @param element IN the element to get the name for.
    //! @param name OUT the name of the element.
    public: DTMELEMENT_EXPORT static StatusInt GetName (ElementHandleCR element, WStringR name);

    //! Get the LastModifiedDate of the DTM Data
    //! @param element IN the element to get the name for.
    //! @return the last modified date.
    public: DTMELEMENT_EXPORT static double GetDTMLastModified (ElementHandleCR element);

    //! Set the ThematicDisplayStyleIndex of the Element for display of Triangles
    //! @param editHandle IN the element to set the name for.
    //! @param displayStyleIndex IN the displayStyleIndex to set.
    public: DTMELEMENT_EXPORT static void SetThematicDisplayStyleIndex (EditElementHandleR editHandle, int displayStyleIndex);

    //! Get the ThematicDisplayStyleIndex of the DTM Element
    //! @param element IN the element to get the name for.
    //! @param displayStyleIndex OUT the displayStyleIndex of the Element.
    public: DTMELEMENT_EXPORT static StatusInt GetThematicDisplayStyleIndex (ElementHandleCR element, int& displayStyleIndex);

    //! Gets the Element which is used for symbology
    //!@param element IN the element
    //!@param symbologyElem OUT the element which is used for symbology
    //!@param destinationModel IN the model which is being displayed?
    //!@return true if it is element not the same as the symbology element.
    public: DTMELEMENT_EXPORT static bool GetElementForSymbology (ElementHandleCR element, ElementHandleR symbologyElem, DgnModelRefP destinationModel);

    //! Traverses DgnModelRefP and calls callback function for each instance
    //! @param element IN modelRef
    //! @param includeRefs IN not root model only
    //! @param callback IN user function
    //! @param userArgs IN passed to callback
    //! @note When callback function returns anything else than SUCCESS traversing is finished
    public: typedef StatusInt (*TraverseCallBack) (ElementHandleCR element, void *userArgs);
    public: DTMELEMENT_EXPORT static void Traverse (DgnModelRefP modelRef, bool includeRefs, TraverseCallBack callback, void *userArgs);


    //! Create a terrain model element based on the symbology setting of a scalable terrain model element.            
    //! @param tmEditHandle OUT TM element handle
    //! @param stmEditHandle IN STM element handle
    //! @param singleResolutionDtm IN terrain model data    
    //! @param modelRef IN modelRef for tmEditHandle
    //! @return the status of the operation
    public: DTMELEMENT_EXPORT static StatusInt CreateDTMFromMrDTM(EditElementHandleR tmEditHandle, EditElementHandleCR stmEditHandle, BcDTMPtr singleResolutionDtm, DgnModelRefP modelRef);        


/*__PUBLISH_SECTION_END__*/
    public: static bool FindDTMData(ElementHandleCR element, ElementHandleR dataEl);
    public: static void AddToModelInOwnBlock (EditElementHandleR editHandle, DgnModelRefP model);

    public: static DTMElementIterator* ScanForDTMInModel(DgnModelRefP modelRef); 
    public: DTMELEMENT_EXPORT static void CheckAndCreateElementDescr (EditElementHandleR elemHandle, ElementHandleCP templateElement, const Bentley::DgnPlatform::ElementHandlerId& handlerId, TransformCR trsf, DgnModelRefR modelRef);
    public: DTMELEMENT_EXPORT static void CheckAndCreateElementDescr107 (EditElementHandleR elemHandle, const Bentley::DgnPlatform::ElementHandlerId & handlerId, TransformCR trsf, DgnModelRefR modelRef);
    public: DTMELEMENT_EXPORT static void CreateDefaultDisplayElements (EditElementHandleR elemHandle);

/*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
