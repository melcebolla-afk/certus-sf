#include "json_reader.h"

#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>

namespace Stockfish::Evidence::Json {

namespace {

std::optional<std::size_t> find_key(const std::string& json, const char* key, std::size_t from = 0) {
    const std::string needle = std::string("\"") + key + "\"";
    return json.find(needle, from);
}

std::optional<std::size_t> skip_ws(const std::string& json, std::size_t i) {
    while (i < json.size() && std::isspace(static_cast<unsigned char>(json[i])))
        ++i;
    return i < json.size() ? std::optional<std::size_t>{i} : std::nullopt;
}

std::optional<std::string> parse_string_at(const std::string& json, std::size_t quote) {
    if (quote >= json.size() || json[quote] != '"')
        return std::nullopt;
    std::string out;
    for (std::size_t i = quote + 1; i < json.size(); ++i)
    {
        const char c = json[i];
        if (c == '"')
            return out;
        if (c == '\\' && i + 1 < json.size())
        {
            ++i;
            const char e = json[i];
            switch (e)
            {
            case '"':
            case '\\':
            case '/':
                out += e;
                break;
            case 'b':
                out += '\b';
                break;
            case 'f':
                out += '\f';
                break;
            case 'n':
                out += '\n';
                break;
            case 'r':
                out += '\r';
                break;
            case 't':
                out += '\t';
                break;
            default:
                out += e;
                break;
            }
        }
        else
            out += c;
    }
    return std::nullopt;
}

std::optional<std::size_t> value_start(const std::string& json, const char* key, std::size_t from = 0) {
    const auto k = find_key(json, key, from);
    if (!k)
        return std::nullopt;
    std::size_t i = *k + std::string("\"").size() + std::strlen(key) + 1;
    if (i >= json.size())
        return std::nullopt;
    i = json.find(':', i);
    if (i == std::string::npos)
        return std::nullopt;
    ++i;
    return skip_ws(json, i);
}

}  // namespace

std::optional<std::string> read_file(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::nullopt;
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

std::optional<std::string> string_field(const std::string& json, const char* key) {
    const auto i = value_start(json, key);
    if (!i || *i >= json.size() || json[*i] != '"')
        return std::nullopt;
    return parse_string_at(json, *i);
}

std::optional<double> number_field(const std::string& json, const char* key) {
    const auto i = value_start(json, key);
    if (!i)
        return std::nullopt;
    std::size_t j = *i;
    while (j < json.size()
           && (std::isdigit(static_cast<unsigned char>(json[j])) || json[j] == '-' || json[j] == '+'
               || json[j] == '.' || json[j] == 'e' || json[j] == 'E'))
        ++j;
    if (j == *i)
        return std::nullopt;
    return std::stod(json.substr(*i, j - *i));
}

std::optional<std::uint32_t> u32_field(const std::string& json, const char* key) {
    const auto n = number_field(json, key);
    if (!n)
        return std::nullopt;
    return static_cast<std::uint32_t>(*n);
}

std::optional<std::string> string_field_obj(const std::string& obj, const char* key) {
    return string_field(obj, key);
}

std::optional<double> number_field_obj(const std::string& obj, const char* key) {
    return number_field(obj, key);
}

std::optional<std::uint32_t> u32_field_obj(const std::string& obj, const char* key) {
    return u32_field(obj, key);
}

std::vector<std::string> string_array_field_obj(const std::string& obj, const char* key) {
    std::vector<std::string> out;
    const auto               i = value_start(obj, key);
    if (!i || *i >= obj.size() || obj[*i] != '[')
        return out;
    std::size_t p = *i + 1;
    while (p < obj.size())
    {
        const auto ws = skip_ws(obj, p);
        if (!ws)
            break;
        p = *ws;
        if (obj[p] == ']')
            break;
        if (obj[p] == ',')
        {
            ++p;
            continue;
        }
        if (obj[p] == '"')
        {
            if (auto s = parse_string_at(obj, p))
            {
                out.push_back(std::move(*s));
                p = obj.find('"', p + 1);
                if (p == std::string::npos)
                    break;
                ++p;
            }
            else
                break;
        }
        else
            ++p;
    }
    return out;
}

std::optional<bool> bool_field_obj(const std::string& obj, const char* key) {
    const auto i = value_start(obj, key);
    if (!i || *i >= obj.size())
        return std::nullopt;
    if (obj.compare(*i, 4, "true") == 0)
        return true;
    if (obj.compare(*i, 5, "false") == 0)
        return false;
    if (obj[*i] == '"')
    {
        if (auto s = parse_string_at(obj, *i))
        {
            if (*s == "true")
                return true;
            if (*s == "false")
                return false;
        }
    }
    return std::nullopt;
}

void for_each_object_in_array(const std::string& json, const char* array_key,
                              const std::function<void(const std::string&)>& fn) {
    const auto i = value_start(json, array_key);
    if (!i || *i >= json.size() || json[*i] != '[')
        return;
    int         depth = 0;
    std::size_t start = std::string::npos;
    for (std::size_t p = *i + 1; p < json.size(); ++p)
    {
        const char c = json[p];
        if (c == '{')
        {
            if (depth == 0)
                start = p;
            ++depth;
        }
        else if (c == '}')
        {
            --depth;
            if (depth == 0 && start != std::string::npos)
            {
                fn(json.substr(start, p - start + 1));
                start = std::string::npos;
            }
        }
        else if (c == ']' && depth == 0)
            break;
    }
}

}  // namespace Stockfish::Evidence::Json
