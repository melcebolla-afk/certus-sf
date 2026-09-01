#ifndef EVIDENCE_JSON_READER_H_INCLUDED
#define EVIDENCE_JSON_READER_H_INCLUDED

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace Stockfish::Evidence::Json {

std::optional<std::string> read_file(const std::string& path);
std::optional<std::uint32_t> u32_field(const std::string& json, const char* key);
std::optional<std::string> string_field(const std::string& json, const char* key);
std::optional<double>      number_field(const std::string& json, const char* key);
std::optional<std::uint32_t> u32_field_obj(const std::string& obj, const char* key);
std::optional<double>        number_field_obj(const std::string& obj, const char* key);
std::optional<std::string>   string_field_obj(const std::string& obj, const char* key);
std::vector<std::string>     string_array_field_obj(const std::string& obj, const char* key);
std::optional<bool>          bool_field_obj(const std::string& obj, const char* key);
std::optional<bool>          bool_field_obj(const std::string& obj, const char* key);
void for_each_object_in_array(const std::string& json, const char* array_key,
                              const std::function<void(const std::string&)>& fn);

}  // namespace Stockfish::Evidence::Json

#endif
