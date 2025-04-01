#include <fstream>
#include <iostream>
#include "block_field.hpp"
#include "graph_data_manager.hpp"
#include "graph_info.hpp"
#include "json_traverser.hpp"
#include "logger.hpp"
#include "output_file_manager.hpp"
#include "xml_parser.hpp"

void set_up_logger(char* input_file_name) {
    std::string log_file_name(input_file_name);
    log_file_name += ".log.txt";
    auto& logger = logger::Logger::get_instance();
    logger.set_outdata_flow(log_file_name);
}

int main(int argc, char* argv[]) {
    using namespace rapidjson;
    using namespace algoview_xml_parser;
    using namespace algoview_json_traverser;
    using namespace graph_info;
    using namespace graph_manager;
    using namespace graph;
    using namespace logger;
    using namespace output_file_manager;
    auto& logger = Logger::get_instance();
    auto& output_file = OutputFileManager::get_instance();

    std::string output_file_name;
    if (argc == 3)
        output_file_name += argv[2];
    else
        output_file_name += "output.json";
    output_file.open(output_file_name);

    if (argc < 2) {
        std::cerr << "Less of arguments: name of input file is needed";
        logger.add_user_error("System error");
        output_file.fatal_error_report();
        exit(1);
    }

    // std::map<BlockId, BlockInfo*> block_map;
    XML_Parser parser;
    set_up_logger(argv[1]);
    logger.log_info_msg("--------------------------1ST STEP--------------------------");
    parser.parse(argv[1]);
    const Document& tree = parser.get_DOM_tree();
    GraphInfo graph;
    logger.log_info_msg("--------------------------2nd STEP--------------------------");
    JSON_Traverser::traverse(tree, graph);
    // convert tree block to map[blockId]map[blockId]block
    graph.rebuild_external_blocks();


    // BlockTagsInfo& blocks_info = graph.get_blocks();
    // const auto& blocks = blocks_info.get_blocks();
    const auto& params = graph.get_params();
    VertexMapManager vertices_manager;
    EdgeMapManager edges_manager;
    GraphCharactManager graph_charact_manager;
    // logger.log_info_msg("--------------------------3rd STEP--------------------------");
    // int it = 0;
    for (auto &external_block : graph.get_BlockTree().get_childen()) {
        ExternalBlock block;
        block.main_cycle(external_block, params, vertices_manager, edges_manager, graph_charact_manager);
    }
    output_file.write("{" + graph_charact_manager.to_json() + vertices_manager.to_json() + edges_manager.to_json() +
                      logger.warn_to_json() + logger.err_to_json() + "}");
    // graph_manager::print_json(vertices_manager, edges_manager);
}

// добавить const в print и get методы
// сделать файл с test mode
// возможно сделать методы парсера статиками
// добавить вывод ошибки подсчёта регулярного выражения
// calc_expr была шаблонной, но не компилировалась, теперь обрабатывает только double(?)
// переписать asserts (?)
// const block_info??
// поправить генерацию json без дуг
