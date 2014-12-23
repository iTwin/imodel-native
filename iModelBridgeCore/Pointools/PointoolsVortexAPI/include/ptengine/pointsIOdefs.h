#ifndef POINTOOLS_PCLOUD2_EXCHANGE_DEFINITIONS
#define POINTOOLS_PCLOUD2_EXCHANGE_DEFINITIONS

namespace pointsengine
{
enum ImportResult
{
	ImportResult_Success,
	ImportResult_BadPath,
	ImportResult_NoAccess,
	ImportResult_NoHandler,
	ImportResult_BadFile
};
enum ExportResult
{
	ExportResult_Success,
	ExportResult_BadPath,
	ExportResult_NoAccess,
	ExportResult_NoHandler
};
}
#endif