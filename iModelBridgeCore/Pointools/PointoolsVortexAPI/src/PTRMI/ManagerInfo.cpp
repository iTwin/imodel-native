#include "PointoolsVortexAPIInternal.h"

#include <PTRMI/ManagerInfo.h>
#include <PTRMI/Manager.h>
#include <PTRMI/Message.h>

#include <ptapi/PointoolsVortexAPI.h>

namespace PTRMI
{

ManagerVariantGraphStatics(ManagerInfoVariantGraph)



ManagerInfo::ManagerInfo(void)
{

}


ManagerInfo::ManagerInfo(Manager &manager)
{
	update(manager);
}


void ManagerInfo::read(DataBuffer &buffer)
{
	Super::read(buffer);
}


void ManagerInfo::write(DataBuffer &buffer) const
{
	Super::write(buffer);
}


Status ManagerInfo::readFile(const wchar_t *file)
{
	if(Super::readFile(file))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed);
}


Status ManagerInfo::writeFile(const wchar_t *file) const
{
	if(Super::writeFile(file))
	{
		return Status();
	}

	return Status(Status::Status_Error_Failed);
}


void ManagerInfo::update(Manager &manager, unsigned int verboseLevel)
{
	PTubyte									version[4];

	ManagerInfoVariantGraph::KeyPath		path;
	const PTRMI::GUID					&	managerGUID = getManager().getManagerGUID();

															// Clear any previous data
	deleteAll();

	ptGetVersionNum(version);
															// Create Manager group path
	getManagerPath(path);
															// Add this manager's GUID
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_GUID), ManagerInfoVariantGraph::VGVariantGUID(managerGUID));
															// Add Vortex API's version number
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V1), ManagerInfoVariantGraph::VGVariantUnsignedChar(version[0]));
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V2), ManagerInfoVariantGraph::VGVariantUnsignedChar(version[1]));
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V3), ManagerInfoVariantGraph::VGVariantUnsignedChar(version[2]));
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V4), ManagerInfoVariantGraph::VGVariantUnsignedChar(version[3]));
															// Add this manager's PTRMI Message version
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_MESSAGE_VERSION), ManagerInfoVariantGraph::ManagerInfoVariantMessageVersion(PTRMI_CURRENT_MESSAGE_HEADER_VERSION));

															// Add Manager/Features Group
	getManagerFeaturesPath(path);
															// Add MultiReadSet as a stated feature
	add(path, std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_FEATURES_MULTI_READ_SET), ManagerInfoVariantGraph::VGVariantBool(true));


	getManagerStatusClientInterfacesPath(path);
															// Include status if verboseness is level 1 or higher
	if(verboseLevel >= 1)
	{
		manager.getObjectManager().getInfo(*this);
	}
}


void ManagerInfo::getManagerPath(ManagerInfo::KeyPath &path) const
{
	path.clear();
	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_GROUP));
}


void ManagerInfo::getManagerFeaturesPath(ManagerInfo::KeyPath &path) const
{
	getManagerPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_FEATURES_GROUP));
}


void ManagerInfo::getManagerStatusPath(ManagerInfo::KeyPath &path) const
{
	getManagerPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_GROUP));
}


void ManagerInfo::getManagerStatusClientInterfacesPath(ManagerInfo::KeyPath &path) const
{
	getManagerStatusPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_CLIENT_INTERFACES_GROUP));
}


void ManagerInfo::getManagerStatusServerInterfacesPath(ManagerInfo::KeyPath &path) const
{
	getManagerStatusPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_SERVER_INTERFACES_GROUP));
}


void ManagerInfo::getManagerStatusObjectsPath(ManagerInfo::KeyPath &path) const
{
	getManagerStatusPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_STATUS_OBJECTS_GROUP));
}



PTRMI::GUID ManagerInfo::getManagerGUID() const
{
	PTRMI::GUID				managerGUID;
	ManagerInfo::KeyPath	path;

	getManagerPath(path);
	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_GUID));

	get<PTRMI::GUID>(path, managerGUID);

	return managerGUID;
}

ManagerInfo::MessageVersion ManagerInfo::getMessageVersion(void) const
{
	MessageVersion			messageVersion;
	ManagerInfo::KeyPath	path;

	getManagerPath(path);
	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MESSAGE_VERSION));

	if(get<ManagerInfoVariantGraph::ManagerInfoVariantMessageVersion::Type>(path, messageVersion) == false)
	{
		messageVersion = Message::getDefaultMessageVersion();
	}

	return messageVersion;
}


bool ManagerInfo::getVortexAPIVersion(unsigned char *version) const
{
	if(version)
	{
		ManagerInfo::KeyPath	path;

		getManagerPath(path);
		path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V1));
		if(get<ManagerInfoVariantGraph::VGVariantUnsignedChar::Type>(path, version[0]) == false)
		{
			return false;
		}

		getManagerPath(path);
		path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V2));
		if(get<ManagerInfoVariantGraph::VGVariantUnsignedChar::Type>(path, version[1]) == false)
		{
			return false;
		}

		getManagerPath(path);
		path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V3));
		if(get<ManagerInfoVariantGraph::VGVariantUnsignedChar::Type>(path, version[2]) == false)
		{
			return false;
		}

		getManagerPath(path);
		path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_VORTEX_API_VERSION_V4));
		if(get<ManagerInfoVariantGraph::VGVariantUnsignedChar::Type>(path, version[3]) == false)
		{
			return false;
		}

		return true;
	}

	return false;
}


bool ManagerInfo::isAtLeastVersion(unsigned char *requiredVersionMin) const
{
	unsigned char	version[4];

	if(requiredVersionMin && getVortexAPIVersion(version))
	{
		return (version[0] >= requiredVersionMin[0] &&
		        version[1] >= requiredVersionMin[1] &&
		        version[2] >= requiredVersionMin[2] &&
		        version[3] >= requiredVersionMin[3]);
	}


	return false;
}


bool ManagerInfo::getFeatureMultiReadSet(void) const
{
	ManagerInfo::KeyPath	path;
	bool					supported;

	getManagerFeaturesPath(path);

	path.push_back(std::wstring(MANAGER_INFO_VARIANT_KEY_MANAGER_FEATURES_MULTI_READ_SET));
	if(get<ManagerInfoVariantGraph::VGVariantBool::Type>(path, supported) == false)
	{
															// Handle version 1.6.0.1 that supports MultiReadSet but does not have ManagerInfo Feature set
		unsigned char minVersion[4] = {1, 6, 0, 1};

		return isAtLeastVersion(minVersion);
	}

	return supported;
}





} // End PTRMI namespace