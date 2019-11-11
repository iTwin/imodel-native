#include <ORDBridgeInternal.h>

BEGIN_ORDBRIDGE_NAMESPACE
USING_NAMESPACE_DGNDBSYNC_DGNV8
USING_NAMESPACE_BENTLEY_OBMNET_GEOMETRYMODEL_SDK

void ConvertOBMElementXDomain::_DetermineElementParams(DgnClassId& classId, DgnCode& code, DgnCategoryId& categoryId, DgnV8EhCR v8el, DgnDbSync::DgnV8::Converter& converter, ECObjectsV8::IECInstance const* primaryV8Instance, DgnDbSync::DgnV8::ResolvedModelMapping const& v8mm)
{

	//first attempt at detecting OBM civil data
	/*DgnV8Api::FindInstancesScopePtr scope = DgnV8Api::FindInstancesScope::CreateScope(v8el, DgnV8Api::FindInstancesScopeOption());
	ECQueryPtr selectAllV8ECQuery = DgnV8Api::ECQuery::CreateQuery(DgnV8Api::ECQUERY_PROCESS_SearchAllExtrinsic);
	for (DgnV8Api::DgnECInstance* v8Instance : DgnV8Api::DgnECManager::GetManager().FindInstances(*scope, *selectAllV8ECQuery))
	{
		ECClassName v8Class(v8Instance->GetClass());
		Utf8String v8SchemaName(v8Class.GetSchemaName());
		Utf8String v8ClassName(v8Class.GetClassName());
		bool hasBridge = false;
		if (v8SchemaName.Contains("Bentley_Civil__Model_Geometry"))
		{
			hasBridge = true;
			classId = converter.GetDgnDb().Schemas().GetClassId("BridgeStructuralPhysical", "GenericSubstructureElement");
		}
		if (v8ClassName.Contains("Column"))
		{
			classId = converter.GetDgnDb().Schemas().GetClassId("BridgeStructuralPhysical", "GenericSubstructureElement");
		}
		Bentley::ObmNET::BridgePtr bridgePtr = Bentley::ObmNET::GeometryModel::SDK::Bridge::CreateFromElementHandle(v8el);
		if (!bridgePtr.IsValid())
		{
			hasBridge = false;
		}
		else
		{
			hasBridge = true;
			m_bridgeV8RefSet.insert(v8el.GetElementRef());
			//GetGeometryOwner<SolidEntity>(pierColPtr);
		}
	}*/

	Bentley::ObmNET::BridgePtr bridgePtr = Bentley::ObmNET::GeometryModel::SDK::Bridge::CreateFromElementHandle(v8el);
	if (bridgePtr.IsValid())
	{
		m_bridgeV8RefSet.insert(v8el.GetElementRef());
		//GetGeometryOwner<SolidEntity>(pierColPtr);
	}
}

Dgn::DgnDbSync::DgnV8::XDomain::Result ConvertOBMElementXDomain::_PreConvertElement(DgnV8EhCR v8el, Dgn::DgnDbSync::DgnV8::Converter &, Dgn::DgnDbSync::DgnV8::ResolvedModelMapping const &)
{
	if (m_elementsSeen.end() != m_elementsSeen.find(v8el.GetElementRef()))
		return Result::Proceed;

	m_elementsSeen.insert(v8el.GetElementRef());
	Bentley::ObmNET::BridgePtr bridgePtr = Bentley::ObmNET::GeometryModel::SDK::Bridge::CreateFromElementHandle(v8el);
	if (bridgePtr.IsValid())
	{
		m_converter.m_bridgeV8RefSet.insert(v8el.GetElementRef());
	}
	return Result::Proceed;
}

ConvertOBMElementXDomain::ConvertOBMElementXDomain(ORDConverter & converter) :m_converter(converter)
{
	m_graphic3dClassId = converter.GetDgnDb().Schemas().GetClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASS_Graphic3d);
}

