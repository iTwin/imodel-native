#include "PointoolsVortexAPIInternal.h"
#include <pt\os.h>
#include <ptengine\pointsexchanger.h>

using namespace pointsengine;

using namespace ptds;

//--------------------------------------------------------------
// plugins manager
//--------------------------------------------------------------
#define NUMSYM 4
typedef  ImportResult (*IMPORTFUNC)(const ptds::FilePath &);
typedef  ExportResult (*EXPORTFUNC)(const ptds::FilePath &);

typedef  int (*SUPPORTEDEXTFUNC)(std::vector<ptds::FileType> &);

#define SYM1			"supportedImports"
#define SYM1MARGS       "RSt6vectorISsSaISsEE"
#define FUNC1			SUPPORTEDEXTFUNC		
#define FUNC1IDENTIFIER	int supportedImports
#define FUNC1ARGSSPEC	std::vector<ptds::FileType> &ftype
#define FUNC1ARGS		ftype

#define SYM2			"supportedExports"
#define SYM2MARGS       "RSt6vectorISsSaISsEE"
#define FUNC2			SUPPORTEDEXTFUNC		
#define FUNC2IDENTIFIER	int supportedExports
#define FUNC2ARGSSPEC	std::vector<ptds::FileType> &ftype
#define FUNC2ARGS		ftype

#define SYM3			"importFile"
#define SYM3MARGS       "RKSs"
#define FUNC3			IMPORTFUNC		
#define FUNC3IDENTIFIER	ImportResult importFile
#define FUNC3ARGSSPEC	const ptds::FilePath &path
#define FUNC3ARGS		path

#define SYM4			"exportFile"
#define SYM4MARGS       "RKSs"
#define FUNC4			EXPORTFUNC		
#define FUNC4IDENTIFIER	ExportResult exportFile
#define FUNC4ARGSSPEC	const ptds::FilePath &path
#define FUNC4ARGS		path

//#define COUT_TRACE
#include <pt\trace.h>
#include <ptapp\pluginsmanager.h>

using namespace pt;
static ptapp::PluginsManager s_plugins("getPointsIOPluginName");

#undef HANDLERFUNC
#undef SUPPORTEDEXTFUNC

//--------------------------------------------------------------
// End of Plugins Manager
//--------------------------------------------------------------
// PointsExhanger constructor 
// -	loads plugins
// -	stores which plugins import which extensions
//--------------------------------------------------------------
PointsExchanger::PointsExchanger()
{
	PTTRACE("PointsExchanger::PointsExchanger");
}
bool PointsExchanger::initialize()
{
	PTTRACE("PointsExchanger::initialize");

	if (!s_plugins.size())
	{
		int count = s_plugins.loadPlugins(_T("io"),_T("io"));
		PTTRACEOUT << " PointsExchange io plugins loaded";
		/*load import/export maps*/ 
		std::vector<FileType> exi;
		std::vector<FileType> exe;
		
		int i;
		for (i=0; i<count; i++)
		{
			s_plugins[i]->supportedImports(exi);
			s_plugins[i]->supportedExports(exe);
		}
		int j;
		for (j=0; j<exi.size(); j++)
		{
			PTTRACEOUT << "Points Exchanger : adding " << exi[j].extension << " import handler ";		
			_importers.insert(IMPORTERS::value_type(std::string(exi[j].extension), i));
		}
		for (j=0; j<exe.size(); j++)
			_exporters.insert(EXPORTERS::value_type(std::string(exe[j].extension), i));
		
		return true;
	}
	return false;
}
//--------------------------------------------------------------
// PointsExhanger destructor - 
//--------------------------------------------------------------
PointsExchanger::~PointsExchanger()
{
	PTTRACE("PointsExchanger::~PointsExchanger");	
}
//--------------------------------------------------------------
// Supports import of file (by extenstion)
//--------------------------------------------------------------
bool PointsExchanger::supportsImport(const FilePath &path)
{
	if (path.extension())
	{
		IMPORTERS::iterator i = _importers.find(std::string(Unicode2Ascii::convert(path.extension()).c_str()));
		if (i!=_importers.end()) return true;
	}
	return false;	
}
//--------------------------------------------------------------
// Supports export of file (by extenstion)
//--------------------------------------------------------------
bool PointsExchanger::supportsExport(const FilePath &path)
{
	if (path.extension())
	{
		EXPORTERS::iterator i = _exporters.find(std::string(Unicode2Ascii::convert(path.extension())));
		if (i!=_exporters.end()) return true;
	}
	return false;	
}
//--------------------------------------------------------------
// Import File
//--------------------------------------------------------------
ImportResult PointsExchanger::importFile(const FilePath &path)
{
	PTTRACE("PointsExchange::import");	
	
	if (path.extension())
	{
		PTTRACEOUT << "Points Exchanger importing ." << path.extension() << " file";
		PTTRACEOUT << "Searching " << s_plugins.size() << " plugins";

		for (unsigned int i=0;i<s_plugins.size(); i++)
		{
			ImportResult res = s_plugins[i]->importFile(path);
			if (res != ImportResult_NoHandler) return res;
		}
	}
	PTTRACEOUT << "Points Exchanger Plugin not found";
	return ImportResult_NoHandler;
}
//--------------------------------------------------------------
// Export File
//--------------------------------------------------------------
ExportResult PointsExchanger::exportFile(const FilePath &path)
{
	PTTRACE("PointsExchange::export");	
	
	if (path.extension())
	{
		for (unsigned int i=0;i<s_plugins.size(); i++)
		{
			ExportResult res = s_plugins[i]->exportFile(path);
			if (res != ExportResult_NoHandler) return res;
		}
	}
	return ExportResult_NoHandler;
}
//--------------------------------------------------------------
// import Types
//--------------------------------------------------------------
void PointsExchanger::importTypes(std::vector<ptds::FileType> &ftypes)
{
	for (uint i=0; i<s_plugins.size(); i++)
	{
		std::vector<FileType> pitypes;

		s_plugins[i]->supportedImports(pitypes);

		for (uint j=0; j<pitypes.size(); j++)
		{
			ftypes.push_back(pitypes[j]);
		}
	}
}
//--------------------------------------------------------------
// export Types
//--------------------------------------------------------------
void PointsExchanger::exportTypes(std::vector<ptds::FileType> &ftypes)
{
	for (uint i=0; i<s_plugins.size(); i++)
	{
		std::vector<FileType> pitypes;

		s_plugins[i]->supportedExports(pitypes);

		for (uint j=0; j<pitypes.size(); j++)
		{
			ftypes.push_back(pitypes[j]);
		}
	}
}