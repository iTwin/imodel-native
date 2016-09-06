#pragma once
#define LOG_SET_PATH( __path) Utf8String __log_path_str = __path;
#define LOG_SET_PATH_W( __path) WString __log_path_wstr = L ## __path;

#define LOG_PATH_STR __log_path_str
#define LOG_PATH_STR_W __log_path_wstr

#define LOGSTRING_NODE_INFO(node, strname) { \
                        strname ## .append(std::to_string(node ## ->GetBlockID().m_integerID).c_str()); \
strname ## .append("_"); \
strname ## .append(std::to_string(node ## ->m_nodeHeader.m_level).c_str()); \
strname ## .append("_"); \
strname ## .append(std::to_string(ExtentOp<EXTENT>::GetXMin(node ## ->m_nodeHeader.m_nodeExtent)).c_str()); \
strname ## .append("_"); \
strname ## .append(std::to_string(ExtentOp<EXTENT>::GetYMin(node ## ->m_nodeHeader.m_nodeExtent)).c_str()); \
}

#define LOGSTRING_NODE_INFO_W(node, strname) { \
    strname ## .append(std::to_wstring(node ## ->GetBlockID().m_integerID).c_str()); \
    strname ## .append(L"_"); \
    strname ## .append(std::to_wstring(node ## ->m_nodeHeader.m_level).c_str()); \
    strname ## .append(L"_"); \
    strname ## .append(std::to_wstring(ExtentOp<EXTENT>::GetXMin(node ## ->m_nodeHeader.m_nodeExtent)).c_str()); \
    strname ## .append(L"_"); \
    strname ## .append(std::to_wstring(ExtentOp<EXTENT>::GetYMin(node ## ->m_nodeHeader.m_nodeExtent)).c_str()); \
}

#define LOG_MESH_FROM_FILENAME_AND_BUFFERS_W(_filename, _nvertex, _nindex, _vbuffer, _indbuffer ) { \
     size_t _nVertices = _nvertex; \
    size_t _nIndices = _nindex; \
    FILE* _meshFile = _wfopen(_filename.c_str(), L"wb"); \
    fwrite(&_nVertices, sizeof(size_t), 1, _meshFile); \
    fwrite(_vbuffer, sizeof(DPoint3d), _nVertices, _meshFile); \
    fwrite(&_nIndices, sizeof(size_t), 1, _meshFile); \
    fwrite(_indbuffer, sizeof(int32_t), _nIndices, _meshFile); \
    fclose(_meshFile); \
}

#define LOG_POLY_FROM_FILENAME_AND_BUFFERS_W(_filename, _nvertex,_vbuffer) { \
     size_t _nVertices = _nvertex; \
    FILE* _meshFile = _wfopen(_filename.c_str(), L"wb"); \
    fwrite(&_nVertices, sizeof(size_t), 1, _meshFile); \
    fwrite(_vbuffer, sizeof(DPoint3d), _nVertices, _meshFile); \
    fclose(_meshFile); \
}

#define LOG_STATS_FOR_NODE(_filename, _node) { \
    std::ofstream stats; \
    stats.open(_filename.c_str(), std::ios_base::trunc); \
    stats << " N OF POINTS " + std::to_string(_node ## ->size()) + "\n"; \
    stats << "N OF INDICES " + std::to_string(_node ## ->m_nodeHeader.m_nbFaceIndexes) + "\n"; \
    stats << " NODE TOTAL COUNT " + std::to_string(_node ##->m_nodeHeader.m_totalCount) + "\n"; \
    stats.close(); \
}