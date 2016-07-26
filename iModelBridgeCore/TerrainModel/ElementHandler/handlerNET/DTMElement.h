/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handlerNET/DTMElement.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE

ref class DTMElement;

public enum class DTMContourSmoothingMethod:long
    {
    /// <summary>
    /// No smoothing.
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    None = 0,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    Vertex = 1,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    Spline  = 2,

    /// <summary>
    /// TODO Spu: Ask what to put here. 
    /// Todo SPU: Ask whether we should use a DTM constant here ?
    /// </summary>
    SplineWithoutOverLapDetection  = 3
    };            

//=======================================================================================
// @bsiclass                                                   Steve.Jones 09/10
//=======================================================================================
public ref class StringLocalizer
    {
    private: System::Resources::ResourceManager^ _resourceManager;
    private: System::Globalization::CultureInfo^ _cultureInfo;
    private: static StringLocalizer^ m_stringLocalizer;

    public: property static StringLocalizer^ Instance
        {
        StringLocalizer^ get()
            {
            if (m_stringLocalizer == nullptr)
                {
                m_stringLocalizer = gcnew StringLocalizer();
                }
            return m_stringLocalizer;
            }
        }

    /// <summary>
    /// Initializes a new instance of the StringLocalizer class.
    /// </summary>
    /// <author>Andy.Farr</author>                              <date>01/2007</date>
    private: StringLocalizer ()
        {
        _resourceManager = gcnew System::Resources::ResourceManager("LocalizableStrings", System::Reflection::Assembly::GetExecutingAssembly());
        _cultureInfo = gcnew System::Globalization::CultureInfo (System::Globalization::CultureInfo::CurrentUICulture->Name);
        }
    /// <summary>
    /// Retrieves translated string for given Resource file ID
    /// </summary>
    /// <param name="resourceID"></param>
    /// <returns>The translated string.</returns>
    /// <remarks>If the string was not found the method return the resource id within brackets</remarks>
    public: System::String^ GetLocalizedString (System::String^ resourceID)
        {
        System::String^ cReturn = nullptr;

        if (_resourceManager != nullptr && _cultureInfo != nullptr)
            {
            cReturn = _resourceManager->GetString (resourceID, _cultureInfo);
            }

        if (cReturn == nullptr)
            {
            cReturn = "<- " + resourceID + " ->";
            }

        return cReturn;
        }

    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMSubElement
    {
    protected private:
        Bentley::TerrainModel::Element::DTMSubElementId* m_id;
        DTMElement^ m_dtmElement;
        DTMElementSubHandler::SymbologyParams* m_params;

    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMSubElement(const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement);

    public:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        ~DTMSubElement()
            {
            this->!DTMSubElement();
            }
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        !DTMSubElement();

    virtual void Commit (DTMElement^ element);

    //=======================================================================================
    // @bsimethod                                                   Steve.Jones 07/10
    //=======================================================================================
    property bool Visible
        {
        bool get();
        void set (bool val);
        }

    ////////=======================================================================================
 //////   // @bsimethod                                                   Daryl.Holmwood 07/10
 //////   //=======================================================================================
 //////   public: Bentley::TerrainModel::Element::DTMSubElementId GetSubElementId()
 //////       {
 //////       return *m_id;
 //////       }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: DTMElement^ GetElement()
        {
        return m_dtmElement;
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: property System::UInt32 Color
        {
        virtual System::UInt32 get();
        virtual void set (System::UInt32 value);
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: property virtual System::Int32 LineStyle
        {
        virtual System::Int32 get();
        virtual void set (System::Int32 value);
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: property virtual System::UInt32 Weight
        {
        virtual System::UInt32 get();
        virtual void set (System::UInt32 value);
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: property virtual DGNET::LevelId LevelId
        {
        virtual DGNET::LevelId get();
        virtual void set (DGNET::LevelId value);
        }

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/10
    //=======================================================================================
    public: property virtual System::Double Transparency
        {
        virtual System::Double get();
        virtual void set (System::Double value);
        }
    };

//=======================================================================================
// @bsiclass                                                   Steve.Jones 10/10
//=======================================================================================
public ref class DTMFeatureElement : public DTMSubElement
    {
    internal:
        Bentley::TerrainModel::Element::DTMElementFeaturesHandler::FeatureTypes FeatureType; // ToDo Vancouver make managed

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        DTMFeatureElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement (xAttrId, dtmElement)
            {
            }
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/10
        //=======================================================================================
        property Bentley::TerrainModel::Element::DTMElementFeaturesHandler::FeatureTypes GetFeatureType
            {
            Bentley::TerrainModel::Element::DTMElementFeaturesHandler::FeatureTypes get(); 
            }

    };


//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMSubElementTextStyle : public DTMSubElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMSubElementTextStyle(const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement (xAttrId, dtmElement)
            {
            }

    public:
    //=======================================================================================
    // @bsimethod                                                   Steve.Jones 01/11
    //=======================================================================================
    property DGNET::DgnTextStyle^ TextStyle
        {
        DGNET::DgnTextStyle^ get(); 
        void set (DGNET::DgnTextStyle^ value);
        }
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMContourElement : public DTMSubElementTextStyle
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMContourElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement);
    private:
        DGNET::LevelId m_additionalLevelId;
    public:
        enum class ContourTextPosition : short
            {
            Embeded = 0,
            Above = 1,
            Below = 2
            };

        enum class ContourTextFrequency : short
            {
            Linear = 0,
            Start = 1,
            Middle = 2,
            End = 3
            };

        enum class ContourDrawTextOption : short
            {
            None = 0,
            Always = 1,
            };

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property double ContourInterval
            {
            double get();
            void set (double value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property bool IsMajorContour
            {
            bool get();
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMContourSmoothingMethod ContourSmoothing
            {
            DTMContourSmoothingMethod get();
            void set (DTMContourSmoothingMethod value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/08
        //=======================================================================================
        property int ContourLabelPrecision
            {
            int get();
            void set (int value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property double SmoothingFactor
            {
            double get();
            void set (double value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property int SplineDensification
            {
            int get();
            void set (int value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property ContourDrawTextOption DrawTextOption
            {
            ContourDrawTextOption get();
            void set (ContourDrawTextOption value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DGNET::LevelId TextLevelId
            {
            DGNET::LevelId get ();
            void set (DGNET::LevelId value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property ContourTextPosition TextPosition
            {
            ContourTextPosition get();
            void set (ContourTextPosition value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property ContourTextFrequency TextFrequency
            {
            ContourTextFrequency get();
            void set (ContourTextFrequency value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        property unsigned char MaxSlopeOption
            {
            unsigned char get();
            void set (unsigned char value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        property double MaxSlopeValue
            {
            double get();
            void set (double value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property double TextInterval
            {
            double get();
            void set (double value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property bool NoTextForSmallContours
            {
            bool get();
            void set (bool value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property double SmallContourLength
            {
            double get();
            void set (double value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/10
        //=======================================================================================
        property System::UInt32 DepressionColor
            {
            System::UInt32 get();
            void set (System::UInt32  value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/10
        //=======================================================================================
        property System::Int32 DepressionLineStyle
            { 
            System::Int32 get();
            void set (System::Int32 value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/10
        //=======================================================================================
        property System::UInt32 DepressionWeight
            {
            System::UInt32 get();
            void set (System::UInt32 value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/10
        //=======================================================================================
        virtual void Commit (DTMElement^ element) override;
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMMaterialElement : public DTMSubElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMMaterialElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement(xAttrId, dtmElement)
            {
            }
    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DGNET::MaterialId^ MaterialId
            {
            DGNET::MaterialId^ get ();
            void set (DGNET::MaterialId^ value);
            }
        void SetMaterialInfo (System::String^ palette, System::String^ material);
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMRegionElement : public DTMMaterialElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMRegionElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMMaterialElement(xAttrId, dtmElement)
            {
            }
    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property System::String^ Description
            {
            System::String^ get(); void set(System::String^ value);
            }
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMTrianglesElement : public DTMMaterialElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMTrianglesElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMMaterialElement (xAttrId, dtmElement)
            {
            }
    };

//=======================================================================================
// @bsiclass                                                   Mathieu.St-Pierre    02/10
//=======================================================================================
public ref class DTMRasterDrapingElement : public DTMSubElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMRasterDrapingElement(const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement (xAttrId, dtmElement)
            {
            }
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMPointElement : public DTMSubElementTextStyle
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        DTMPointElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElementTextStyle (xAttrId, dtmElement)
            {
            }
    public:
        virtual void Commit (DTMElement^ element) override;
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        property System::String^ CellName
            {
            System::String^ get();
            void set (System::String^ value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property BGEO::DPoint3d CellSize
            {
            BGEO::DPoint3d get();
            void set (BGEO::DPoint3d value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================		
        property int CellType
            {
            int get();
            void set (int value);
            }

        // Need to split these so FlowArrows dont have this.
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 11/10
        //=======================================================================================
        property bool DisplayText
            {
            bool get();
            void set (bool value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 11/10
        //=======================================================================================
        property System::String^ TextPrefix
            {
            System::String^ get();
            void set (System::String^ value);
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 11/10
        //=======================================================================================
        property System::String^ TextSuffix
            {
            System::String^ get();
            void set (System::String^ value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property BGEO::DPoint3d TextOffset
            {
            BGEO::DPoint3d get();
            void set (BGEO::DPoint3d value);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property short TextAlignment
            {
            short get();
            void set (short value);
            }
    };

//=======================================================================================
// @bsiclass                                                   Steve.Jones 07/10
//=======================================================================================
public ref class DTMFlowArrowElement : public DTMPointElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        DTMFlowArrowElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMPointElement (xAttrId, dtmElement)
            {
            }
    };

//=======================================================================================
// @bsiclass                                                   Steve.Jones 07/10
//=======================================================================================
public ref class DTMLowPointElement : public DTMPointElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        DTMLowPointElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMPointElement (xAttrId, dtmElement)
            {
            }
    public:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/11
        //=======================================================================================
        property double MinimumDepth
            {
            double get();
            void set (double value);
            }
    };

//=======================================================================================
// @bsiclass                                                   Steve.Jones 07/10
//=======================================================================================
public ref class DTMHighPointElement : public DTMPointElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        DTMHighPointElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMPointElement (xAttrId, dtmElement)
            {
            }
    public:
    };

#ifdef INCLUDE_CATCHMENT
//=======================================================================================
// @bsiclass                                                   Steve.Jones 07/10
//=======================================================================================
public ref class DTMCatchmentAreaElement : public DTMSubElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 07/10
        //=======================================================================================
        DTMCatchmentAreaElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement (xAttrId, dtmElement)
            {
            }
    public:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================		
        property double CatchmentAreaMinimumDepth
            {
            double get();
            void set (double value);
            }
    };

//=======================================================================================
// @bsiclass                                                   Steve.Jones 03/11
//=======================================================================================
public ref class DTMPondElement : public DTMSubElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 03/11
        //=======================================================================================
        DTMPondElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMSubElement (xAttrId, dtmElement)
            {
            }
    };
#endif

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMSpotElement : public DTMPointElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMSpotElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMPointElement (xAttrId, dtmElement)
            {
            }
    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property bool DisplayCell
            {
            bool get();
            void set (bool value);
            }

    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 07/08
//=======================================================================================
public ref class DTMFeatureSpotElement : public DTMPointElement
    {
    internal:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        DTMFeatureSpotElement (const Bentley::TerrainModel::Element::DTMSubElementId& xAttrId, DTMElement^ dtmElement) : DTMPointElement (xAttrId, dtmElement)
            {
            }
    public:

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property bool DisplayCell
            {
            bool get();
            void set (bool value);
            }
    };

//=======================================================================================
// @bsiclass                                                   Daryl.Holmwood 06/13
//=======================================================================================
public ref class DTMElement : public DGNET::Elements::DisplayableElement
    {
protected:
    DTMElement (ElementHandleCR eh) : DGNET::Elements::DisplayableElement (eh) {}
    virtual ~DTMElement();
    !DTMElement();
internal:

    void ForceToUseEditElemHandle (EditElementHandleP eeh)
        {
        pin_ptr <byte> arrayPtr = &m_elementHandle[0];
        EditElementHandle* mElementHandle = reinterpret_cast <EditElementHandle*> (arrayPtr);
        mElementHandle->SetElementDescr (eeh->GetElementDescrP(), false, false, eeh->GetModelRef());
        }
    void ReleaseElementHandler ()
        {
        pin_ptr <byte> arrayPtr = &m_elementHandle[0];
        EditElementHandle* mElementHandle = reinterpret_cast <EditElementHandle*> (arrayPtr);
        mElementHandle->Invalidate();
        AdjustFinalizeRequirement();
        }
    static Element^ GetDTMElement (ElementHandleCR eh)
        {
        return gcnew DTMElement (eh);
        }

    internal: 
        DTMFeatureElement^ GetFeatureElement (Bentley::TerrainModel::Element::DTMElementFeaturesHandler::FeatureTypes type);

    public: 

        DTMElement (Bentley::DgnPlatformNET::DgnModel^ model, Element^ templateElement, TerrainModelNET::DTM^ dtm);

        static void RegisterManagedElementHandler();
        TerrainModelNET::DTM^ GetDTM ();

        property bool CanHaveSymbologyOverride
            {
            bool get();
            }

        property bool HasSymbologyOverride
            {
            bool get();
            void set (bool value);
            }
        DTMElement^ GetSymbologyOverrideElement ();

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 03/11
        //=======================================================================================
        property System::String^ Name
            {
            System::String^ get();
            void set (System::String^ val);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 04/11
        //=======================================================================================
        property System::String^ ThematicDisplayStyle
            {
            System::String^ get();
            void set (System::String^ val);
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        cli::array<DTMRegionElement^>^ GetRegionElements();

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        cli::array<DTMSubElement^>^ GetSubElements();

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMTrianglesElement^ TrianglesElement
            {
            DTMTrianglesElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMFeatureElement^ FeatureElement
            {
            DTMFeatureElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureBreaklineElement
            {
            DTMFeatureElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureHoleElement
            {
            DTMFeatureElement^ get();
            }
        
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureIslandElement
            {
            DTMFeatureElement^ get();
            }
        
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureVoidElement
            {
            DTMFeatureElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureBoundaryElement
            {
            DTMFeatureElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 10/10
        //=======================================================================================
        property DTMFeatureElement^ FeatureContourElement
            {
            DTMFeatureElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 01/11
        //=======================================================================================
        property DTMFeatureSpotElement^ FeatureSpotElement
            {
            DTMFeatureSpotElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMSpotElement^ SpotsElement
            {
            DTMSpotElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property DTMFlowArrowElement^ FlowArrowElement
            {
            DTMFlowArrowElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property DTMLowPointElement^ LowPointElement
            {
            DTMLowPointElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property DTMHighPointElement^ HighPointElement
            {
            DTMHighPointElement^ get();
            }

#ifdef INCLUDE_CATCHMENT
        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property DTMCatchmentAreaElement^ CatchmentAreaElement
            {
            DTMCatchmentAreaElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Steve.Jones 08/10
        //=======================================================================================
        property DTMPondElement^ PondElement
            {
            DTMPondElement^ get();
            }
#endif
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMContourElement^ MajorContourElement
            {
            DTMContourElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 07/08
        //=======================================================================================
        property DTMContourElement^ MinorContourElement
            {
            DTMContourElement^ get();
            }

        //=======================================================================================
        // @bsimethod                                                   Sylvain.Pucci 11/06
        //=======================================================================================
        //////property BCDTM::DTM^ DTM
        //////    {
        //////    BCDTM::DTM^ get();
        //////    void set (BCDTM::DTM^ val);
        //////    }

    private:
        //=======================================================================================
        // @bsimethod                                                   Daryl.Holmwood 03/11
        //=======================================================================================
        void DoDispose ();
    };

END_BENTLEY_TERRAINMODELNET_ELEMENT_NAMESPACE
