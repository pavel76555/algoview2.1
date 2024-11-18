#include "block_field.hpp"
#include <iostream>
#include <algorithm>
#include "expr.hpp"
#include "json_traverser.hpp"
#include "logger.hpp"
#include "output_file_manager.hpp"

const std::string file_name = "block_field.cpp";

namespace graph {
using namespace graph_info;
using namespace graph_manager;
using namespace algoview_json_traverser;
using namespace reg_expr;
using namespace logger;
using namespace output_file_manager;
using VarsMap = std::map<std::string, double>;
static int x_block_shift = 0;
// int i_o_vertex_flag = 0;

void calc_src_vertex_coords(const std::string& src_string,
                            CoordType& i, CoordType& j, CoordType& k,
                            const VarsMap& vars_map) {

    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("getting scr vertex coords values");

    std::vector<std::string> splited;
    const std::string delimiter = ",";

    size_t start = 0;
    size_t end = src_string.find(delimiter);
    while (end != std::string::npos) {
        splited.push_back(src_string.substr(start, end - start));
        start = end + 1;
        end = src_string.find(delimiter, start);
    }
    splited.push_back(src_string.substr(start));

    if (splited.size() == 3) {
        i = calc_expr(splited[0], vars_map);
        j = calc_expr(splited[1], vars_map);
        k = calc_expr(splited[2], vars_map);
    } else if (splited.size() == 2) {
        i = calc_expr(splited[0], vars_map);
        j = calc_expr(splited[1], vars_map);
        k = 0;
    } else {
        i = calc_expr(splited[0], vars_map);
        j = 0;
        k = 0;
    }

    const std::string msg = "i src coord = " + std::to_string(i) + "\tj src coord = " + std::to_string(j) +
            "\tk src coord = " + std::to_string(k);
    logger.log_info_msg(msg);
    logger.log_info_finish_msg("getting scr vertex coords values");
    logger.log_file_exit(__func__, __FILE__);
}

void create_var_value_map(const ParamsMap& params_map, VarsMap& vars_map) {
    const std::string func_name = "create_var_value_map";
    auto& logger = Logger::get_instance();
    logger.log_file_enter(func_name, file_name);
    logger.log_info_start_msg("creating var_value map");

    for (const auto& param : params_map) {
        vars_map[param.first] = param.second;
    }

    logger.log_file_exit(func_name, file_name);
    logger.log_info_finish_msg("creating var_value map");
}

void change_var_value_map(std::string var_name, int var_value, VarsMap& vars_map) {
    const std::string func_name = "change_var_value_map";
    auto& logger = Logger::get_instance();
    logger.log_file_enter(func_name, file_name);
    logger.log_info_start_msg("adding new variable to var_value map");
    std::string msg = var_name + " = " + std::to_string(var_value);
    logger.log_info_msg(msg);

    vars_map[var_name] = var_value;

    logger.log_info_finish_msg("adding new variable to var_value map");
    logger.log_file_exit(func_name, file_name);
}


void create_edge(const VertexId& src, const VertexId& target, EdgeMapManager& edges_manager, GraphCharactManager& graph_character_manager, VertexMapManager& vertices_manager) {
    if (vertices_manager.get_vertex(src).type != "0") {
        edges_manager.add_edge(src, target, "1");
        graph_character_manager.inc_edges_counter();
    } else {
        edges_manager.add_edge(src, target);
    }
}

void ExternalBlock::main_cycle(BlockTree& blocks, const ParamsMap& params,
                               VertexMapManager& verticesManager, EdgeMapManager& edgesManager, GraphCharactManager& graphCharactManager) {
    auto &logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    VarsMap varsMap;
    create_var_value_map(params, varsMap);

    int max_shift = 0;

    logger.log_info_start_msg("iterating external for block " + blocks.id);
    auto &blockArgs = blocks.get_args();
    if (blockArgs.get_args().empty() && blocks.get_childen().size() == 1) {
        blockArgs = blocks.get_childen()[0].get_args();
    }
    const auto &args = blockArgs.get_args();

    int z_block_shift = 0;
    for (int i = args[0].begin; i <= args[0].end; ++i) {
        for (const auto &block : blocks.get_childen()) {
            const auto &local_args = block.get_args().get_args();
            logger.log_info_start_msg("start iterating in block " + block.id + " with " + args[0].name + "=" + std::to_string(i));
            int y_block_shift = 0;
            for (int j = local_args[1].begin; j <= local_args[1].end; ++j) {
                for (int k = local_args[2].begin; k <= local_args[2].end; ++k) {
                    // assert len(vertices) <= 1
                    for (const auto &vertex : block.get_vertices().get_vertices()) {
                        if (local_args[0].name != "_") {
                            change_var_value_map(local_args[0].name, i, varsMap);
                        }
                        if (local_args[1].name != "_") {
                            change_var_value_map(local_args[1].name, j, varsMap);
                        }
                        if (local_args[2].name != "_") {
                            change_var_value_map(local_args[2].name, k, varsMap);
                        }
                        logger.log_info_start_msg("calculating condition " + vertex.cond);
                        double cond = calc_expr(vertex.cond, varsMap);
                        logger.log_info_msg("Result = " + std::to_string(cond));
                        logger.log_info_finish_msg("calculating condition");

                        if (cond) {
                            VertexId vertexId = verticesManager.add_vertex(block.id, i, j, k, x_block_shift + k - args[0].begin, y_block_shift, z_block_shift, vertex.type);
                            graphCharactManager.inc_vertices_counter();
                            std::vector<int> src_vertices_levels;

                            for (const auto& bsrc : vertex.bsrc) {
                                int src_i, src_j, src_k;
                                calc_src_vertex_coords(bsrc.second, src_i, src_j, src_k, varsMap);
                                VertexId src_vertex_id = verticesManager.get_vertex_id(bsrc.first, src_i, src_j, src_k);
                                if (src_vertex_id == invalidVertexId) {
                                    const std::string msg = "Source vertex with coords [" + std::to_string(src_i) + ", " + std::to_string(src_j) + ", " + std::to_string(src_k) + "] in block " + bsrc.first + " does not exist, ignoring it";
                                    logger.add_user_warning(msg);
                                    continue;
                                }

                                src_vertices_levels.push_back(verticesManager.get_vertex(src_vertex_id).level);
                                create_edge(src_vertex_id, vertexId, edgesManager, graphCharactManager, verticesManager);
                            }

                            if (!src_vertices_levels.empty()) {
                                int level = *std::max_element(src_vertices_levels.begin(), src_vertices_levels.end()) + 1;
                                verticesManager.get_vertex(vertexId).level = level;
                                graphCharactManager.inc_level_vertex_counter(level);
                                graphCharactManager.add_critical_lenght(level - 1);
                            } else if (verticesManager.get_vertex(vertexId).level == 1) {
                                graphCharactManager.inc_level_vertex_counter(1);
                            }
                        }
                    }
                    max_shift = std::max(max_shift, x_block_shift + k - args[0].begin);
                }
                y_block_shift++;
            }
            logger.log_info_finish_msg("start iterating in block " + block.id + " with " + args[0].name + "=" + std::to_string(i));
            z_block_shift++;
        }
    }
    x_block_shift = max_shift + 2;

    logger.log_info_finish_msg("iterating external for block " + blocks.id);
    logger.log_file_exit(__func__, __FILE__);
}
} // namespace graph
