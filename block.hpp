#pragma once
#include <vector>
#include <set>
#include "argument.hpp"
#include "logger.hpp"
#include "vertex.hpp"

namespace graph_info {
using BlockId = std::string;

class BlockTree {
public:
    BlockId id;
    int dim = 0;

    BlockTree() = default;
    BlockTree(BlockId id_, int dim_, ArgTagsInfo args);

    // makes new children, inherit params
    void new_child();
    // set "{father.id}::{id}" id to last child
    void set_child_id(BlockId id_);
    // set `father.dim + id.dim` dim to last child
    void set_child_dim(int dim_);

    void print_leaves() const;
    bool has_child_with_id(BlockId id) const { return childrenIds_.count(id); }

    void rebuild_external_blocks();
    void collect_leaves(std::vector<BlockTree> &);

    VertexTagsInfo& get_vertices() { return vertices_; }
    const VertexTagsInfo& get_vertices() const { return vertices_; }

    ArgTagsInfo& get_args() { return args_; }
    const ArgTagsInfo& get_args() const { return args_; }

    BlockTree& get_last_child() { return children_.back(); }
    const BlockTree& get_last_child() const { return children_.back(); }

    std::vector<BlockTree>& get_childen() { return children_; }
    const std::vector<BlockTree>& get_childen() const { return children_; }

private:
    // should be empty if not leaf
    VertexTagsInfo vertices_;

    ArgTagsInfo args_;
    std::set<BlockId> childrenIds_;
    std::vector<BlockTree> children_;
};
} // namespace graph_info
