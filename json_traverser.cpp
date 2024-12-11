#include "json_traverser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include "argument.hpp"
#include "block.hpp"
#include "expr.hpp"
#include "logger.hpp"
#include "output_file_manager.hpp"
#include "vertex.hpp"

namespace algoview_json_traverser {

using namespace rapidjson;
using namespace reg_expr;
using namespace logger;
using namespace output_file_manager;

void parse_value(const std::string& str, std::string& left_value, std::string& right_value, char parse_sym) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    std::string msg = "Start parsing string = " + str + " by symbol" + parse_sym;
    logger.log_info_msg(msg);

    int first_parse_sym_pos = str.find(parse_sym);
    if (first_parse_sym_pos == std::string::npos) {
        msg = "Expression " + str + " should contain a parse symbol (";
        msg.push_back(parse_sym);
        msg += ")";
        logger.log_err_msg(__func__, __FILE__, msg);
        logger.add_user_error(msg);
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    char* c_left_value = (char*)malloc(first_parse_sym_pos);
    if (c_left_value == NULL) {
        logger.log_err_msg(__func__, __FILE__, "Malloc error");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    int left_value_len = str.copy(c_left_value, first_parse_sym_pos, 0);
    if (left_value_len == 0) {
        logger.log_err_msg(__func__, __FILE__, "Copy error");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    c_left_value[left_value_len] = '\0';
    left_value = std::string(c_left_value);
    free(c_left_value);
    if (parse_sym == ',')
        first_parse_sym_pos++;
    else if (parse_sym == '.') {
        first_parse_sym_pos += 2;
        // int extra_parse_sym_pos = str.find(first_parse_sym_pos, parse_sym);
        // if (extra_parse_sym_pos == std::string::npos) {
        //     msg = "Expression " + str + " should contain a parse symbol (..)";
        //     logger.log_err_msg(__func__, __FILE__, msg);
        //     logger.add_user_error(msg);
        //     auto& output_file = OutputFileManager::get_instance();
        //     output_file.fatal_error_report();
        //     exit(1);
        // }
    }
    int right_value_len = str.length() - first_parse_sym_pos;
    char* c_right_value = (char*)malloc(right_value_len);
    if (c_right_value == NULL) {
        logger.log_err_msg(__func__, __FILE__, "Malloc error");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    left_value_len = str.copy(c_right_value, right_value_len, first_parse_sym_pos);
    if (left_value_len == 0) {
        logger.log_err_msg(__func__, __FILE__, "Copy error");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    c_right_value[left_value_len] = '\0';
    right_value = std::string(c_right_value);
    free(c_right_value);

    logger.log_info_finish_msg("parsing string");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_arg_element(const Value& arg_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    if (!arg_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "Arg tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!arg_tag.HasMember("@name")) {
        logger.log_err_msg(__func__, __FILE__, "Arg tag has no name");
        logger.add_user_error("Argument tag has no name");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!arg_tag["@name"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Arg name is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    logger.log_char_msg("Arg name = ", arg_tag["@name"].GetString());
    if (!arg_tag.HasMember("@val")) {
        logger.log_err_msg(__func__, __FILE__, "Arg value range not specified");
        logger.add_user_error("Argument value range not specified");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!arg_tag["@val"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Arg value is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    logger.log_char_msg("Arg val range = ", arg_tag["@val"].GetString());

    ArgTagsInfo& args = block.get_args();
    const ParamsMap& params = graph.get_params();

    std::string begin_expr, end_expr;
    parse_value(arg_tag["@val"].GetString(), begin_expr, end_expr, '.');
    std::string msg = "After parsing : begin = " + begin_expr + "\tend = " + end_expr;
    logger.log_info_msg(msg);
    double begin = calc_expr(begin_expr, params);
    double end = calc_expr(end_expr, params);
    msg.clear();
    msg = "After calculations : begin = " + std::to_string(begin) + "\tend = " + std::to_string(end);
    logger.log_info_msg(msg);
    args.add_arg(arg_tag["@name"].GetString(), (int)begin, (int)end);

    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_arg(const Value& arg_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing args");

    if (arg_tag.IsArray()) {
        for (SizeType i = 0; i < arg_tag.Size(); i++) {
            traverse_arg_element(arg_tag[i], block, graph);
        }
    } else if (arg_tag.IsObject()) {
        traverse_arg_element(arg_tag, block, graph);
    } else {
        logger.log_err_msg(__func__, __FILE__, "Arg tag has undefined type");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    logger.log_info_finish_msg("traversing args");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_vertex_in_element(const Value& in_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    if (!in_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "In tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!in_tag.HasMember("@src")) {
        logger.log_err_msg(__func__, __FILE__, "Src attr not specified");
        logger.add_user_error("Src attribute of tag <in> is not specified");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!in_tag["@src"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Src tag is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    VertexTagsInfo& vertices = block.get_vertices();
    if (in_tag.HasMember("@bsrc")) {
        if (!in_tag["@bsrc"].IsString()) {
            logger.log_err_msg(__func__, __FILE__, "Bsrc tag is not a string");
            logger.add_user_error("System error");
            auto& output_file = OutputFileManager::get_instance();
            output_file.fatal_error_report();
            exit(1);
        }
        logger.log_char_msg("bsrc block id = ", in_tag["@bsrc"].GetString());
        logger.log_char_msg("src coordinates = ", in_tag["@src"].GetString());
        vertices.add_bsrc(in_tag["@bsrc"].GetString(), in_tag["@src"].GetString());
    } else {
        logger.log_char_msg("src coordinates = ", in_tag["@src"].GetString());
        vertices.add_bsrc(block.id, in_tag["@src"].GetString());
    }
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_vertex_in(const Value& in_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing sources");

    if (in_tag.IsArray()) {
        for (SizeType i = 0; i < in_tag.Size(); i++) {
            traverse_vertex_in_element(in_tag[i], block, graph);
        }
    } else if (in_tag.IsObject()) {
        traverse_vertex_in_element(in_tag, block, graph);
    } else {
        logger.log_err_msg(__func__, __FILE__, "In tag has undefined type");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    logger.log_info_finish_msg("traverse_vertex_in");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_vertex_element(const Value& vertex_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    if (!vertex_tag.HasMember("@condition")) {
        logger.log_err_msg(__func__, __FILE__, "Vertex tag has no condition");
        logger.add_user_error("Vertex tag has no attribute condition (it should be, even if condition is empty)");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!vertex_tag["@condition"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Condition is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!vertex_tag["@type"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Type is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    VertexTagsInfo& vertices = block.get_vertices();
    vertices.new_vertex();
    for (Value::ConstMemberIterator itr = vertex_tag.MemberBegin(); itr != vertex_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "@condition")) {
            logger.log_char_msg("Current vertex condition = ", vertex_tag["@condition"].GetString());
            vertices.add_condition(vertex_tag["@condition"].GetString());
        } else if (!strcmp(itr->name.GetString(), "@type")) {
            logger.log_char_msg("Current vertex type = ", vertex_tag["@type"].GetString());
            vertices.add_type(vertex_tag["@type"].GetString());
        } else if (!strcmp(itr->name.GetString(), "in")) {
            logger.log_info_msg("Found first in tag");
            traverse_vertex_in(itr->value, block, graph);
        } else {
            logger.log_err_msg(__func__, __FILE__, "Undefined tag inside tag vertex");
            logger.add_user_error("Undefined tag inside tag vertex");
            auto& output_file = OutputFileManager::get_instance();
            output_file.fatal_error_report();
            exit(1);
        }
    }

    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_vertex(const Value& vertex_tag, BlockTree& block, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing verticies");

    if (vertex_tag.IsArray()) {
        for (SizeType i = 0; i < vertex_tag.Size(); i++) {
            traverse_vertex_element(vertex_tag[i], block, graph);
        }
    } else if (vertex_tag.IsObject()) {
        traverse_vertex_element(vertex_tag, block, graph);
    } else {
        logger.log_err_msg(__func__, __FILE__, "Vertex tag has undefined type");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    logger.log_info_finish_msg("traversing verticies");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_block_element(const Value& block_tag, BlockTree &parent, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    if (!block_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "Block tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (block_tag.HasMember("id"))
        if (!block_tag["@id"].IsString()) {
            logger.log_err_msg(__func__, __FILE__, "Id attr is not a string");
            logger.add_user_error("System error");
            auto& output_file = OutputFileManager::get_instance();
            output_file.fatal_error_report();
            exit(1);
        }
    if (!block_tag.HasMember("@dims")) {
        logger.log_err_msg(__func__, __FILE__, "Dimensional not specified");
        logger.add_user_error("Block dimensional not specified");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!block_tag["@dims"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Dimensional is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    for (Value::ConstMemberIterator itr = block_tag.MemberBegin(); itr != block_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "@id")) {
            parent.set_child_id(block_tag["@id"].GetString());
            logger.log_char_msg("Start handling block with id = ", block_tag["@id"].GetString());
        } else if (!strcmp(itr->name.GetString(), "@dims")) {
            if (!block_tag["@dims"].IsString()) {
                exit(1);
            }
            logger.log_char_msg("Current block dim = ", block_tag["@dims"].GetString());
            parent.set_child_dim(std::stoi(block_tag["@dims"].GetString()));
        } else if (!strcmp(itr->name.GetString(), "arg")) {
            logger.log_info_msg("Found first tag arg");
            traverse_arg(itr->value, parent.get_last_child(), graph);
        } else if (!strcmp(itr->name.GetString(), "vertex")) {
            logger.log_info_msg("Found first tag vertex");
            if (block_tag.HasMember("block")) {
                logger.log_err_msg(__func__, __FILE__, "Vertex in block " + parent.get_last_child().id + "should be in block");
                logger.add_user_error("Vertex in block " + parent.get_last_child().id + "should be in block");
                auto& output_file = OutputFileManager::get_instance();
                output_file.fatal_error_report();
                exit(1);
            }
            traverse_vertex(itr->value, parent.get_last_child(), graph);
        } else if (!strcmp(itr->name.GetString(), "block")) {
            logger.log_info_msg("Found tag block");
            traverse_blocks(itr->value, parent.get_last_child(), graph);
        } else {
            logger.log_err_msg(__func__, __FILE__, "Undefined tag inside block tag");
            logger.add_user_error("Undefined tag inside block tag");
            auto& output_file = OutputFileManager::get_instance();
            output_file.fatal_error_report();
            exit(1);
        }
    }
}

void JSON_Traverser::traverse_blocks(const Value& block_tag, BlockTree& parent,  GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing blocks");

    if (block_tag.IsArray()) {
        for (SizeType i = 0; i < block_tag.Size(); i++) {
            parent.new_child();
            traverse_block_element(block_tag[i], parent, graph);
        }
    } else if (block_tag.IsObject()) {
        parent.new_child();
        traverse_block_element(block_tag, parent, graph);
    } else {
        logger.log_err_msg(__func__, __FILE__, "Block tag has undefined type");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    logger.log_info_finish_msg("traversing external blocks");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_param_element(const Value& param_tag, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("handling parameter");

    if (!param_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "Param tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!param_tag.HasMember("@name")) {
        logger.log_err_msg(__func__, __FILE__, "Parameter has no name");
        logger.add_user_error("Parameter has no name");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!param_tag["@name"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Parameter's name is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!param_tag.HasMember("@type")) {
        logger.log_err_msg(__func__, __FILE__, "Parameter has no type");
        logger.add_user_error("Parameter has no type");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!param_tag["@type"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Parameter's type is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    if (!param_tag.HasMember("@value")) {
        logger.log_err_msg(__func__, __FILE__, "Parameter has no value");
        logger.add_user_error("Parameter has no value");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    if (!param_tag["@value"].IsString()) {
        logger.log_err_msg(__func__, __FILE__, "Parameter's value is not a string");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    graph.add_param(param_tag["@name"].GetString(), std::stoi(param_tag["@value"].GetString()));

    logger.log_info_finish_msg("handling parameter");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_param(const Value& param_tag, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    if (param_tag.IsArray()) {
        for (SizeType i = 0; i < param_tag.Size(); i++) {
            traverse_param_element(param_tag[i], graph);
        }
    } else if (param_tag.IsObject()) {
        traverse_param_element(param_tag, graph);
    } else {
        logger.log_err_msg(__func__, __FILE__, "Param tag has undefined type");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }

    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_params(const Value& params_tag, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing parameters");

    if (!params_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "Params tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    if (!params_tag.HasMember("param")) {
        logger.log_err_msg(__func__, __FILE__, "Params tag does not contain parameters' definitions");
        logger.add_user_error("Params tag does not contain parameters' definitions");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    traverse_param(params_tag["param"], graph);

    logger.log_info_finish_msg("traversing parameters");
    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse_algo(const Value& algo_tag, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);

    logger.log_info_start_msg("traversing tags inside algo tag");
    for (Value::ConstMemberIterator itr = algo_tag.MemberBegin(); itr != algo_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "params")) {
            logger.log_info_msg("Found tag params");
            traverse_params(itr->value, graph);
        } else if (!strcmp(itr->name.GetString(), "block")) {
            logger.log_info_msg("Found tag block");
            auto &parent = graph.get_BlockTree();
            traverse_blocks(itr->value, parent, graph);
        } else {
            logger.log_err_msg(__func__, __FILE__, "Undefined tag inside algo tag");
            logger.add_user_error("Undefined tag inside algo tag");
            auto& output_file = OutputFileManager::get_instance();
            output_file.fatal_error_report();
            exit(1);
        }
    }
    logger.log_info_finish_msg("traversing tags inside algo tag");

    logger.log_file_exit(__func__, __FILE__);
}

void JSON_Traverser::traverse(const Document& doc, GraphInfo& graph) {
    auto& logger = Logger::get_instance();
    logger.log_file_enter(__func__, __FILE__);
    logger.log_info_start_msg("traversing DOM tree");

    const Value& head_tag = doc["algo"];
    if (!head_tag.IsObject()) {
        logger.log_err_msg(__func__, __FILE__, "Algo tag is not an Object");
        logger.add_user_error("System error");
        auto& output_file = OutputFileManager::get_instance();
        output_file.fatal_error_report();
        exit(1);
    }
    traverse_algo(head_tag, graph);

    logger.log_info_finish_msg("traversing DOM tree");
    logger.log_file_exit(__func__, __FILE__);
}

} // namespace algoview_json_traverser
