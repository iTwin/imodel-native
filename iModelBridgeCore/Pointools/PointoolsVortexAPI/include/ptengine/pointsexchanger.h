#ifndef POINTOOLS_PCLOUD2_POINTS_EXCHANGER
#define POINTOOLS_PCLOUD2_POINTS_EXCHANGER

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <vector>
#include <string>
#include <map>
#include <ptcloud2\defs.h>
#include <ptfs\filepath.h>
#include <ptengine\ptengine_api.h>
#include <ptengine\pointsIOdefs.h>

namespace pointsengine
{
	class PTENGINE_API PointsExchanger
	{
	public:
		PointsExchanger();
		~PointsExchanger();

		bool initialize();

		ImportResult importFile(const ptds::FilePath &file);
		bool supportsImport(const ptds::FilePath &file);
		void importTypes(std::vector<ptds::FileType> &types);

		ExportResult exportFile(const ptds::FilePath &file);
		bool supportsExport(const ptds::FilePath &file);
		void exportTypes(std::vector<ptds::FileType> &types);

	private:
		typedef std::map <std::string, uint> IMPORTERS;
		typedef std::map <std::string, uint> EXPORTERS;
		IMPORTERS _importers;
		EXPORTERS _exporters;
	};
}

#endif
#endif