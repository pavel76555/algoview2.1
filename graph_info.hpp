#pragma once
#include <map>
#include "argument.hpp"
#include "block.hpp"
#include "vertex.hpp"

namespace graph_info {

using ParamsMap = std::map<std::string, double>;

class GraphInfo {
public:
    void add_param(std::string name, int value);
    void print_params() const;
    void print_graph() const;
    void rebuild_external_blocks();
    ParamsMap& get_params();
    BlockTree& get_BlockTree() { return blocks_; }
private:
    ParamsMap params_;
    BlockTree blocks_;
};
} // namespace graph_info
