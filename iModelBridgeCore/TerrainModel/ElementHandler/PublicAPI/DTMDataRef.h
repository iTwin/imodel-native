/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/DTMDataRef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <TerrainModel/ElementHandler/TerrainModelElementHandler.h>


#include <TerrainModel/ElementHandler/IMultiResolutionGridMaterialManager.h>




TERRAINMODEL_TYPEDEFS(IDTM);
//struct QvElem;

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//__PUBLISH_SECTION_END__

struct DTMDrawingInfo;
#ifndef GRAPHICCACHE
struct DTMQvCacheDetails;
#endif

// Purpose/reason for geting a dtm.
enum DTMDataRefPurpose
    {
    None,
    GetRange,
    GetHull,
    GetStatistics,
    DrawTriangles,
    DrawFeatures,
    DrawShadedTriangles,
    GetMrDtm
    };

//__PUBLISH_SECTION_START__
struct DTMDataRef;
typedef RefCountedPtr<DTMDataRef> DTMDataRefPtr;

struct DTMDataRef : RefCounted<IRefCounted>
    {
//__PUBLISH_SECTION_END__
//__PUBLISH_CLASS_VIRTUAL__*/
protected:
    Bentley::DgnPlatform::ElementHandle m_element;

public:
    protected: virtual bool _GetExtents (DRange3dR range) = 0;

    protected: virtual Bentley::TerrainModel::IDTM* _GetDTMStorage (DTMDataRefPurpose purpose, ViewContextR context) = 0;
    protected: virtual Bentley::TerrainModel::IDTM* _GetDTMStorage (DTMDataRefPurpose purpose) = 0;
    protected: virtual StatusInt _GetDTMReferenceStorage (Bentley::TerrainModel::DTMPtr& outDtm) = 0;

    protected: virtual double _GetLastModified()
                   {
                   return m_element.GetElementCP()->ehdr.lastModified;
                   }
    public: Bentley::TerrainModel::IDTM* GetDTM (DTMDataRefPurpose purpose, ViewContextR context);
    public: Bentley::TerrainModel::IDTM* GetDTM (DTMDataRefPurpose purpose);

    protected: virtual bool _IsReadOnly()
                {
                return true;
                }
    protected: virtual bool _IsMrDTM()
                {
                return false;
                }
    
    public: DTMELEMENT_EXPORT virtual Bentley::TerrainModel::Element::IMultiResolutionGridMaterialManagerPtr _GetMultiResGridMaterialManager()
                {
                return 0;
                }

    public:
        DTMELEMENT_EXPORT DTMDataRef();
        DTMELEMENT_EXPORT virtual ~DTMDataRef();

    ElementHandleCR GetElement (void) const
        {
        return m_element;   
        }
    virtual ElementHandleCR GetGraphicalElement (void) const
        {
        return m_element;
        }
#ifndef GRAPHICCACHE
    protected: virtual DTMQvCacheDetails* _GetDTMDetails(ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo) = 0;
    public: DTMELEMENT_EXPORT DTMQvCacheDetails* GetDTMDetails(ElementHandleCR element, DTMDataRefPurpose purpose, ViewContextR context, DTMDrawingInfo& drawingInfo);
#endif

    DTMELEMENT_EXPORT virtual bool CanDrapeRasterTexture();
    public: virtual void UpdateAfterModelUnitDefinitionChange()
        {
        // Default is to do nothing
        }

    //! Gets the DTM for the the purpose. The DTM returned is in the original storage units
    //! @param[in] purpose The reason for the DTM.
    //! @param[in] context The view context for which the DTM is required.
    //! @return The DTM.
    public: DTMELEMENT_EXPORT Bentley::TerrainModel::IDTM* GetDTMStorage (DTMDataRefPurpose purpose, ViewContextR context);

    //! Gets the DTM for the the purpose. The DTM returned is in the original storage units
    //! @param[in] purpose The reason for the DTM.
    //! @return The DTM.
    public: DTMELEMENT_EXPORT Bentley::TerrainModel::IDTM* GetDTMStorage (DTMDataRefPurpose purpose);
    //! Gets the the DTM in UORs.
    //! @param [out] outDTM the DTM
    //! @param [in] displayElem The Element Handle of the dtm.
    public: DTMELEMENT_EXPORT StatusInt GetDTMReferenceDirect (Bentley::TerrainModel::DTMPtr& outDtm);

    //! Gets the DTM in original storage units
    //! @param [out] outDtm the DTM.
    public: DTMELEMENT_EXPORT StatusInt GetDTMReferenceStorage (Bentley::TerrainModel::DTMPtr& outDtm);

    //! Gets the DTM using the current Transformation
    //! @param [out] outDtm the DTM.
    //! @param [in] displayElem The Display Element.
    public: DTMELEMENT_EXPORT StatusInt GetDTMReference (Bentley::TerrainModel::DTMPtr& outDtm, TransformCR currTrans);

//__PUBLISH_SECTION_START__
    //! Gets the extents of the DTM.
    //! @param [out] low The low point of the DTM.
    //! @param [in] high The high point of the DTM.
    public: DTMELEMENT_EXPORT bool GetExtents (DRange3dR range);

    //! Is the DTM Read Only.
    //! @return true if the DTM is readonly
    public: DTMELEMENT_EXPORT bool IsReadOnly();

    //! Is the DTM a STM?
    //! @return true if the DTM is A STM?
    public: DTMELEMENT_EXPORT bool IsMrDTM();
//__PUBLISH_SECTION_END__

    //! Gets a point on the DTM from a projection.
    //! @param [out] pointOnDTM the point on the dtm.
    //! @param [in] The DTM Element.
    //! @param [in] viewport The viewport to project using.
    //! @param [in] The point to test.
    //! @return true if the point was found on the DTM.
   public: DTMELEMENT_EXPORT bool GetProjectedPointOnDTM (DPoint3d& pointOnDTM, ElementHandleCR thisElm, ViewportP viewport, const DPoint3d& testPoint);

    //! Gets a point on the DTM from a projection.
    //! @param [out] pointOnDTM the point on the dtm.
    //! @param [in] The DTM Element.
    //! @param [in] w2vMap the matrix to use to project.
    //! @param [in] The point to test.
    //! @return true if the point was found on the DTM.
    public: DTMELEMENT_EXPORT bool GetProjectedPointOnDTM (DPoint3d& pointOnDTM, ElementHandleCR thisElm, DMatrix4d const & w2vMap, const DPoint3d& testPoint);

    protected: virtual bool GetProjectedPointOnDTM
                   (
                   DPoint3dR pointOnDTM,
                   ElementHandleCR thisElm,
                   DMatrix4dCR w2vMap,
                   DPoint3dCR testPoint,
                   ViewportP viewport
                   );

    //! Gets a last Modified date of the DTM.
    //! @return the last modified date.
    public: DTMELEMENT_EXPORT double GetLastModified();

    static DTMDataRef* GetDTMAppData (ElementHandleCR element);
    static void AddDTMAppData (ElementHandleCR element, DTMDataRef* dtmDataRef);
    static void DropDTMAppData (ElementHandleCR element);
    };

//__PUBLISH_SECTION_START__
END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE



