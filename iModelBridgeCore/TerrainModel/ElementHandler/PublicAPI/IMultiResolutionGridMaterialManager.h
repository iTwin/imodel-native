/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/IMultiResolutionGridMaterialManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

/*--------------------------------------------------------------------------------------+
|   Header File Dependencies
+--------------------------------------------------------------------------------------*/
// MicroStation Includes
//#include <bentley.h>
//#include <material.h>
//#include <mstypes.h>

#include "IRasterTextureSource.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

struct MRImageTileId;

typedef  std::vector<MRImageTileId>     MRImageTileIdVector;
typedef  MRImageTileIdVector*           MRImageTileIdVectorP;
typedef  MRImageTileIdVector const*     MRImageTileIdVectorCP;
typedef  MRImageTileIdVector &          MRImageTileIdVectorR;
typedef  MRImageTileIdVector const&     MRImageTileIdVectorCR;

//=======================================================================================
// @bsiclass 
//=======================================================================================
struct MRImageTileId
{
private:

protected:
    UInt   m_layer;
    uint64_t m_row;
    uint64_t m_column;    

public:

    static MRImageTileId invalid_tile_id;

    MRImageTileId () : m_layer(0), m_row(0), m_column(0) {}
    MRImageTileId (UInt layer, uint64_t row, uint64_t col) : m_layer(layer), m_row(row), m_column(col) { }
    ~MRImageTileId() {}

    bool operator == (MRImageTileId const & object) const { return m_layer == object.m_layer && m_row == object.m_row && m_column == object.m_column; }
    bool operator != (MRImageTileId const & object) const { return !this->operator==(object); }
    bool operator <  (MRImageTileId const & object) const
        {
        if (m_layer < object.m_layer)
            return true;
        if (m_layer == object.m_layer)
            {
            if (m_row < object.m_row)
                return true;
            if (m_row == object.m_row)
                return m_column < object.m_column;
            }
        return false;
        }
    bool operator >  (MRImageTileId const & object) const
        {
        if (m_layer > object.m_layer)
            return true;
        if (m_layer == object.m_layer)
            {
            if (m_row > object.m_row)
                return true;
            if (m_row == object.m_row)
                return m_column > object.m_column;
            }
        return false;
        }

    uint64_t  GetRow () const       { return m_row; }
    uint64_t  GetColumn () const       { return m_column; }
    UInt    GetLayer () const       { return m_layer; }
    void    SetRow (uint64_t row)   { m_row = row; }
    void    SetColumn (uint64_t col)   { m_column = col; }
    void    SetLayer (UInt layer)   { m_layer = layer; }

    void    IncRow ()             { ++m_row; }
    void    IncColumn ()             { ++m_column; }

   // static MRImageTileId const& GetInvalidTileId();
};

// Forward declarations
struct ICachedMaterial;

typedef RefCountedPtr<ICachedMaterial> ICachedMaterialPtr;


/*--------------------------------------------------------------------------------------+
|interface ITexturedElement
+--------------------------------------------------------------------------------------*/               
struct ITexturedElement : RefCounted<IRefCounted>
    {            
    protected : 

        virtual const DRange3d& _GetRange() const = 0;
        
        virtual bool            _DrapePointOnElement(DPoint3d& pointInUors) const = 0;    

      //  virtual bool            _TextureAllOverElementRange() const = 0;

    public : 
        
        DTMELEMENT_EXPORT const DRange3d& GetRange() const;

        DTMELEMENT_EXPORT bool            DrapePointOnElement(DPoint3d& pointInUors) const;    

      //bool            TextureAllOverElementRange() const;
    };

typedef RefCountedPtr<ITexturedElement> ITexturedElementPtr;


/*--------------------------------------------------------------------------------------+
|interface ICachedMaterial
+--------------------------------------------------------------------------------------*/  
struct ICachedMaterial : RefCounted <IRefCounted>
    {        

    protected : 


        virtual StatusInt                                _Initialize(const WString& cachedMaterialInitDataP) = 0;

        virtual WString                                  _Serialize() = 0;

        virtual const DRange3d&                          _GetMaterialEffectiveRange() const = 0;

        virtual const MRImageTileId&                     _GetMaterialTileID() const = 0;
        
        //virtual const MaterialProperties* _GetMaterialProperties() const = 0;        

        virtual const Bentley::DgnPlatform::Material*    _GetMaterialProperties() const = 0;                

        virtual const DRange3d&                          _GetTextureRange() const = 0;

        virtual void                                     _GetTextureSizeInPixels(uint32_t& textureWidthInPixels, 
                                                                                 uint32_t& textureHeightInPixels) const = 0;

        virtual SourceID                                 _GetTextureSourceID() const = 0;        

    public :

        DTMELEMENT_EXPORT StatusInt                             Initialize(const WString& cachedMaterialInitDataP);

        DTMELEMENT_EXPORT WString                               Serialize();

        DTMELEMENT_EXPORT const DRange3d&                       GetMaterialEffectiveRange();

        DTMELEMENT_EXPORT const MRImageTileId&                  GetMaterialTileID();
        
        //const MaterialProperties* GetMaterialProperties();        

        DTMELEMENT_EXPORT const Bentley::DgnPlatform::Material* GetMaterialProperties();        

        DTMELEMENT_EXPORT const DRange3d&                       GetTextureRange();

        DTMELEMENT_EXPORT void                                  GetTextureSizeInPixels(uint32_t& textureWidthInPixels, 
                                                                                       uint32_t& textureHeightInPixels);

        DTMELEMENT_EXPORT SourceID                              GetTextureSourceID();   
    };


/*--------------------------------------------------------------------------------------+
|interface IMultiResolutionGridManager
+--------------------------------------------------------------------------------------*/  
struct IMultiResolutionGridManager : RefCounted <IRefCounted>
    {
    protected : 
                                        
        virtual bool               _GetTileExtentInUors(DRange2d& tileExtent, 
                                                        const MRImageTileId& tileId) const = 0;

        virtual void               _GetVisibleTilesInView(MRImageTileIdVectorR      visibleTiles, 
                                                          ViewContextP             pViewContext,
                                                          double                    minScreenPixelsPerDrapePixel,
                                                          const ITexturedElementPtr pTextureElement = 0) const = 0;                
    public : 


        DTMELEMENT_EXPORT bool GetTileExtentInUors(DRange2d&            tileExtent, 
                                                   const MRImageTileId& tileId) const;
        
        DTMELEMENT_EXPORT void GetVisibleTilesInView(MRImageTileIdVectorR      visibleTiles, 
                                                     ViewContextP              pViewContext, 
                                                     double                    minScreenPixelsPerDrapePixel = 1.0,
                                                     const ITexturedElementPtr pTextureElement = 0) const;                               
    }; 

typedef RefCountedPtr<IMultiResolutionGridManager> IMultiResolutionGridManagerPtr;

/*--------------------------------------------------------------------------------------+
|interface IMultiResolutionGridMaterialManager
+--------------------------------------------------------------------------------------*/  
struct IMultiResolutionGridMaterialManager : public IMultiResolutionGridManager 
    {
    protected : 
                        
        virtual void               _ClearCachedMaterials() = 0;           
        
        virtual ICachedMaterialPtr _GetMaterial (const MRImageTileId& gridId, DgnModelRefR modelRef) = 0;

        virtual int                _PrepareLookAhead(MRImageTileIdVectorR visibleTiles, DgnModelRefR modelRef) = 0;

        virtual int                _SetupLookAheadForTile(const MRImageTileId& gridId, DgnModelRefR modelRef) = 0;

    public : 
       

        DTMELEMENT_EXPORT void               ClearCachedMaterials();           
        
        DTMELEMENT_EXPORT ICachedMaterialPtr GetMaterial(const MRImageTileId& gridId, DgnModelRefR modelRef);                        

        DTMELEMENT_EXPORT int                PrepareLookAhead(MRImageTileIdVectorR visibleTiles, DgnModelRefR modelRef);

        DTMELEMENT_EXPORT int                SetupLookAheadForTile(const MRImageTileId& gridId, DgnModelRefR modelRef);        

    }; 

typedef RefCountedPtr<IMultiResolutionGridMaterialManager> IMultiResolutionGridMaterialManagerPtr;


/*--------------------------------------------------------------------------------------+
|class IMultiResolutionGridManagerCreator
+--------------------------------------------------------------------------------------*/  
struct IMultiResolutionGridManagerCreator : RefCounted <IRefCounted>
    {
    protected : 
           
        virtual IMultiResolutionGridManagerPtr         _CreateSimpleManager(DRange2d& dataExtentInUors, 
                                                                            DPoint2d& dataResolutionInUors) = 0; 
        
        virtual IMultiResolutionGridMaterialManagerPtr _CreateMaterialManager(SourceID        rasterTextureSourceID,          //Returned by IRasterTextureSource::GetSourceID()
                                                                              DRange2d&       dataExtentInUors,               //The extent of the data unto which materials will be draped.                                                                           
                                                                              const DPoint2d& defaultMinPixelSizeInUors,      //The default minimum pixel size in UORS to use in case the minimum pixel size cannot be found (unlimited raster).          
                                                                              double          uorsPerMeter) = 0;              //The uors per meter conversion factor
                                                                   
    public : 
                        
        DTMELEMENT_EXPORT IMultiResolutionGridManagerPtr         CreateSimpleManager(DRange2d& dataExtentInUors, 
                                                                                     DPoint2d& dataResolutionInUors); 

        DTMELEMENT_EXPORT IMultiResolutionGridMaterialManagerPtr CreateMaterialManager(SourceID        rasterTextureSourceID,      //Returned by IRasterTextureSource::GetSourceID()
                                                                                       DRange2d&       dataExtentInUors,           //The extent of the data unto which materials will be draped.                                                                                      
                                                                                       const DPoint2d& defaultMinPixelSizeInUors,  //The default minimum pixel size in UORS to use in case the minimum pixel size cannot be found (unlimited raster).                     
                                                                                       double          uorsPerMeter);              //Uors per meter conversion factor
    };  

typedef RefCountedPtr<IMultiResolutionGridManagerCreator> IMultiResolutionGridManagerCreatorPtr;

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
