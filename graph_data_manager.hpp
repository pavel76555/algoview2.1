#pragma once
#include <map>
#include <string>
#include <vector>

namespace graph_manager {
using BlockId = std::string;
using VertexId = int;
using EdgeId = int;
using CoordType = int;

struct Vertex {
    std::string block_id;
    CoordType i, j, k;
    CoordType x, y, z;
    int level = 0;
    std::string type = "0";
    std::string info = "normal";
};

constexpr VertexId invalidVertexId = -1;

struct Coords {
    CoordType i, j, k;

    bool operator<(const Coords &other) const {
        if (i != other.i) {
            return i < other.i;
        }

        if (j != other.j) {
            return j < other.j;
        }

        return k < other.k;
    }
};

struct Edge {
    VertexId source_vertex_id, target_vertex_id;
    std::string type = "0";
};

struct GraphCharact {
    std::vector<int> each_level_vertex_num;
    int width = 0;
    int critical_length = 0;
    int vertex_num = 0;
    int edge_num = 0;
};

using ReverseVertexMap = std::map<BlockId, std::map<Coords, VertexId>>;
using VertexMap = std::map<VertexId, Vertex>;
using EdgeMap = std::map<EdgeId, Edge>;

class VertexMapManager {
public:
    VertexId add_vertex(const BlockId &blockId,
                        const CoordType& i, const CoordType& j, const CoordType& k,
                        const CoordType& x, const CoordType& y, const CoordType& z,
                        const std::string& type);
    Vertex& get_vertex(const VertexId& vertexId);
    const VertexId& get_vertex_id(const BlockId &blockId, const CoordType& i, const CoordType& j, const CoordType& k);
    void print_reverse_vertices_map();

    std::string to_json();
private:
    VertexId vertex_id_counter_ = 0;
    VertexId get_new_vertex_id() { return vertex_id_counter_++; }
    VertexMap vertices_;
    ReverseVertexMap reverseVerticesMap_;
};

class EdgeMapManager {
public:
    void add_edge(const VertexId& src, const VertexId &dst, const std::string& type = "0");
    std::vector<VertexId> get_target_vertex_ids(const VertexId &vertex_id) const;
    std::string to_json();
private:
    VertexId edge_id_counter_ = 0;
    VertexId get_new_edge_id() { return edge_id_counter_++; }
    EdgeMap edges_;
};

class GraphCharactManager {
public:
    void inc_vertices_counter() { graph_charact_.vertex_num++; };
    void inc_edges_counter() { graph_charact_.edge_num++; };
    void add_critical_lenght(int lenght);
    void inc_level_vertex_counter(int level);
    void calculate_width();
    std::string to_json();
private:
    GraphCharact graph_charact_;
};

void print_json(VertexMapManager& vertices_manager, EdgeMapManager& edges_manager);
} // namespace graph_manager
