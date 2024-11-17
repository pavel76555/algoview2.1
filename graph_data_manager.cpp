#include "graph_data_manager.hpp"
#include <iostream>
#include "logger.hpp"

namespace graph_manager {
using namespace logger;

int get_max(const std::vector<int>& levels) {
    int max_level = 0;
    for (auto level : levels) {
        if (level > max_level)
            max_level = level;
    }
    return max_level;
}

VertexId VertexMapManager::add_vertex(const BlockId &blockId,
                        const CoordType& i, const CoordType& j, const CoordType& k,
                        const CoordType& x, const CoordType& y, const CoordType& z,
                        const std::string& type) {
    VertexId new_vertex_id = get_new_vertex_id();
    vertices_[new_vertex_id] = Vertex{
        .block_id = blockId,
        .i = i, .j = j, .k = k,
        .x = x, .y = y, .z = z,
        .type = type, .level = (type == "0") ? 0 : 1
        };

    reverseVerticesMap_[blockId][Coords{.i = i, .j = j, .k = k}] = new_vertex_id;
    return new_vertex_id;
}

Vertex& VertexMapManager::get_vertex(const VertexId& vertexId) {
    return vertices_[vertexId];
}

const VertexId& VertexMapManager::get_vertex_id(const BlockId &blockId, const CoordType& i, const CoordType& j, const CoordType& k) {
    if (!reverseVerticesMap_[blockId].count(Coords{.i = i, .j = j, .k = k})) {
        return invalidVertexId;
    }
    return reverseVerticesMap_[blockId][Coords{.i = i, .j = j, .k = k}];
}

void EdgeMapManager::add_edge(const VertexId& src, const VertexId &dst, const std::string& type) {
    edges_[get_new_edge_id()] = Edge{.source_vertex_id=src, .target_vertex_id=dst, .type=type};
}

std::vector<VertexId> EdgeMapManager::get_target_vertex_ids(const VertexId &vertex_id) const {
    std::vector<VertexId> res;
    for (const auto& edge : edges_) {
        if (edge.second.source_vertex_id == vertex_id)
            res.push_back(edge.second.target_vertex_id);
    }
    return res;
}

void GraphCharactManager::inc_level_vertex_counter(int level) {
    if (graph_charact_.each_level_vertex_num.size() <= level) {
        graph_charact_.each_level_vertex_num.resize(level + 1);
    }
    graph_charact_.each_level_vertex_num[level]++;
}

void GraphCharactManager::add_critical_lenght(int length) {
    if (length > graph_charact_.critical_length)
        graph_charact_.critical_length = length;
}

void GraphCharactManager::calculate_width() {
    graph_charact_.width = get_max(graph_charact_.each_level_vertex_num);
}

std::string GraphCharactManager::to_json() {
    std::string result_string;
    calculate_width();
    result_string += "\n\t\"characteristics\":\n\t\t{ \"vertex_num\": ";
    result_string += std::to_string(graph_charact_.vertex_num);
    result_string += ", \"edge_num\": ";
    result_string += std::to_string(graph_charact_.edge_num);
    result_string += ", \"critical_path_length\": ";
    result_string += std::to_string(graph_charact_.critical_length);
    result_string += ", \"parallel_form_width\": ";
    result_string += std::to_string(graph_charact_.width);
    result_string += "},\n";
    return result_string;
}

std::string VertexMapManager::to_json() {
    std::string result_string;
    result_string += "\t\"vertices\": [";
    for (const auto& vertex : vertices_) {
        std::string vertex_string = "\n\t\t{ \"id\": " + std::to_string(vertex.first) + ", \"coordinates\": [" +
                                    std::to_string(vertex.second.x) + ", " + std::to_string(vertex.second.y) + ", " +
                                    std::to_string(vertex.second.z) + "], \"type\": \"" + vertex.second.type + "\"" +
                                    ", \"info\": \"" + vertex.second.info + "\"" +
                                    ", \"level\": " + std::to_string(vertex.second.level) + " },";
        result_string += vertex_string;
    }
    if (result_string.back() != '[')
        result_string.pop_back();
    result_string += "\n\t],\n";
    vertices_.clear();
    return result_string;
}

std::string EdgeMapManager::to_json() {
    std::string result_string;
    result_string += "\t\"edges\": [";
    for (const auto& edge : edges_) {
        std::string edge_string = "\n\t\t{ \"id\": " + std::to_string(edge.first) +
                                    ", \"sourceVertexId\": " + std::to_string(edge.second.source_vertex_id) +
                                    ", \"targetVertexId\": " + std::to_string(edge.second.target_vertex_id) +
                                    ", \"type\": \"" + edge.second.type + "\" },";
        result_string += edge_string;
    }
    if (result_string.back() != '[')
        result_string.pop_back();
    result_string += "\n\t],\n";
    edges_.clear();
    return result_string;
}

void print_json(VertexMapManager& vertices_manager, EdgeMapManager& edges_manager) {
    std::cout << "{";
    std::cout << vertices_manager.to_json();
    std::cout << edges_manager.to_json();
    std::cout << "}" << std::endl;
}
} // namespace graph_manager
