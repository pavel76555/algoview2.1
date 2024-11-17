#pragma once
#include "graph_data_manager.hpp"
#include "graph_info.hpp"

namespace graph {
using namespace graph_info;
using namespace graph_manager;
using BlockId = std::string;
constexpr int ignore_vertex_id = -2;

class ExternalBlock {
public:
    void main_cycle(BlockTree& blocks, const ParamsMap& params,
                               VertexMapManager& vertexes, EdgeMapManager& edges, GraphCharactManager& graphCharact);
private:
};

} // namespace graph
