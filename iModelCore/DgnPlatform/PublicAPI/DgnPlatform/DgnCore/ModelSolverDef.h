/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/ModelSolverDef.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//========================================================================================
//! Specifies the solver to invoke when changes to a model or its contents are validated.
//! @see DgnScript::ExecuteModelSolver for information on script type model solvers.
//=======================================================================================
struct ModelSolverDef
    {
    friend struct DgnModel;
    friend struct CreateParams;

    //========================================================================================
    //! Identifies the type of solver used by a model
    //========================================================================================
    enum class Type 
        {
        None=0,     //!< This model has no solver
        Script,     //!< Execute a named script function. See DgnScript::ExecuteModelSolver
        // *** TBD: Add built-in constraint solvers 
        };

    //========================================================================================
    //! A solver parameter
    //========================================================================================
    struct Parameter
        {
        //! The variability scope of the parameter
        enum class Scope
            {
            Class=0,        //!< fixed for all types and instances
            Type=1,         //!< fixed for all instances of a given type
            Instance=2      //!< can vary per instance
            };
        private:
        Scope   m_scope;
        Utf8String m_name;
        ECN::ECValue m_value;
        friend struct ModelSolverDef;
        //! Serialize this Parameter in JSON format
        Json::Value ToJson() const;
        //! Deserialize a Parameter from a stored JSON object
        explicit Parameter(Json::Value const&);
        public:
        //! Construct a new Parameter
        DGNPLATFORM_EXPORT Parameter(Utf8CP n, Scope s, ECN::ECValueCR v);
        //! Get the scope of this parameter
        Scope GetScope() const {return m_scope;}
        //! Get the name of this parameter
        Utf8StringCR GetName() const {return m_name;}
        //! Get the value of this parameter
        DGNPLATFORM_EXPORT ECN::ECValueCR GetValue() const;
        //! Set the value of this parameter
        DGNPLATFORM_EXPORT DgnDbStatus SetValue(ECN::ECValueCR newValue);
        };

    //========================================================================================
    //! A parameter set
    //========================================================================================
    struct ParameterSet
        {
        private:
        bvector<Parameter> m_parameters;
        public:
        ParameterSet() {;}
        DGNPLATFORM_EXPORT explicit ParameterSet(Json::Value const&);
        explicit ParameterSet(bvector<Parameter> const& p) : m_parameters(p) {;}

        DGNPLATFORM_EXPORT Json::Value ToJson() const;

        //! Get a parameter by name
        DGNPLATFORM_EXPORT Parameter const* GetParameter(Utf8StringCR pname) const;

        //! Get a parameter by name
        DGNPLATFORM_EXPORT Parameter* GetParameterP(Utf8StringCR pname);

        //! Convert this parameter set to a string that can be used as a key in the ComponentSolution table.
        Utf8String ComputeSolutionName() const;

        //! Set parameter values
        DgnDbStatus SetValuesFromECProperties(ECN::IECInstanceCR instance);

        //! Set parameter values
        DgnDbStatus SetValues(ParameterSet const& parmsIn);

    
        //! Convert to Json
        BentleyStatus ConvertValuesToJson(Json::Value& json) const;

        bvector<Parameter>::const_iterator begin() const {return m_parameters.begin();}
        bvector<Parameter>::const_iterator end() const {return m_parameters.end();}

        bvector<Parameter>::iterator begin() {return m_parameters.begin();}
        bvector<Parameter>::iterator end() {return m_parameters.end();}
        };

    private:
    Type        m_type;
    Utf8String  m_name;
    Utf8String  m_version;
    ParameterSet m_parameters;

    void FromJson(Utf8CP);
    Utf8String ToJson() const;

    void Solve(DgnModelR);

    DGNPLATFORM_EXPORT void RelocateToDestinationDb(DgnImportContext&);

    public:

    //! @private
    DGNPLATFORM_EXPORT ModelSolverDef();

    //! Construct a ModelSolverDef specification, in preparation for creating a new DgnModel. 
    //! @see DgnScriptLibrary
    //! @param type         The solver type
    //! @param identifier   Identifies the solver. The meaning of this identifier varies, depending on the type of the solver.
    //! @param parameters   The parameters to be passed to the solver
    DGNPLATFORM_EXPORT ModelSolverDef(Type type, Utf8CP identifier, bvector<Parameter> const& parameters);

    //! Test if this object specifies a solver
    bool IsValid() const {return Type::None != GetType();}
    //! Get the type of the solver
    Type GetType() const {return m_type;}
    //! Get the identifier of the solver
    Utf8StringCR GetName() const {return m_name;}
    //! Get the parameters of the solver
    DGNPLATFORM_EXPORT ParameterSet const& GetParameters() const;
    //! Get the parameters of the solver.
    DGNPLATFORM_EXPORT ParameterSet& GetParametersR();
    //! Get the solver version
    Utf8String GetVersion() {return m_version;}
    //! Set the solver version
    void SetVersion(Utf8StringCR v) {m_version=v;}
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
