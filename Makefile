FLAGS=-g -std=c++17
all: main

rebuild: clean all

main: main.o xml_parser.o json_traverser.o argument.o vertex.o block.o graph_info.o expr.o graph_data_manager.o logger.o output_file_manager.o block_field.o
	g++ main.o xml_parser.o json_traverser.o argument.o vertex.o block.o graph_info.o expr.o block_field.o graph_data_manager.o logger.o output_file_manager.o $(FLAGS) -o main

main.o: main.cpp
	g++ -c main.cpp $(FLAGS)

xml_parser.o: xml_parser.cpp xml_parser.hpp
	g++ -c xml_parser.cpp $(FLAGS)

json_traverser.o: json_traverser.cpp json_traverser.hpp
	g++ -c json_traverser.cpp $(FLAGS)

argument.o: argument.cpp argument.hpp
	g++ -c argument.cpp $(FLAGS)

vertex.o: vertex.cpp vertex.hpp
	g++ -c vertex.cpp $(FLAGS)

block.o: block.cpp block.hpp
	g++ -c block.cpp $(FLAGS)

graph_info.o: graph_info.cpp graph_info.hpp
	g++ -c graph_info.cpp $(FLAGS)

expr.o: expr.cpp expr.hpp
	g++ -c expr.cpp $(FLAGS)

graph_data_manager.o: graph_data_manager.cpp graph_data_manager.hpp
	g++ -c graph_data_manager.cpp $(FLAGS)

block_field.o: block_field.cpp block_field.hpp
	g++ -c block_field.cpp $(FLAGS)

logger.o: logger.cpp logger.hpp
	g++ -c logger.cpp $(FLAGS)

output_file_manager.o: output_file_manager.cpp output_file_manager.hpp
	g++ -c output_file_manager.cpp $(FLAGS)

clean:
	rm -rf *.o main