void ConvertOBMElementXDomain::CreateDgnBridges()
{
	bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimIDMap = m_converter.GetCifAlignmentToBimIDMap();
	for (auto bridgeV8ref : m_bridgeV8RefSet)
	{
		ElementHandle bridgeEh(bridgeV8ref, NULL);
		Bentley::ObmNET::BridgePtr bridgePtr = Bentley::ObmNET::GeometryModel::SDK::Bridge::CreateFromElementHandle(bridgeEh);
		CreateDgnBridge(bridgePtr, cifAlignmentToBimIDMap);
	}
}

void ConvertOBMElementXDomain::CreateDgnBridge(Bentley::ObmNET::BridgePtr bridgePtr, bmap<Bentley::ElementRefP, Dgn::DgnElementId> cifAlignmentToBimID)
{
	Bentley::ObmNET::AlignmentsPtr bridgeAlignmentsPtr = bridgePtr->GetAlignments();
	if (bridgeAlignmentsPtr != nullptr)
	{
		Bentley::Cif::AlignmentPtr bridgeAlignmentPtr = bridgeAlignmentsPtr->GetBridgeAlignment();
		if (bridgeAlignmentPtr != nullptr)
		{
			auto cifAlignmentRef = bridgeAlignmentPtr->GetElementHandle()->GetElementRef();
			auto cifIter = cifAlignmentToBimID.find(cifAlignmentRef);
			if (cifIter != cifAlignmentToBimID.end())
			{
				Dgn::DgnElementId bimAlignmentId = cifIter->second;
				RoadRailAlignment::AlignmentCPtr bimAlignmentCPtr = RoadRailAlignment::Alignment::Get(m_converter.GetDgnDb(), bimAlignmentId);
			}
		}
	}
}

Bentley::ECN::ECRelationshipClassCP ConvertOBMElementXDomain::FindRelationshipClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className)
{
	Bentley::ECN::ECClassCP ecClass = FindClass(dgnFile, schemaName, className);
	if (nullptr == ecClass)
	{
		return nullptr;
	}
	return dynamic_cast<Bentley::ECN::ECRelationshipClassCP>(ecClass);
}

Bentley::ECN::ECClassCP ConvertOBMElementXDomain::FindClass(DgnFileP dgnFile, WCharCP schemaName, WCharCP className)
{
	auto& mgr = DgnPlatform::DgnECManager::GetManager();
	DgnPlatform::SchemaInfo schemaInfo(Bentley::ECN::SchemaKey(schemaName, (uint32_t)CIF_MAJOR_SCHEMA_VERSION_NO, (uint32_t)CIF_MINOR_SCHEMA_VERSION_NO), *dgnFile);
	Bentley::ECN::ECSchemaPtr schema = mgr.LocateSchemaInDgnFile(schemaInfo, Bentley::ECN::SCHEMAMATCHTYPE_Latest);
	return schema->GetClassCP(className);
}

DgnPlatform::DgnECRelationshipIterable ConvertOBMElementXDomain::FindRelationships(DgnECInstanceCR instance, Bentley::ECN::ECRelationshipClassCP relClass, Bentley::ECN::ECRelatedInstanceDirection dir)
{
	auto& mgr = DgnPlatform::DgnECManager::GetManager();
	DgnPlatform::QueryRelatedClassSpecifierPtr classSpec = DgnPlatform::QueryRelatedClassSpecifier::Create(*relClass, nullptr, dir);
	return mgr.FindRelationships(instance, *classSpec, Bentley::Cif::Dgn::CifDgnECManager::GetQualifiedPepCreateContext());
}

template<class T> 
RefCountedPtr<T> ConvertOBMElementXDomain::GetGeometryOwner(Bentley::ObmNET::PierColumnPtr pierColPtr)
{
	RefCountedPtr<Bentley::Cif::SDK::ObjectCollection<T>> col = Bentley::Cif::SDK::ObjectCollection<T>::Create(*pierColPtr, "RelBMSolidSolid", BMSOLID_CLASSNAME, ECN::STRENGTHDIRECTION_Forward);
	return col;
}

END_ORDBRIDGE_NAMESPACE