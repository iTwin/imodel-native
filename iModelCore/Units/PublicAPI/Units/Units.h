
#pragma once

struct UnitRegistry;

struct Unit
	{
friend struct UnitRegistry;

private:
	bvector<Utf8String> m_numerator;
	bvector<Utf8String> m_denominator;

	Unit(Utf8CP system, Utf8CP phenomena, Utf8CP id, Utf8CP definition) { }

protected:

	bvector<Utf8String> Numerator() const { return m_numerator; }
	bvector<Utf8String> Denominator() const { return m_denominator; }

	//virtual Utf8CP _GetName() const;
	//virtual Utf8CP _GetDisplayLabel() const;
	
public:
	virtual ~Unit() { }

	bool IsValid() { return true; }

	//Utf8CP GetName() const { return _GetName(); }
	//Utf8CP GetDisplayLabel() const { return _GetDisplayLabel(); }

	// Overload operators.
	//virtual Unit& operator*(const Unit& rhs) const = 0;
	};

struct QuantityBase
	{
private:
	double   m_quantity;
	Unit  *  m_unit;

	QuantityBase() { }

public:
	// Lookup the unit type using a string (from UnitRegistry) and
	// return the unit or null.
	static QuantityBase& Create(double value, Utf8CP unit);

	/// Overloaded Operators
	};

struct Quantity : QuantityBase
	{
	
	};

struct Constant : QuantityBase
	{

	};

struct UnitRegistry
	{
private:
	static UnitRegistry * s_instance;

	bvector<Utf8String> m_systems;
	bvector<Utf8String> m_phenomena;
	bmap<Utf8String, Unit> m_baseUnits; // Indexed by Id.
	bmap<Utf8String, Unit> m_units; // Indexed by Id.
	bvector<Constant> m_constants;
	bmap<bpair<Utf8String, Utf8String>, double> m_conversions;

	UnitRegistry();
	UnitRegistry(const UnitRegistry& rhs) = delete;
	UnitRegistry & operator= (const UnitRegistry& rhs) = delete;

	void AddGlobalSystems();
	void AddGlobalPhenomena();
	void AddGlobalConstants();

	void AddSystem(Utf8CP systemName);
	void AddPhenomena(Utf8CP phenomenaName);

public:
	static UnitRegistry & Instance();

	// Register methods.
	UNITS_EXPORT void AddUnit(Utf8CP name, int phenemona, int system, Utf8CP expression, double factor, double offset);
	UNITS_EXPORT void AddConstant(double magnitude, Utf8CP unitName);

	// Lookup methods
	UNITS_EXPORT Unit *     LookupUnit(Utf8CP name);
	UNITS_EXPORT Constant * LookupConstant(Utf8CP name);
		
	// bool Exists methods.
	// Probably some query methods. (Find base for phenomena and system probably).
	};