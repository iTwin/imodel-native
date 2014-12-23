
#pragma once

#include <PTRMI/Message.h>
#include <PTRMI/ManagerInfoVariantGraph.h>

#include <map>

#define MANAGER_INFO_VARIANT_KEY_MANAGER_GROUP								L"Manager"
#define MANAGER_INFO_VARIANT_KEY_GUID										L"ManagerGUID"
#define MANAGER_INFO_VARIANT_KEY_MESSAGE_VERSION							L"MessageVersion"
#define MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V1						L"VortexVersionV1"
#define MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V2						L"VortexVersionV2"
#define MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V3						L"VortexVersionV3"
#define MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V4						L"VortexVersionV4"

#define MANAGER_INFO_VARIANT_KEY_MANAGER_FEATURES_GROUP						L"Features"
#define MANAGER_INFO_VARIANT_KEY_MANAGER_FEATURES_MULTI_READ_SET			L"MultiReadSet"

#define MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_GROUP						L"Status"
#define MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_CLIENT_INTERFACES_GROUP		L"ClientInterfaces"
#define MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_SERVER_INTERFACES_GROUP		L"ServerInterfaces"

#define MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_OBJECTS_GROUP				L"Objects"

namespace PTRMI
{

class DataBuffer;
class Manager;


class ManagerInfo : public ManagerInfoVariantGraph
{

public:

	typedef ManagerInfoVariantGraph	Super;

	typedef Message::MessageVersion		MessageVersion;

	typedef Super::KeyPath				KeyPath;

protected:

	bool								isAtLeastVersion						(unsigned char *requiredVersionMin) const;

public:
										ManagerInfo								(void);
										ManagerInfo								(Manager &manager);

	void								update									(Manager &manager, unsigned int verboseLevel = 0);

	void								read									(DataBuffer &buffer);
	void								write									(DataBuffer &buffer) const;

	Status								readFile								(const wchar_t *file);
	Status								writeFile								(const wchar_t *file) const;

	void								getManagerPath							(ManagerInfo::KeyPath &path) const;
	void								getManagerFeaturesPath					(ManagerInfo::KeyPath &path) const;
	void								getManagerStatusPath					(ManagerInfo::KeyPath &path) const;
	void								getManagerStatusClientInterfacesPath	(ManagerInfo::KeyPath &path) const;
	void								getManagerStatusServerInterfacesPath	(ManagerInfo::KeyPath &path) const;
	void								getManagerStatusObjectsPath				(ManagerInfo::KeyPath &path) const;

	PTRMI::GUID							getManagerGUID							(void) const;
	MessageVersion						getMessageVersion						(void) const;
	bool								getVortexAPIVersion						(unsigned char *version) const;
	bool								getFeatureMultiReadSet					(void) const;
};


} // End PTRMI
