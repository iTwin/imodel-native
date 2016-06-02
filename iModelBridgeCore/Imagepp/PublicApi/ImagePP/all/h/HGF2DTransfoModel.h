//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DTransfoModel.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DTransfoModel
//-----------------------------------------------------------------------------
// Description of 2D transformation model.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DShape.h>
#include "HFCMatrix.h"

BEGIN_IMAGEPP_NAMESPACE

class HGF2DDisplacement;
class HGF2DLiteExtent;


/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This ABSTRACT class encapsulates a 2D transformation model. An object of this
    class expresses the bi-directional transformations to apply to coordinates.
    Coordinates can be transformed directly or inverse.

    The inverse or direct labels can be seen as simple labels to two bi-directional
    doors that lead to a black box which is the transformation model.

    To each input/output channel is assigned distance units. If the units are not
    the same, unit conversion will be performed.

    The following is a representation of a transformation model. All descendants
    of a transformation model have this general structure. Different kinds of
    transformation models implement different transformation equations.



    The interface also implements some methods which are practical, even if they
    are not essential to the proper operation of the defined structure.
    The IsIdentity(), IsStretchable() and GetStretchParams()ask the transformation
    model if it can be represented by a simple identity model, a stretching and permit
    to obtain the stretch parameters. These will permit a lot of optimization in
    image processing, which is simple to apply to a grid of data such as raster.

    In addition to these, there are added methods that increase the flexibility and
    the usage of transformation models, if more than one transformation model is
    required. Any transformation model must possess the ability to Reverse().
    After reversing of a transformation model, the role of the direct and inverse
    input/output channels is reversed. This also applies to the channels units.
    Any transformation model must possess the ability to compose with any other
    transformation model. The process of composition here is defined as the ability
    to generate a transformation model from two transformation models. This new
    transformation model can be of the same type as either one of the composed models,
    or of a completely different type. This generation of a new model may result in
    an HGF2DComplexTransfoModel, a descendant of the present class, but nevertheless
    known and used. A HGF2DComplexTransfoModel is simply a list of copies of the
    composed transformation model. This last class enables composition of any transformation
    models whatever the type. The process of composition either binds the inverse channels
    of first model with direct channels of second model or the inverse channel
    of the first model with the inverse of the second.
    The generation of a complex transformation model, is nevertheless a way of
    representing the composition of transformation models which cannot be matched,
    or do not understand the structure of each other. Generation of such complex transformation
    models does not improve the performance over performing the transformation in
    sequence through the two original transformation model. To generate a more
    efficient model, it is advised that as much as possible, a kind of transformation
    model should be able to compile transformation parameters into its own parameters
    for transformation models of more simple kinds. For example, the parameters of a
    translation transformation model should be extracted and compiled into the parameters
    of a similitude transformation model.

    Since transformation models are used as HGF2DTransfoModel not as known specific
    type, an efficient way of performing composition requires models to
    be able to ask their type to each other. This is the main purpose of
    the GetClassID() method. Each kind of method must register a unique ID,
    and this ID must be returned by the method. The method can also be used in
    the application to cast the model to a more precise type, to prevent
    virtual call of the conversion interface for performance reason.

