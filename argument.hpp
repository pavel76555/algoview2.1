#pragma once
#include <string>
#include <vector>

namespace graph_info {
struct ArgTagInfo {
    std::string name;
    std::string begin;
    std::string end;
};

class ArgTagsInfo {
public:
    ArgTagsInfo();

    void add_arg(std::string name, std::string begin_value, std::string end_value);
    bool is_arg_name_unique(std::string name);
    void print_arg_tags() const;
    const std::vector<ArgTagInfo>& get_args() const;

private:
    int n_ = 0;
    std::vector<ArgTagInfo> arg_tags_;
};
} // namespace graph_info
