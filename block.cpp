#include "block.hpp"
#include "cassert"
#include "iostream"
#include "output_file_manager.hpp"

namespace graph_info {
using namespace logger;
using namespace output_file_manager;

BlockTree::BlockTree(BlockId id_, int dim_, ArgTagsInfo args)
    : id(id_)
    , dim(dim_)
    , args_(args)
    {}

void BlockTree::new_child() {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("adding child to block");

    if (has_child_with_id(id)) {
        logger.log_err_msg(__func__, __FILE__, "Block id is used twice");
        logger.add_user_error("Block id is used twice");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    children_.emplace_back(id, dim, args_);
    childrenIds_.insert(id);

    logger.log_info_finish_msg("adding child to block ");
    logger.log_file_exit(__func__, __FILE__);
}

void BlockTree::set_child_id(BlockId id_) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("add id to last child");

    childrenIds_.erase(id);

    auto new_id = id + "::" + id_;
    if (id.empty()) {
        new_id = id_;
    }

    if (has_child_with_id(new_id)) {
        logger.log_err_msg(__func__, __FILE__, "Block id is used twice");
        logger.add_user_error("Block id is used twice");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    childrenIds_.insert(new_id);
    children_.back().id = new_id;

    logger.log_info_finish_msg("add id to last child");
    logger.log_file_exit(__func__, __FILE__);
}

void BlockTree::set_child_dim(int dim_) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("adding block dim to internal structure");

    auto new_dim = dim + dim_;
    if (!(new_dim >= 1 && new_dim <= 3)) {
        std::string msg = "Invalid dimension of block";
        logger.log_err_msg(__func__, __FILE__, msg);
        logger.add_user_error(msg);
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    children_.back().dim = new_dim;

    logger.log_info_finish_msg("adding block dim to internal structure");
    logger.log_file_exit(__func__, __FILE__);
}

void BlockTree::print_leaves() const {
    if (children_.empty()) {
        std::cout << std::endl << "\t\t\tBLOCK TAG" << std::endl;
        std::cout << "id = " << id << std::endl;
        std::cout << "dim = " << dim << std::endl << std::endl;
        std::cout << "\t\t\tARG TAGS" << std::endl;
        args_.print_arg_tags();
        std::cout << "\t\t\tVERTEX TAGS" << std::endl;
        vertices_.print_vertex_tags();
    }

    for (const auto &child : children_) {
        child.print_leaves();
    }
}

void BlockTree::rebuild_external_blocks() {
    for (auto &child: children_) {
        std::vector<BlockTree> leaves;
        child.collect_leaves(leaves);

        child.children_ = leaves;
        child.childrenIds_.clear();
    }
}

void BlockTree::collect_leaves(std::vector<BlockTree> &res) {
    if (children_.empty()) {
        res.push_back(std::move(*this));
        return;
    }

    for (auto &child : children_) {
        child.collect_leaves(res);
    }
}

} // namespace graph_info