Updates dating June 2014
    Due to the new support for geographic coordinate system we were forced to
    introduce the notion of Domain. The domain is the area around which the transformation is defined
    Defining and reporting a domain is not recommended if possible. It can be more efficient
    to find ways to augment till infinity the domain by linear adaptation 
    (triangular or rectangular affine or perspective mesh based transformation
    with an external domain mean projective).

    Unfortunately in some case such strategy is difficult to implement
    and it is more efficiant to announce right away the presence of a limiting domain.
    The domain is expressed as either direct of inverse channel coordinates. The domain
    is the same but coordinates are expressed in either direct or inverse.
    The domain is specified by use of a HGF2DShape.

    The fact a domain can be present imposes that all calling 
    code that performs coordinate conversion must either check the transformation
    model has a domain or check the returned value of the convert method.

    In addition to this new introduction we had loosened the definition of the transformation model.
    The old transformation imposed that the transformation be defined for the 
    whole cartesian plane in both direction. The introduction of domains waves off this rule.
    The second chjange in defintion concers the already supported projective (perspective)
    linear transformation. Even though linear, the transformation posseses inherent to 
    the mathemnatics of the transformation a vanishing line or horizon line. Coordinates 
    located upon this horizon line cannot be converted since an infinity of solutions are
    then available. For the moment we will trust users not to convert coordinates located
    on two different sides of this vanishing line a situation is usually pointless.
    If however a coordinate close or at the vanishing line is performed we expect
    a value to be returned.

    Example:
    @code
        HGF2DIdentity       MyIdentityModel();

        // An affine is a kind of transformation model
        HGF2DAffine         MyAffineModel;

        // An HGF2DSimilitude is a kind of HGF2DTransfoModel
        HGF2DSimilitude        MySimilitudeModel();

        if (MyAffineModel.CanBeRepresentedByAMatrix())
            HFCMatrix<3, 3> MyMatrix = MyModel.GetMatrix();

        HFCPtr<HGF2DTransfoModel> pComposedModel1 = MySimilitudeModel.ComposeInverseWithDirectOf(MyAffineModel);

        HFCPtr<HGF2DTransfoModel> pComposedModel2 = MySimilitudeModel.ComposeInverseWithInverseOf(MyAffineModel);


        // Conversion
        double     NewX;
        double     NewY;
        MyAffineModel.ConvertDirect(10.0, 10.4, &NewX, &NewY)
        MyAffineModel.ConvertInverse(10.0, 10.4, &NewX, &NewY)

        double     ImageWidth;
        double     ImageHeight;
        MyAffineModel.ConvertDirect (100, 100, ImageWidth, ImageHeight);
        MyAffineModel.ConvertInverse (ImageWidth, ImageHeight, WorldPosX, WorldPosY);

        double     XArray[100];
        double     YArray[100];
        MyAffineModel.ConvertDirect (12.5, 4.5, 100, 0.5, XArray, YArray);
        MyAffineModel.ConvertInverse (12.5, 4.5, 100, 0.5, XArray, YArray);

        // Matrix extraction
        if (MyAffineModel.CanBeRepresentedByAMatrix())
            HFCMatrix<3, 3> MyMatrix = MyAffineModel.GetMatrix();


        // Model simplification
        HFCPtr<HGF2DtransfoModel> pSimple = MyAffineModel.CreateSimplifiedModel();
        if (pSimple != 0)
            // My model was simplified. Use pSimple instead
        else
            // Impossible to simplify, continue to use my model.


        // Stretch parameters extraction
        if (MyModel.IsStretchable  ())
        {
            double ScaleX;
                       double ScaleY;
            HGF2DDisplacement MyTrans;
            MyModel.GetStretchParams (&ScaleX,
                                      &ScaleY,
                                      &MyTrans);
            ...
        }
    @end

    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGF2DTransfoModel : public HFCShareableObject<HGF2DTransfoModel>
    {
    HDECLARE_BASECLASS_ID(HGF2DTransfoModelId_Base)

public:

    typedef uint32_t TypeId;

    IMAGEPP_EXPORT virtual           ~HGF2DTransfoModel();

    // Conversion interface
    /** -----------------------------------------------------------------------------
        This method converts direct the given coordinates by the relation of the
        HGF2DTransfoModel. The transformation model really transforms distances,
        therefore, the methods must be capable of transforming the given input
        into distances. The numbers given are raw values and are interpreted in the
        units set in the model at its construction. This therefore prevents
        requiring the convert method of performing a unit conversion, but lets the
        user check that the values refer to the proper units.

        @param pio_pXInOut IN OUT Pointer to double that contains on
                            input the X value to convert and receives the converted
                            X value on output.

        @param pio_pYInOut IN OUT Pointer to double that contains on input the
                            Y value to convert and receives the converted Y
                            value on output.

        @see ConvertInverse()
        -----------------------------------------------------------------------------
    */
    //! Computation intensive algorithms can call ConvertDirect/ConvertInverse concurrently.
    //! Override this method if the descendant HGF2DTransfoModel is thread safe.
    bool IsConvertDirectThreadSafe() const;
    bool IsConvertInverseThreadSafe() const;

    IMAGEPP_EXPORT StatusInt ConvertDirect (double* pio_pXInOut, double* pio_pYInOut) const;

    IMAGEPP_EXPORT StatusInt ConvertDirect (double    pi_YIn,
                                            double    pi_XInStart,
                                            size_t     pi_NumLoc,
                                            double    pi_XInStep,
                                            double*   po_aXOut,
                                            double*   po_aYOut) const;

    IMAGEPP_EXPORT StatusInt ConvertDirect(double    pi_XIn,
                                           double    pi_YIn,
                                           double*   po_pXOut,
                                           double*   po_pYOut) const;

    IMAGEPP_EXPORT StatusInt ConvertDirect(size_t    pi_NumLoc,
                                           double*   pio_aXInOut,
                                           double*   pio_aYInOut) const;


    IMAGEPP_EXPORT StatusInt ConvertPosDirect (HGF2DPosition* pio_rpCoord) const;

    /** -----------------------------------------------------------------------------
        This method converts inverse the given coordinates by the relation of the
        HGF2DTransfoModel. The transformation model really transforms distances,
        therefore, the method must be capable of transforming the given input into
        distances. The numbers given are raw values, so the values are
        interpreted in the units set in the model at its construction. This
        therefore prevents requiring the convert method of performing a unit
        conversion, but lets the user check that the values refer to the proper
        units.

        @param pio_pXInOut IN OUT Pointer to double that contains on input
                            the X value to convert and receives the converted X
                            value on output.

        @param pio_pYInOut IN OUT Pointer to double that contains on input the Y
                            value to convert and receives the converted Y
                            value on output.

        @see ConvertDirect()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT StatusInt ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const;

    IMAGEPP_EXPORT StatusInt ConvertInverse(double    pi_YIn,
                                            double    pi_XInStart,
                                            size_t     pi_NumLoc,
                                            double    pi_XInStep,
                                            double*   po_aXOut,
                                            double*   po_aYOut) const;

    IMAGEPP_EXPORT StatusInt ConvertInverse(double    pi_XIn,
                                            double    pi_YIn,
                                            double*   po_pXOut,
                                            double*   po_pYOut) const;

    IMAGEPP_EXPORT StatusInt ConvertInverse(size_t    pi_NumLoc,
                                            double*   pio_aXInOut,
                                            double*   pio_aYInOut) const;

    IMAGEPP_EXPORT StatusInt ConvertPosInverse(HGF2DPosition* pio_rpCoord) const;



    
    /** -----------------------------------------------------------------------------
    This method returns true if the present instance of the transformation model
    can be represented by an identity transformation model. This implies that
    the model contains no transformation. If not overridden this method always
    returns false.

    @return A Boolean value. true if the model can be represented completely by
    an identity, and false otherwise

    @see GetStretchParams()
    @see CanBeRepresentedByAMatrix()
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT bool IsIdentity() const;


    /** -----------------------------------------------------------------------------
    This method returns true if the present instance of the transformation model
    can be represented by a stretch. This implies that the model only contains a
    translation component and scaling factors. If not overridden this method
    always returns false.
    The optional angle tolerance parameter when provided indicates that some small
    specified amount of rotation can be disregarded in the determination of
    stretcheability of the model.

    @param pi_AngleTolerance IN OPTIONAL An angular tolerance interpreted in
    radians. The default value is 0 meaning that no
    rotation is acceptable.

    @return A Boolean value. true if the model can be represented completely by
    a stretch, and false otherwise.

    @see GetStretchParams()
    @see CanBeRepresentedByAMatrix()
    -----------------------------------------------------------------------------*/
    IMAGEPP_EXPORT bool IsStretchable(double pi_AngleTolerance = 0) const;

    /** -----------------------------------------------------------------------------
        This method permits to extract the stretch parameters. These stretch parameters
        are the translation and scaling components of the model. The user must call
        the IsStretchable() method in order to know if the stretch parameters are
        the only transformation components in the model. If this is not the case,
        the values returned are the best stretch parameters that can approximate
        the model at the origin (0, 0).

        @param po_pScaleFactorX OUT Pointer to double that receives the scaling factor
                                component in the first dimension for the transformation
                                model.

        @param po_pScaleFactorY OUT Pointer to double that receives the scaling factor
                                component in the second dimension for the transformation
                                model.

        @param po_pDisplacement OUT Pointer to an HGF2DDisplacement object that receives
                                the translation component of the model.


        @see IsStretchable()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT void GetStretchParams(double*  po_pScaleFactorX,
                                         double*  po_pScaleFactorY,
                                         HGF2DDisplacement* po_pDisplacement) const;

    IMAGEPP_EXPORT void GetStretchParamsAt(double*  po_pScaleFactorX,
                                           double*  po_pScaleFactorY,
                                           HGF2DDisplacement* po_pDisplacement,
                                           double   pi_XLocation,
                                           double   pi_YLocation,
                                           double   pi_AreaSize = -1.0) const;


    IMAGEPP_EXPORT HGF2DTransfoModel* Clone() const;

    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel>     ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;
    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel>     ComposeInverseWithInverseOf(const HGF2DTransfoModel& pi_rModel) const;

    /** -----------------------------------------------------------------------------
        This method indicates if the transformation model can be completely
        represented by a transformation matrix

        @param true if the model can be completely represented by a transformation
               matrix, and false otherwise.

        @see GetMatrix()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool CanBeRepresentedByAMatrix() const;

    /** -----------------------------------------------------------------------------
        These methods extract the matrix or equation parameters from the transformation
        model. These parameters correspond and are named according to the following
        expressed in the 2D-transformation matrix.

        To use this method, the user must first make sure that a matrix can represent
        the transformation model. To do this, call CanBeRepresentedByAMatrix().

        Use of the matrix is a bit tricky since some of the positions are unitless
        and some are not. The a02 is the X translation, a12 is the Y translation.
        Both are expressed in the corresponding units of the DIRECT channel.
        A20 and A21 are projection parameters. They are expressed in the inverse
       of the X and Y direct channels units. All other numbers of the matrix are unitless.

        @see CanBeRepresentedByAMatrix()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT HFCMatrix<3, 3>  GetMatrix() const;

    /** -----------------------------------------------------------------------------
    This method attempts to create a new transformation model that is a simplified,
    more efficient form of the current model. If this is not possible, a null
    pointer will be returned.

    @return A smart pointer to the new simplified model, or null if the model
    is already in its simplest form.
    -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel> CreateSimplifiedModel() const;

    // Geometric properties

    /** -----------------------------------------------------------------------------
        This method returns true if the present instance of the transformation model
        possesses the linearity preserving transformation property.
        A transformation model that conserves linearity guaranties that given a
        straight line, which is transformed through the model, will result in a
        straight line.

        @return A Boolean value. true if the model preserves the geometric
                transformation property, and false otherwise.

        @see PreserverParallelism()
        @see PreservesShape()
        @see PreservesDirection()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool     PreservesLinearity() const;

    /** -----------------------------------------------------------------------------
        This method returns true if the present instance of the transformation model
        possesses the indicated parallelism preserving property.
        A transformation model which preserves parallelism guaranties that given a
        line (straight or curved) which is parallel to some other differently positioned
        in space line (again straight or curved), both which are transformed through
        the model, will result in lines (straight or curved) which are still parallel
        one to the other.

        @return A Boolean value. true if the model preserves the geometric
                transformation property, and false otherwise.

        @see PreserverLinearity()
        @see PreservesShape()
        @see PreservesDirection()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool     PreservesParallelism() const;
    /** -----------------------------------------------------------------------------
        This method returns true if the present instance of the transformation model
        possesses the indicated shape preserving property.
        A transformation model that preserves shape guaranties
        that given a vector (of any type) which possesses some shape, which is
        transformed through the model will result in a vector that is of the
        same shape.
        A transformation model that preserves shape automatically conserves parallelism
        since if it were not the case, then the shape could be changed.

        @return A Boolean value. true if the model preserves the geometric
                transformation property, and false otherwise.

        @see PreserverParallelism()
        @see PreservesLinearity()
        @see PreservesDirection()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool     PreservesShape() const;

    /** -----------------------------------------------------------------------------
        This method returns true if the present instance of the transformation model
        possesses the indicated direction preserving property.
        A transformation model that preserves direction guaranties that given a line,
        which is transformed through the model, will result in a line which has the
        same bearing at all points in the new coordinate system as in the first one.
        A transformation model which preserves direction automatically preserves
        linearity, shape and parallelism.

        @return A Boolean value. true if the model preserves the geometric
                transformation property, and false otherwise.

        @see PreserverParallelism()
        @see PreservesShape()
        @see PreservesLinearity()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool            PreservesDirection() const;

    /** -----------------------------------------------------------------------------
        This method returns true if the transformation model has a domain
        that limits its application.

        @return A Boolean value. true if the model has a limiting domain
                and false otherwise.

        @see GetDirectDomain()
        @see GetInverseDomain()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT bool            HasDomain() const;

    /** -----------------------------------------------------------------------------
        This method returns the direct domain of the transformation model.
        The method will return a shape representing the universe if there is no 
        domain. 

        Notice that no SetDomain() method is available as the domain expected
        to limit a transformation model is assumed to be as a result of the 
        limitations to the computations not from some desire to impose 
        arbitrarily limitations

        @return A HGF2DShape that defines the domain of the direct
                channel expressed in direct coordinates.

        @see HasDomain()
        @see GetInverseDomain()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT HFCPtr<HGF2DShape> GetDirectDomain() const;
    

    /** -----------------------------------------------------------------------------
        This method returns the inverse domain of the transformation model.
        The method will return a shape representing the universe if there is no 
        domain. 

        Notice that no SetDomain() method is available as the domain expected
        to limit a transformation model is assumed to be as a result of the 
        limitations to the computations not from some desire to impose 
        arbitrarily limitations

        @return A HGF2DShape that defines the domain of the direct
                channel expressed in direct coordinates.

        @see HasDomain()
        @see GetDirectDomain()
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT HFCPtr<HGF2DShape> GetInverseDomain() const;


    /** -----------------------------------------------------------------------------
    This method reverses the current transformation model. The operation of
    reversing implies that results produced by the ConvertDirect() method will
    from now on be produced by ConvertInverse() and vice-versa. The different
    parameters related to the model are modified. The units used for the direct
    and inverse output channels are also swapped.

    -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT void Reverse();

    /** -----------------------------------------------------------------------------
        Studies the reversibility of the model over a region using the given step.
        Since mzGCoord models are notably un-reversible when region of
        operation is far from usual region of application, it is recommended
        to estimate the reversibility of the model before using. The method
        will sample coordinate transformation by converting direct then inverse
        this result. The deviation from the original value is used in the
        calculation of mean and maximum error which are returned.
    
        @param pi_rPrecisionArea An extent over which to perform the study. The
                                 area may not be empty.
    
        @param pi_Step The step used in X and Y for sampling. This value must be
                       greater than 0.0
    
    
        @param po_pMeanError Pointer to double that receives the mean error.
    
        @param po_pMaxError  Pointer to double that receives the maximum error.
    
        @param po_pScaleChangeMean Pointer to double that receives the mean scale change
    
        @param po_pScaleChangeMax Pointer to double that receives the MAX scale change
    
        @param pi_ScaleThreshold Value indicating the scale that will result in a stop
                                 of study. Even if study is stopped, at least one
                                 sample has been completely proecessed and thus
                                 all stats are valid. The threshold is specified as
                                 a change of scale from 1.0 (no scale change). This parameter
                                 is optional and defaults to 1.0 meaning a factor of 2.0
                                 will stop the process
    
        -----------------------------------------------------------------------------
    */
    void StudyReversibilityPrecisionOver (const HGF2DLiteExtent& pi_PrecisionArea,
                                          double                pi_Step,
                                          double*               po_pMeanError,
                                          double*               po_pMaxError,
                                          double*               po_pScaleChangeMean,
                                          double*               po_pScaleChangeMax,
                                          double                pi_ScaleTreshold = 1.0) const;

    /** -----------------------------------------------------------------------------
        Extracts the mean and max error between two transformation model over a specific area.
    
        @param pi_rModel The other transformation model to check equivalence over area.
    
        @param pi_rPrecisionArea An extent over which to perform the study. The
                                 area may not be empty.
    
        @param pi_Step The step used in X and Y for sampling. This value must be
                       greater than 0.0
    
        @param pi_Direct Indicates if the transformation (and this the result) must be performed
                         in the direct direction or inverse direction. 
                         ATTENTION If Direct then the transformation is performed 
                         using ConvertDirect then the values of Max and Mean errors are in the 
                         Inverse Channel units. If false then conversion will be performed using
                         ConvertInverse and that the mena and max errors will be in the direct
                         channel units.
    
        @param po_pMeanError Pointer to double that receives the mean error. Note this error is
                             expressed in the converted domain.
    
        @param po_pMaxError  Pointer to double that receives the maximum error. Note that this
                             error is expressed in the converted domain.
    
        -----------------------------------------------------------------------------
    */
    IMAGEPP_EXPORT void GetEquivalenceToOver (const HGF2DTransfoModel& pi_rModel,
                               const HGF2DLiteExtent& pi_PrecisionArea,
                               double                pi_Step,
                               bool                  pi_Direct,
                               double*               po_pMeanError,
                               double*               po_pMaxError) const;
protected:
    // Primary methods
    IMAGEPP_EXPORT                   HGF2DTransfoModel();
    IMAGEPP_EXPORT                   HGF2DTransfoModel(const HGF2DTransfoModel& pi_rObj);
    IMAGEPP_EXPORT HGF2DTransfoModel&    operator=(const HGF2DTransfoModel& pi_rObj);

    virtual bool            _IsConvertDirectThreadSafe()  const = 0;
    virtual bool            _IsConvertInverseThreadSafe() const = 0;
    
    virtual StatusInt       _ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const = 0;
    virtual StatusInt       _ConvertDirect(double  pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_aXOut, double* po_aYOut) const;
    virtual StatusInt       _ConvertDirect(double  pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const;
    virtual StatusInt       _ConvertDirect(size_t  pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const;

    virtual StatusInt       _ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const = 0;
    virtual StatusInt       _ConvertInverse(double  pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_aXOut, double* po_aYOut) const;
    virtual StatusInt       _ConvertInverse(double  pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const; 
    virtual StatusInt       _ConvertInverse(size_t  pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const;

    virtual bool            _IsIdentity() const { return false; }

    virtual bool            _IsStretchable(double pi_AngleTolerance) const { return false; }
    virtual void            _GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const = 0;

    virtual bool                              _HasDomain() const { return false;}
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DShape> _GetDirectDomain() const;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DShape> _GetInverseDomain() const;

    virtual HGF2DTransfoModel*          _Clone() const = 0;

    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>   _ComposeYourself(const HGF2DTransfoModel& pi_rModel) const;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>   _ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const;
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DTransfoModel>   _ComposeInverseWithInverseOf(const HGF2DTransfoModel& pi_rModel) const;

    IMAGEPP_EXPORT virtual void                        _Reverse();

    virtual bool                        _CanBeRepresentedByAMatrix() const = 0;
    virtual HFCMatrix<3, 3>             _GetMatrix() const = 0;

    virtual HFCPtr<HGF2DTransfoModel>   _CreateSimplifiedModel() const = 0;

    virtual bool                        _PreservesLinearity() const = 0;
    virtual bool                        _PreservesParallelism() const = 0;
    virtual bool                        _PreservesShape() const = 0;
    virtual bool                        _PreservesDirection() const = 0;
    
    /** -----------------------------------------------------------------------------
        PROTECTED
        This protected method must be overloaded by transformation models to
        prepare acceleration attributes whenever the state of the object is
        changed. This method must be overloaded, and is called if units are changed
        by any of the unit specification methods
        -----------------------------------------------------------------------------
    */
    void Prepare() { _Prepare(); }
    virtual void _Prepare () = 0;

    //-----------------------------------------------------------------------------
    // CallComposeOf
    // This protected method permits to call the protected method of another
    // transfo model not necessarely of the same type.
    //----------------------------------------------------------------------------
    HFCPtr<HGF2DTransfoModel> CallComposeOf(const HGF2DTransfoModel& pi_rModel) const { return pi_rModel._ComposeYourself(*this); }

private:


    };

END_IMAGEPP_NAMESPACE
