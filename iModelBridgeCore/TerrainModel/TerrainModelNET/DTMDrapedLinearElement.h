/*--------------------------------------------------------------------------------------+
|
|     $Source: TerrainModelNET/DTMDrapedLinearElement.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

using namespace System::Collections;

#include "Bentley.Civil.DTM.h"
#include "dtmhelpers.h"
BEGIN_BENTLEY_TERRAINMODELNET_NAMESPACE


//=======================================================================================
/// <summary>
/// Defines code for section points.
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public enum class DTMDrapedLinearElementPointCode
    {
    /// <summary>
    /// Defines a point external to the Tin
    /// </summary>
    External = 0,

    /// <summary>
    /// Defines a point in a triangle
    /// </summary>
    Triangle = 1,

    /// <summary>
    /// Defines a point on breakline
    /// </summary>
    BreakLine = 2,

    /// <summary>
    /// Defines a point On A Break Triangle. 
    /// The drape point is on a triangle edge that is not a break line, 
    /// But one or both of the other triangle edges are a break line.
    /// </summary>
    BreakTriangle = 3,

    /// <summary>
    /// Defines a point in void
    /// </summary>
    Void = 4,

    /// <summary>
    /// Defines a drape point coincident with a triangle point
    /// </summary>
    TrianglePoint = 5,

    /// <summary>
    /// Defines a drape point coincident with a triangle edge
    /// </summary>
    TriangleEdge = 6
    };
//=======================================================================================
/// <summary>
/// Used To Store DTM Feature Information At A Drape Point
/// </summary>                
/// <author>Rob.Cormack</author>                                  <date>07/2011</date>
//=======================================================================================
public value struct DTMDrapePointFeature
    {
    private:

        long            m_dtmFeatureIndex;        // Index To DTM Feature In DTM Feature Table
        DTMFeatureType  m_dtmFeatureType;         // DTM Feature Type  
        ::DTMUserTag    m_dtmUserTag;             // DTM Feature User Tag
        DTMFeatureId    m_dtmFeatureId;           // DTM Feature ID
        long            m_priorFeaturePoint;      // Index To Prior Feature Point Before The Drape Point
        long            m_nextFeaturePoint;       // Index To Next  Feature Point After  The Drape Point

    internal:

        DTMDrapePointFeature (long dtmFeatureIndex,DTMFeatureType dtmFeatureType,::DTMUserTag dtmUserTag,DTMFeatureId dtmFeatureId,long priorFeaturePoint,long nextFeaturePoint)
           {
            m_dtmFeatureIndex   = dtmFeatureIndex;
            m_dtmFeatureType    = dtmFeatureType;
            m_dtmUserTag        = dtmUserTag;
            m_dtmFeatureId      = dtmFeatureId;
            m_priorFeaturePoint = priorFeaturePoint ;
            m_nextFeaturePoint  = nextFeaturePoint ;
           }

    public:

        //=======================================================================================
        /// <summary>
        /// Get DTM Feature Index
        /// </summary>                
        //=======================================================================================
        property long DtmFeatureIndex
            {
            long get()
                {
                return m_dtmFeatureIndex;
                }
            }


        //=======================================================================================
        /// <summary>
        /// Get DTM Feature Type
        /// </summary>                
        //=======================================================================================
        property DTMFeatureType DtmFeatureType
            {
            DTMFeatureType get()
                {
                return m_dtmFeatureType;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get DTM Feature ID
        /// </summary>                
        //=======================================================================================
        property DTMFeatureId DtmFeatureId
            {
            DTMFeatureId get()
                {
                return m_dtmFeatureId;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get DTM Feature User Tag
        /// </summary>                
        //=======================================================================================
        property ::DTMUserTag DtmUserTag 
            {
            ::DTMUserTag get()
                {
                return m_dtmUserTag ;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get DTM Feature Prior Point
        /// </summary>                
        //=======================================================================================
        property long DtmFeaturePriorPoint
            {
            long get()
                {
                return m_priorFeaturePoint;
                }
            }


        //=======================================================================================
        /// <summary>
        /// Get DTM Feature Prior Point
        /// </summary>                
        //=======================================================================================
        property long DtmFeatureNextPoint
            {
            long get()
                {
                return m_nextFeaturePoint;
                }
            }


   };

//=======================================================================================
/// <summary>
/// Contains the definition of a section point.
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref struct DTMDrapedLinearElementPoint
    {
    private:

        DTMDrapedLinearElementPointCode m_code;
        double m_distanceAlong;
        BGEO::DPoint3d m_coordinates;
        array<DTMFeatureId>^ m_featureIds;
        array<DTMDrapePointFeature>^ m_features;

    public : 

        //=======================================================================================
        /// <summary>
        /// Section point code
        /// </summary>
        /// <author>Sylvain.Pucci</author>                              <date>1/2008</date>
        //=======================================================================================
        property DTMDrapedLinearElementPointCode Code
            {
            DTMDrapedLinearElementPointCode get () { return m_code; };
            };

        //=======================================================================================
        /// <summary>
        /// Distance along the section
        /// </summary>
        /// <author>Sylvain.Pucci</author>                              <date>1/2008</date>
        //=======================================================================================
        property double DistanceAlong
            {
            double get () { return m_distanceAlong; };
            };

        //=======================================================================================
        /// <summary>
        /// Coordinates of the section point
        /// </summary>
        /// <author>Sylvain.Pucci</author>                              <date>1/2008</date>
        //=======================================================================================
        property BGEO::DPoint3d Coordinates
            {
            BGEO::DPoint3d get () { return m_coordinates; };
            };

        //=======================================================================================
        /// <summary>
        /// Array of feature Ids on this draped point.
        /// </summary>
        /// <author>Sylvain.Pucci</author>                              <date>1/2008</date>
        //=======================================================================================
        property array<DTMFeatureId>^ FeatureIds
            {
            array<DTMFeatureId>^ get()
                {
                return m_featureIds;
                }
            }


        //=======================================================================================
        /// <summary>
        /// Array of DTM Features At This Draped point.
        /// </summary>
        /// <author>Sylvain.Pucci</author>                              <date>7/201</date>
        //=======================================================================================
        property array<DTMDrapePointFeature>^ Features
            {
            array<DTMDrapePointFeature>^ get()
                {
                return m_features;
                }
            }

    internal:

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTMDrapedLinearElementPoint struct.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>1/2008</date>
        //=======================================================================================
        DTMDrapedLinearElementPoint (BcDTMDrapedLinePoint* pt)
            {
            DPoint3d curPoint;
            pt->AddRef();
            m_code = (DTMDrapedLinearElementPointCode) pt->GetCode();
            pt->GetPointCoordinates(curPoint);
            int numFeatures = pt->GetFeatureIdCount();
            m_featureIds = gcnew array<DTMFeatureId>(numFeatures);
            m_features = gcnew array<DTMDrapePointFeature>(numFeatures);
            for(int i = 0; i < numFeatures; i++)
            {
                m_featureIds[i] = DTMFeatureId (pt->GetFeatureIdAtIndex(i));
                m_features[i] = DTMDrapePointFeature
                (
                 pt->GetFeatureIndexAtIndex(i),
                 DTMFeatureType(pt->GetFeatureTypeAtIndex(i)),
                 pt->GetUserTagAtIndex(i),
                 DTMFeatureId(pt->GetFeatureIdAtIndex(i)),
                 pt->GetFeaturePriorPointAtIndex(i),
                 pt->GetFeatureNextPointAtIndex(i)
                 ) ;
            }     
            DTMHelpers::Copy (m_coordinates, curPoint);
            m_distanceAlong = pt->GetDistanceAlong();
            pt->Release();
            }
    };

//=======================================================================================
/// <summary>
/// Defines a DTM section.
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMDrapedLinearElement: public System::Collections::Generic::IEnumerable<DTMDrapedLinearElementPoint^>
    {
    private: 

        BcDTMDrapedLine* m_dtmDrapedLine;

    internal: 


        virtual System::Collections::IEnumerator^ GetEnumeratorInternal() sealed = System::Collections::IEnumerable::GetEnumerator;


    public: 

        //=======================================================================================
        /// <summary>
        /// Initializes a new instance of the DTMDrapedLinearElement class.
        /// </summary>                    
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        [EditorBrowsable(EditorBrowsableState::Never) ]
        DTMDrapedLinearElement (BcDTMDrapedLine* dtmDrapedLine);

        !DTMDrapedLinearElement ();
        ~DTMDrapedLinearElement ();

        [EditorBrowsable(EditorBrowsableState::Never) ]
        property BcDTMDrapedLine*  ExternalHandle
            {
            BcDTMDrapedLine* get()
                {
                return m_dtmDrapedLine;
                }
            }

        //=======================================================================================
        /// <summary>
        /// Get an enumerator DTMDrapedLinearElementPoint.
        /// </summary>                    
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        virtual System::Collections::Generic::IEnumerator<DTMDrapedLinearElementPoint^>^ GetEnumerator();
    };

//=======================================================================================
/// <summary>
/// Defines an enumerator for DTM section points.
/// </summary>                
/// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
//=======================================================================================
public ref class DTMDrapedLinearElementPointEnumerator 
: public System::Collections::Generic::IEnumerator<DTMDrapedLinearElementPoint^>
    {
    private:

        BcDTMDrapedLine* m_dtmDrapedLine;
        int m_index;

        property System::Object^ CurrentObject  
            {  
            virtual Object^ get() sealed = IEnumerator::Current::get;  
            } 

        ~DTMDrapedLinearElementPointEnumerator();
        !DTMDrapedLinearElementPointEnumerator();  

    internal:

        DTMDrapedLinearElementPointEnumerator (BcDTMDrapedLine* dtmDrapedLine);

    public:

        //=======================================================================================
        /// <summary>
        /// Gets the current section point.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        property DTMDrapedLinearElementPoint^ Current  
            {  
            virtual DTMDrapedLinearElementPoint^ get();  
            }  

        //=======================================================================================
        /// <summary>
        /// Moves to the next point.
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        virtual bool MoveNext(void);  

        //=======================================================================================
        /// <summary>
        /// Resets the enumerator (goes just before the first feature)
        /// </summary>                
        /// <author>Sylvain.Pucci</author>                              <date>08/2005</date>
        //=======================================================================================
        virtual void Reset(void);  
    };

END_BENTLEY_TERRAINMODELNET_NAMESPACE 
