
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "3SMGL.h"
#include "CachedRenderer.h"

// ScalableMesh headers
#include <DgnPlatform/DgnPlatform.h>

#define VANCOUVER_API
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh/IScalableMesh.h>
#include <ScalableMesh/IScalableMeshNodeCreator.h>
#include <ScalableMesh/IScalableMeshQuery.h>
#include <ScalableMesh/ScalableMeshLib.h>
//-----------------------------------------------------------------------------
// DLL Entry Point
//-----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
//-----------------------------------------------------------------------------
uint64_t GetAmountOfPhysicalMemory()
{
	MEMORYSTATUSEX  memoryStatus;
	memoryStatus.dwLength = sizeof(memoryStatus);
	GlobalMemoryStatusEx(&memoryStatus);
	/*
	std::cout << " Memory % " << memoryStatus.dwMemoryLoad << std::endl;
	std::cout << " Memory Total (kb) :: " << memoryStatus.ullTotalPhys / 1024 << std::endl;
	std::cout << " Memory Available (kb) :: " << memoryStatus.ullAvailPhys / 1024 << std::endl;
	*/

	return memoryStatus.ullTotalPhys; // the total memory in bytes
									  //return memoryStatus.ullAvailPhys; // the available memory in bytes
}
//-----------------------------------------------------------------------------
uint64_t GetAmountOfAvaliableMemory()
{
	MEMORYSTATUSEX  memoryStatus;
	memoryStatus.dwLength = sizeof(memoryStatus);
	GlobalMemoryStatusEx(&memoryStatus);
	/*
	std::cout << " Memory % " << memoryStatus.dwMemoryLoad << std::endl;
	std::cout << " Memory Total (kb) :: " << memoryStatus.ullTotalPhys / 1024 << std::endl;
	std::cout << " Memory Available (kb) :: " << memoryStatus.ullAvailPhys / 1024 << std::endl;
	*/

	return memoryStatus.ullAvailPhys; // the total memory in bytes
									  //return memoryStatus.ullAvailPhys; // the available memory in bytes
}
//-----------------------------------------------------------------------------
// need this to retain a pointer to SM 
//-----------------------------------------------------------------------------
struct SM_Object
{
	ScalableMesh::IScalableMeshPtr ptr;
};
//-----------------------------------------------------------------------------
std::wstring str2wstr(const std::string& s)
//-----------------------------------------------------------------------------
{
	int len;
	int slength = (int)s.length() + 1;
	len = ::MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
//-----------------------------------------------------------------------------
struct SMHost : ScalableMesh::ScalableMeshLib::Host
{
	SMHost();
	ScalableMesh::ScalableMeshAdmin& _SupplyScalableMeshAdmin();
};
//-----------------------------------------------------------------------------
// Simple SMHost implementation - nothing more required for local use
//-----------------------------------------------------------------------------
SMHost *m_SMHost = 0;
SMHost::SMHost()
//-----------------------------------------------------------------------------
{}
//-----------------------------------------------------------------------------
ScalableMesh::ScalableMeshAdmin& SMHost::_SupplyScalableMeshAdmin()
//-----------------------------------------------------------------------------
{
	return *new ScalableMesh::ScalableMeshAdmin(); // delete will be hopefully called by ScalableMeshAdmin::_OnHostTermination
};

//-----------------------------------------------------------------------------
GL_ScalableMesh::GL_ScalableMesh(const char *filepath)
//-----------------------------------------------------------------------------
{
	// load the mesh
	static bool bInitialized = false;

	m_sm = 0;
	m_renderer = 0;

	if (!bInitialized)
	{
		m_SMHost = new SMHost();
		ScalableMesh::ScalableMeshLib::Initialize(*m_SMHost);
		bInitialized = true;

		size_t memory_usage = (size_t)(GetAmountOfPhysicalMemory() * 0.25); // in bytes, default is 30% of available memory
		size_t video_memory = 536870912 * 2; // in bytes, default is 512 Mbytes
		ScalableMesh::IScalableMeshMemoryCounts::SetMaximumMemoryUsage(memory_usage);
		ScalableMesh::IScalableMeshMemoryCounts::SetMaximumVideoMemoryUsage(video_memory);      // default video memory usage is 512 Mbytes
		ScalableMesh::IScalableMeshMemoryCounts::GetAmountOfUsedVideoMemory();                  // use this for framerate estimation
	}
	bInitialized = true;
	m_texUnit = -1;
	load3sm(filepath);

	strcpy_s(m_filename, sizeof(m_filename), filepath);
}
//-----------------------------------------------------------------------------
GL_ScalableMesh::~GL_ScalableMesh()
//-----------------------------------------------------------------------------
{
	CachedRenderer *renderer = reinterpret_cast<CachedRenderer*>(m_renderer);
	if (renderer)	delete renderer;

	SM_Object *smo = reinterpret_cast<SM_Object*>(m_sm);
	if (smo) delete smo;
	m_sm = 0;
}
//-----------------------------------------------------------------------------
bool GL_ScalableMesh::isValid() const
//-----------------------------------------------------------------------------
{
	return m_sm ? true : false;
}
//-----------------------------------------------------------------------------
void GL_ScalableMesh::getBounds(double & x, double & y, double & z, double & dx, double & dy, double & dz)
//-----------------------------------------------------------------------------
{
	x = m_dims[0];
	y = m_dims[1];
	z = m_dims[2];
	dx = m_dims[3]; 
	dy = m_dims[4];
	dz = m_dims[5];
}
//-----------------------------------------------------------------------------
void GL_ScalableMesh::getCenter(double & x, double & y, double & z)
//-----------------------------------------------------------------------------
{
	x = m_dims[0] + m_dims[3] * 0.5;
	y = m_dims[1] + m_dims[4] * 0.5;
	z = m_dims[2] + m_dims[5] * 0.5;
}
//-----------------------------------------------------------------------------
void GL_ScalableMesh::setUniformTexID( unsigned int id )
//-----------------------------------------------------------------------------
{
	m_texUnit = id;
}
//-----------------------------------------------------------------------------
float GL_ScalableMesh::getAverageDisplayLod() const
//-----------------------------------------------------------------------------
{
	CachedRenderer *renderer = reinterpret_cast<CachedRenderer*>(m_renderer);

	if (renderer)
	{
		return renderer->averageLevel();
	}
	else
	{
		return 0.0f;
	}
}
//-----------------------------------------------------------------------------
void GL_ScalableMesh::setWorldOffset(double x, double y, double z)
//-----------------------------------------------------------------------------
{
	DPoint3d cen;
	cen.Init(x, y, z);
	CoordinateSystem::instance().setWorldCenter(cen);
}
//-----------------------------------------------------------------------------
void GL_ScalableMesh::getWorldOffset(double &x, double &y, double &z)
//-----------------------------------------------------------------------------
{
	DPoint3d cen(CoordinateSystem::instance().worldCenter());
	x = cen.x;
	y = cen.y;
	z = cen.z;
}
//-----------------------------------------------------------------------------
SMPointer GL_ScalableMesh::load3sm(const std::string & filepath)
//-----------------------------------------------------------------------------
{
	// attempt to open the file
	StatusInt status;
	
	std::wstring filepath_w(str2wstr(filepath));

	if (0 != _waccess(filepath_w.c_str(), 04))
	{
		status = BSISUCCESS;
		return 0; // File not found
	}

	SM_Object *smo = new SM_Object;
	smo->ptr = ScalableMesh::IScalableMesh::GetFor(filepath_w.c_str(), true, true, status);

	m_sm = reinterpret_cast<void*>(smo);

	if (smo->ptr == nullptr)
	{
		std::cout << " !! Cannot read file : \t" << filepath << std::endl;
		std::cout << " !! Status return value : \t" << status << std::endl;
		delete smo;
		m_sm = 0;
	}
	else
	{
		DRange3d range;
		smo->ptr->GetRange(range);
		m_dims[0] = range.low.x;
		m_dims[1] = range.low.y;
		m_dims[2] = range.low.z;
		m_dims[3] = range.XLength();
		m_dims[4] = range.YLength();
		m_dims[5] = range.ZLength();

		DPoint3d center = DPoint3d::From(range.low.x + range.XLength() / 2, 
			range.low.y + range.YLength() / 2, 
			range.low.z + range.ZLength() / 2);

		//	this is responsibility of the client
		//CoordinateSystem::instance().setWorldCenter(center);
	}

	return m_sm;
}
//-----------------------------------------------------------------------------
bool GL_ScalableMesh::draw(GL_Camera & camera, bool faces, bool texture, bool wire, bool boxes)
//-----------------------------------------------------------------------------
{
	if (!m_sm) return false;

	if (!m_renderer)
	{
		m_renderer = new CachedRenderer;
	}
	
	SM_Object *smo = reinterpret_cast<SM_Object*>(m_sm);
	ScalableMesh::IScalableMeshPtr sm = smo->ptr;
	CachedRenderer *renderer = reinterpret_cast<CachedRenderer*>(m_renderer);
	
	Camera smcamera;
	smcamera.m_farPlane = camera.far_plane;
	smcamera.m_fov = camera.fov;
	smcamera.m_nearPlane = camera.near_plane;
	smcamera.m_position.x = camera.position[0];
	smcamera.m_position.y = camera.position[1];
	smcamera.m_position.z = camera.position[2];
	smcamera.m_target.x = camera.target[0];
	smcamera.m_target.y = camera.target[1];
	smcamera.m_target.z = camera.target[2];
	
	renderer->m_bDrawBox = boxes;
	renderer->m_bDrawFaces = faces;
	renderer->m_bTextured= texture;
	renderer->m_bWireframe= wire;

	return renderer->drawScalableMesh(smo->ptr, &smcamera);
}
