/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/IPointCloudViewSettings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Interface to change the view presentation settings of pointclouds
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudViewSettings : public IRefCounted
    {
    public:
        enum PointCloudDisplayStyles
            {
            DisplayStyle_None           = 0,
            DisplayStyle_Intensity      = 1,
            DisplayStyle_Classification = 2,
            DisplayStyle_Location       = 3,
            DisplayStyle_Custom         = 4, //New Display style in SS4
            };
    protected:
        virtual bool    _GetUseRGB () const = 0;
        virtual void    _SetUseRGB (bool use) = 0;
        virtual bool    _GetUseIntensity () const = 0;
        virtual void    _SetUseIntensity (bool use) = 0;
        virtual bool    _GetUsePlane () const = 0;
        virtual void    _SetUsePlane (bool use) = 0;
        virtual bool    _GetUseFrontBias () const = 0;
        virtual void    _SetUseFrontBias (bool use) = 0;

        virtual float   _GetContrast () const = 0;
        virtual void    _SetContrast (float val) = 0;
        virtual float   _GetBrightness () const = 0;
        virtual void    _SetBrightness (float val) = 0;
        virtual float   _GetDistance () const = 0;
        virtual void    _SetDistance (float val) = 0;
        virtual float   _GetOffset () const = 0;
        virtual void    _SetOffset (float val) = 0;
      
        virtual bool    _GetACSAsPlaneAxis () const = 0;
        virtual void    _SetACSAsPlaneAxis (bool val) = 0;
        virtual uint16_t _GetPlaneAxis () const = 0;
        virtual void    _SetPlaneAxis (uint16_t axis) = 0;

        virtual uint32_t _GetIntensityRampIndex () const = 0;
        virtual uint32_t _GetPlaneRampIndex () const = 0;
        virtual WStringCR _GetIntensityRamp () const = 0;
        virtual void      _SetIntensityRamp(WCharCP rampName) = 0;
        virtual WStringCR _GetPlaneRamp () const = 0;
        virtual void      _SetPlaneRamp(WCharCP rampName) = 0;


        virtual PointCloudDisplayStyles _GetDisplayStyle () const = 0;
        virtual void                    _SetDisplayStyle (PointCloudDisplayStyles style) = 0;

        //Advanced Information for new kind of display styles
        virtual void    _SetHasAdvancedInfo (bool use) = 0;
        virtual bool    _GetClampIntensity () const = 0;        
        virtual void    _SetClampIntensity (bool use) = 0;
        virtual bool    _GetNeedClassifBuffer   () const = 0;
        virtual void    _SetNeedClassifBuffer   (bool use) = 0;
        virtual WString _GetDisplayStyleName   () const = 0;
        virtual void    _SetDisplayStyleName(WCharCP use) = 0;
        virtual int     _GetDisplayStyleIdx() const = 0;
        virtual void    _SetDisplayStyleIdx(int idx) = 0;

    public:
        /*---------------------------------------------------------------------------------**//**
        * Creates an instance of IPointCloudViewSettings to change the presentation 
        * settings of pointclouds in a view. Changes are only applied when commit is called
        * @param   viewIndex IN View index to retrieve the settings from 
        * @return   Reference counted pointer to an IPointCloudViewSettings instance.
        * @see
        *     Commit
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT static IPointCloudViewSettingsPtr GetPointCloudViewSettings (int viewIndex);
        
        /*---------------------------------------------------------------------------------**//**
        * Gets the intensity ramps.
        * @return   A vector containing the names of the intensity ramps.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT static  bvector<WString> const& GetIntensityRamps ();

        /*---------------------------------------------------------------------------------**//**
        * Gets the plane ramps.
        * @return   A vector containing the names of the plane ramps.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT static  bvector<WString> const& GetPlaneRamps ();

        /*---------------------------------------------------------------------------------**//**
        * Indicates if RGB display option is enabled.
        * @return   true: RGB is enabled; false: disabled.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool    GetUseRGB () const;

        /*---------------------------------------------------------------------------------**//**
        * Enables or disables RGB display option.
        * @param   use IN   true: enables RGB display option; false: disables.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetUseRGB (bool use);

        /*---------------------------------------------------------------------------------**//**
        * Indicates if intensity display option is enabled.
        * @return   true: intensity is enabled; false: disabled.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool    GetUseIntensity () const;

        /*---------------------------------------------------------------------------------**//**
        * Enables or disables intensity display option.
        * @param   use IN   true: enables intensity display option; false: disables.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetUseIntensity (bool use);


        // The plane shader options are not published for the moment, as they're not actively used / tested.

        /*---------------------------------------------------------------------------------**//**
        * This display option indicates to shade by distance from a plane. 
        * @return   true: plane display option is enabled; false: disabled.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool    GetUsePlane () const;

        /*---------------------------------------------------------------------------------**//**
        * This display option indicates to shade by distance from a plane. 
        * @param   use IN   true: enables plane display option; false: disables.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetUsePlane (bool use);

        /*---------------------------------------------------------------------------------**//**
        * When this display option is enabled, more detail is rendered near front during dynamic rendering.
        * @return   true: front bias display option is enabled; false: disabled.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool    GetUseFrontBias () const;

        /*---------------------------------------------------------------------------------**//**
        * When this display option is enabled, more detail is rendered near front during dynamic rendering.
        * @param   use IN   true: enables front bias display option; false: disables.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetUseFrontBias (bool use);

        /*---------------------------------------------------------------------------------**//**
        * Gets the contrast value.
        * @return   The contrast value [0..360].
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT float   GetContrast () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the contrast value.
        * @param   val IN   The contrast value [0..360].
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetContrast (float val);

        /*---------------------------------------------------------------------------------**//**
        * Gets the brightness value.
        * @return   The brightness value [0..360].
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT float   GetBrightness () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the brightness value.
        * @param   val IN   The brightness value [0..360].
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetBrightness (float val);

        /*---------------------------------------------------------------------------------**//**
        * Gets the distance over which the plane shader operates.
        * @return   The distance in meters.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT float   GetDistance () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the distance over which the plane shader operates.
        * @param   val IN   The distance in meters.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetDistance (float val);

        /*---------------------------------------------------------------------------------**//**
        * Gets the offset to the start of the plane shader.
        * @return   The offset.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT float   GetOffset () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the offset to the start of the plane shader.
        * @param   val IN   The offset.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetOffset (float val);

        // use ACS instead of XYZ axis as plane direction
        POINTCLOUDSCHEMA_EXPORT bool    GetACSAsPlaneAxis () const;
        POINTCLOUDSCHEMA_EXPORT void    SetACSAsPlaneAxis (bool val);
        POINTCLOUDSCHEMA_EXPORT uint16_t GetPlaneAxis () const;
        POINTCLOUDSCHEMA_EXPORT void    SetPlaneAxis (uint16_t axis);


        /*---------------------------------------------------------------------------------**//**
        * Gets the intensity ramp index for the current ramp.
        * @return   Index of the ramp or INVALID_RAMP_INDEX if the ramp was not found.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT uint32_t GetIntensityRampIndex () const;

        /*---------------------------------------------------------------------------------**//**
        * Gets the plane ramp index for the current ramp.
        * @return   Index of the ramp or INVALID_RAMP_INDEX if the ramp was not found.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT uint32_t GetPlaneRampIndex () const;

        /*---------------------------------------------------------------------------------**//**
        * Gets the name of the current intensity ramp.
        * @return   Name of the plane ramp or empty string if it is not found.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT WStringCR GetIntensityRamp () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the current intensity ramp.
        * @param   rampName IN   The name of the ramp.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void      SetIntensityRamp(WCharCP rampName);

        /*---------------------------------------------------------------------------------**//**
        * Gets the name of the current plane ramp.
        * @return   Name of the plane ramp or empty string if it is not found.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT WStringCR GetPlaneRamp () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the current plane ramp.
        * @param   rampName IN   The name of the ramp.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void      SetPlaneRamp(WCharCP rampName);

        /*---------------------------------------------------------------------------------**//**
        * Gets the display style.
        * @return   Display style.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT PointCloudDisplayStyles GetDisplayStyle () const;

        /*---------------------------------------------------------------------------------**//**
        * Sets the display style.
        * @param   style IN   The display style.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void                    SetDisplayStyle (PointCloudDisplayStyles style);

        //Advanced Information for new kind of display styles
        POINTCLOUDSCHEMA_EXPORT bool GetClampIntensity () const;
        POINTCLOUDSCHEMA_EXPORT void SetClampIntensity (bool use);
        
        /*---------------------------------------------------------------------------------**//**
        * If we want to display the classification color in a view or we just want to show or hide
        * some points related to some specific classes
        * @return true to use the color to blend\see classes
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool    GetNeedClassifBuffer() const;
        
        POINTCLOUDSCHEMA_EXPORT void    SetNeedClassifBuffer (bool use);

        /*---------------------------------------------------------------------------------**//**
        * returns the name of the style saved in the view
        * @return name of the style in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT WString GetDisplayStyleName() const;

        /*---------------------------------------------------------------------------------**//**
        * set the name of the style saved in the view
        * @param name IN name to set in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetDisplayStyleName(WCharCP name);

        /*---------------------------------------------------------------------------------**//**
        * Get the displaystyle index saved in the view
        * @return idx of the style in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT int     GetDisplayStyleIdx() const;

        /*---------------------------------------------------------------------------------**//**
        * Set the displaystyle index saved in the view
        * @param idx IN idx of the style in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void    SetDisplayStyleIdx (int idx);

        /*---------------------------------------------------------------------------------**//**
        * Says if there's some AdvancedViewSettings into a view or it's a simple DisplayStyle \
        * from SS3
        * some points related to some specific classes
        * @return true if there's an xattribute related to IPointCloudViewSettingsAdvanced in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void SetHasAdvancedInfo(bool use);
    };

/*---------------------------------------------------------------------------------**//**
* Interface to change the classification view presentation settings of pointclouds
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct IPointCloudClassificationViewSettings : public IRefCounted
    {
    protected:
        virtual bool                                _GetState (unsigned char idx) const = 0;
        virtual void                                _SetState (unsigned char idx, bool state) = 0;
        virtual PointCloudColorDefCR                _GetColor (unsigned char idx) const = 0;
        virtual void                                _SetColor(unsigned char idx, PointCloudColorDefR color) = 0;
        virtual unsigned char                       _GetLastIndex () const = 0;
        virtual bool                                _GetBlendColor() const = 0;
        virtual void                                _SetBlendColor(bool use) = 0;
        virtual bool                                _GetUseBaseColor() const = 0;
        virtual void                                _SetUseBaseColor(bool use) = 0;

        virtual BePointCloud::PointCloudColorDef    _GetUnclassColor() const = 0;
        virtual void                                _SetUnclassColor(BePointCloud::PointCloudColorDef& color) = 0;
        virtual bool                                _GetUnclassState() = 0;
        virtual void                                _SetUnclassState(bool state) = 0;
        virtual bool                                _GetClassActiveState(unsigned char idx) = 0;
        virtual void                                _SetClassActiveState(unsigned char idx, bool state) = 0;

    public:
        /*---------------------------------------------------------------------------------**//**
        * Creates an instance of IPointCloudClassificationViewSettings from the context and the modelref,
        * if you want to get the classif buffer from a model in reference the classif buffer will be created from the display style
        * associated with the model ref
        * @param   context IN Context of the request
        * @param   modelRef IN Model of the requested classif view settings
        * @return   Reference counted pointer to an IPointCloudClassificationViewSettings instance.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT static IPointCloudClassificationViewSettingsPtr GetPointCloudClassificationViewSettings(ViewContextCR context);

        /*---------------------------------------------------------------------------------**//**
        * Creates an instance of IPointCloudClassificationViewSettings to change the presentation 
        * settings of pointclouds in a view. Changes are only applied when commit is called
        * @param   viewIndex IN View index to retrieve the settings from 
        * @return   Reference counted pointer to an IPointCloudClassificationViewSettings instance.
        * @see
        *     Commit
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT static IPointCloudClassificationViewSettingsPtr GetPointCloudClassificationViewSettings(int viewIndex);

        /*---------------------------------------------------------------------------------**//**
        * @return  the last index of exposed classifications
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT unsigned char GetLastIndex () const;

        /*---------------------------------------------------------------------------------**//**
        * Returns true if the specified classification id is visible in the view
        * @param idx IN id number of the classification to query the state, 0-GetLastIndex
        * @return true of the classification is visible in the view
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool GetState (unsigned char idx) const;

        /*---------------------------------------------------------------------------------**//**
        * DEPRECATED use GetState instead
        * Returns true if the specified classification id is visible in the view
        * @param idx IN id number of the classification to query the state, 0-GetLastIndex
        * @return true of the classification is visible in the view
        * @see  
        *   GetState
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool GetClassificationState (unsigned char idx) const;

        /*---------------------------------------------------------------------------------**//**
        * Set a classification id has visible or not in the view
        * @param idx IN id number of the classification
        * @param state IN the new state, true for visible
        * @see  
        *    Commit
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void SetState (unsigned char idx, bool state);
        /*---------------------------------------------------------------------------------**//**
        * Returns the RGB color of the the specified classification id in the view
        * @param idx IN id number of the classification       
        * @return color of the classification       
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT PointCloudColorDefCR GetColor (unsigned char idx) const;
        /*---------------------------------------------------------------------------------**//**
        * Sets the color for the specified classification
        * @param idx IN id number of the classification       
        * @param color IN new color to use   
        * @see  
        *    Commit
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void SetColor(unsigned char idx, PointCloudColorDefR color);

        //These function have been added into SS4
        /*---------------------------------------------------------------------------------**//**
        * Returns true if we want to blend the classif with other colors       
        * @return true if we want to blend  
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool GetBlendColor() const;
        /*---------------------------------------------------------------------------------**//**
        * Set if we want to blend the color      
        * @param use IN value to set if we want to blend
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void SetBlendColor(bool use);
        /*---------------------------------------------------------------------------------**//**
        * If we want to use the classif color with a blending    
        * @return true if we want to use only the classif color
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool GetUseBaseColor() const;
        /*---------------------------------------------------------------------------------**//**
        * If we want to use the classif color with a blending    
        * @param use IN Value if we want to use the classif color or the rgb or intensity value
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void SetUseBaseColor(bool use);

        /*---------------------------------------------------------------------------------**//**
        * Gets the color to use for classifications not defined in the current dgn.
        * @return   Color.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT BePointCloud::PointCloudColorDef GetUnclassColor() const ;

        /*---------------------------------------------------------------------------------**//**
        * Sets the color to use for classifications not defined in the current dgn.
        * @param color IN   Color to use.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void               SetUnclassColor(BePointCloud::PointCloudColorDef& color);

        /*---------------------------------------------------------------------------------**//**
        * Gets the state for classifications not defined in the current dgn.
        * @return   true: classifications not defined in the current dgn are visible in the view; false: not visible.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool               GetUnclassState();

        /*---------------------------------------------------------------------------------**//**
        * Sets the state for classifications not defined in the current dgn.
        * @param state IN   true: classifications not defined in the current dgn are visible in the view; false: not visible.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void               SetUnclassState(bool state);

        /*---------------------------------------------------------------------------------**//**
        * Indicates if the specified classification is defined in the current dgn.
        * @param idx IN     Id number of the classification.
        * @return   true: classification is defined; false: not defined.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT bool               GetClassActiveState(unsigned char idx);

        /*---------------------------------------------------------------------------------**//**
        * Sets the specified classification as defined or not.
        * @param idx IN     Id number of the classification.
        * @param state IN   true: sets classification as defined; false: not defined.
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        POINTCLOUDSCHEMA_EXPORT void               SetClassActiveState(unsigned char idx, bool state);
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
