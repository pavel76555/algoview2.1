#include <fstream>
#include <iostream>
#include <sstream>
#include "include/rapidjson/document.h"
#include "include/rapidjson/pointer.h"
#include "include/rapidjson/prettywriter.h"
#include "include/xml2json.hpp"

using namespace rapidjson;

static const char* kTypeNames[] = {"Null", "False", "True", "Object", "Array", "String", "Number"};

void parse_arg_element(const Value& arg_tag) {
    assert(arg_tag.IsObject() && "Тег arg не является объектом");
    assert(arg_tag.HasMember("@name") && "Не указано имя аргумента");
    assert(arg_tag["@name"].IsString());
    printf("name = %s\n", arg_tag["@name"].GetString());
    assert(arg_tag.HasMember("@val") && "Не указано значение аргумента");
    assert(arg_tag["@val"].IsString());
    printf("val = %s\n", arg_tag["@val"].GetString());
}

void parse_arg(const Value& arg_tag) {
    if (arg_tag.IsArray()) {
        for (SizeType i = 0; i < arg_tag.Size(); i++) {
            parse_arg_element(arg_tag[i]);
        }
    } else if (arg_tag.IsObject()) {
        parse_arg_element(arg_tag);
    } else {
        assert("Тег arg имеет неправильный тип");
    }
}

void parse_vertex_in_element(const Value& in_tag) {
    assert(in_tag.IsObject() && "Тег in не является объектом");
    if (in_tag.HasMember("@bsrc")) {
        assert(in_tag["@bsrc"].IsString());
        printf("bsrc = %s\n", in_tag["@bsrc"].GetString());
    }
    assert(in_tag.HasMember("@src") && "Не указан src");
    assert(in_tag["@src"].IsString());
    printf("src = %s\n", in_tag["@src"].GetString());
}

void parse_vertex_in(const Value& in_tag) {
    if (in_tag.IsArray()) {
        for (SizeType i = 0; i < in_tag.Size(); i++) {
            parse_vertex_in_element(in_tag[i]);
        }
    } else if (in_tag.IsObject()) {
        parse_vertex_in_element(in_tag);
    } else {
        assert("Тег in имеет неправильный тип");
    }
}

void parse_vertex_element(const Value& vertex_tag) {
    assert(vertex_tag.HasMember("@condition") && "Не указан condition");
    assert(vertex_tag["@condition"].IsString());
    assert(vertex_tag["@type"].IsString());
    for (Value::ConstMemberIterator itr = vertex_tag.MemberBegin(); itr != vertex_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "@condition")) {
            assert(vertex_tag["@condition"].IsString());
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        } else if (!strcmp(itr->name.GetString(), "@type")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        } else if (!strcmp(itr->name.GetString(), "in")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
            parse_vertex_in(itr->value);
        } else {
            assert("Неопознанный тег внутри vertex");
        }
    }
}

void parse_vertex(const Value& vertex_tag) {
    if (vertex_tag.IsArray()) {
        for (SizeType i = 0; i < vertex_tag.Size(); i++) {
            parse_vertex_element(vertex_tag[i]);
        }
    } else if (vertex_tag.IsObject()) {
        parse_vertex_element(vertex_tag);
    } else {
        assert("Тег vertex имеет неправильный тип");
    }
}

void parse_block_element(const Value& block_tag) {
    assert(block_tag.IsObject() && "Тег block не является объектом");
    assert(block_tag["@id"].IsString());
    assert(block_tag.HasMember("@dims") && "Не указана размерность пространства");
    assert(block_tag["@dims"].IsString());
    for (Value::ConstMemberIterator itr = block_tag.MemberBegin(); itr != block_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "@id")) {
            assert(block_tag["@id"].IsString());
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        } else if (!strcmp(itr->name.GetString(), "@dims")) {
            assert(block_tag["@dims"].IsString());
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
        } else if (!strcmp(itr->name.GetString(), "arg")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
            parse_arg(itr->value);
        } else if (!strcmp(itr->name.GetString(), "vertex")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
            parse_vertex(itr->value);
        } else {
            assert("Неопознанный тег внутри block");
        }
    }
}

void parse_block(const Value& block_tag) {
    if (block_tag.IsArray()) {
        for (SizeType i = 0; i < block_tag.Size(); i++) {
            parse_block_element(block_tag[i]);
        }
    } else if (block_tag.IsObject()) {
        parse_block_element(block_tag);
    } else {
        assert("Тег block имеет неправильный тип");
    }
}

void parse_param_element(const Value& param_tag) {
    assert(param_tag.IsObject() && "Тег param не является объектом");
    assert(param_tag.HasMember("@name") && "Не указано имя параметра");
    assert(param_tag["@name"].IsString());
    printf("name = %s\n", param_tag["@name"].GetString());
    assert(param_tag.HasMember("@type") && "Не указан тип параметра");
    assert(param_tag["@type"].IsString());
    printf("type = %s\n", param_tag["@type"].GetString());
    if (param_tag.HasMember("@value")) {
        assert(param_tag["@value"].IsString());
        printf("val = %s\n", param_tag["@value"].GetString());
    }
}

void parse_param(const Value& param_tag) {
    if (param_tag.IsArray()) {
        for (SizeType i = 0; i < param_tag.Size(); i++) {
            parse_param_element(param_tag[i]);
        }
    } else if (param_tag.IsObject()) {
        parse_param_element(param_tag);
    } else {
        assert("Тег param имеет неправильный тип");
    }
}

void parse_params(const Value& params_tag) {
    assert(params_tag.IsObject() && "Тег params не является объектом");
    assert(params_tag.HasMember("param") && "Не указаны параметры");
    parse_param(params_tag["param"]);
}

void parse_algo(const Value& algo_tag) {
    for (Value::ConstMemberIterator itr = algo_tag.MemberBegin(); itr != algo_tag.MemberEnd(); ++itr) {
        if (!strcmp(itr->name.GetString(), "params")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
            parse_params(itr->value);
        } else if (!strcmp(itr->name.GetString(), "block")) {
            printf("Type of member %s is %s\n", itr->name.GetString(), kTypeNames[itr->value.GetType()]);
            parse_block(itr->value);
        } else {
            assert("Неопознанный тег внутри algo");
        }
    }
}

void parse_json(Document& doc) {
    const Value& head_tag = doc["algo"];
    assert(head_tag.IsObject() && "Тег algo не является объектом");
    parse_algo(head_tag);
}

int main() {
    std::cout << "Start parsing a test file" << std::endl;
    std::ifstream file("test1.xml");  // open file by using constructor
    std::ostringstream oss;
    oss << file.rdbuf();
    const std::string json_str = xml2json(oss.str().data());
    std::cout << json_str << std::endl;
    std::cout << "Ready to parse json" << std::endl;
    const char* json_cstr = json_str.c_str();
    Document document;
    if (document.Parse(json_cstr).HasParseError())
        return 1;
    printf("\nParsing to document succeeded.\n");
    parse_json(document);
    //  traverse(document);
}