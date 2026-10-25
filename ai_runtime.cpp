#include "ai_runtime.h"

#include <cctype>
#include <chrono>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
std::string cleanEnvValue(std::string value)
{
    while (!value.empty() && (value.front() == ' ' || value.front() == '\t' || value.front() == '\r' || value.front() == '\n'))
    {
        value.erase(value.begin());
    }
    while (!value.empty() && (value.back() == ' ' || value.back() == '\t' || value.back() == '\r' || value.back() == '\n'))
    {
        value.pop_back();
    }

    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\''))
    {
        const auto closingQuote = value.find(value.front(), 1);
        if (closingQuote != std::string::npos)
        {
            return value.substr(1, closingQuote - 1);
        }
    }

    const auto comment = value.find(" #");
    if (comment != std::string::npos)
    {
        value = value.substr(0, comment);
        while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
        {
            value.pop_back();
        }
    }
    return value;
}

std::string env(const char *name)
{
    const char *value = std::getenv(name);
    if (value != nullptr && *value != '\0')
    {
        return cleanEnvValue(value);
    }

    std::ifstream file(".env");
    if (!file)
    {
        return "";
    }

    std::string line;
    while (std::getline(file, line))
    {
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t'))
        {
            line.erase(line.begin());
        }

        const auto equals = line.find('=');
        if (equals == std::string::npos || line.front() == '#')
        {
            continue;
        }

        std::string key = line.substr(0, equals);
        while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
        {
            key.pop_back();
        }
        if (key != name)
        {
            continue;
        }

        return cleanEnvValue(line.substr(equals + 1));
    }

    return "";
}

bool mockEnabled()
{
    const std::string value = env("STORYLANG_AI_MOCK");
    return value == "1" || value == "true" || value == "TRUE";
}

std::string trim(std::string value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    value = value.substr(first, last - first + 1);

    while (!value.empty() && (value.back() == '.' || value.back() == ',' || value.back() == '!' || value.back() == '?'))
    {
        value.pop_back();
    }
    return value;
}

std::string lower(std::string value)
{
    for (char &ch : value)
    {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

std::string jsonEscape(const std::string &value)
{
    std::string result;
    for (char ch : value)
    {
        switch (ch)
        {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += ch; break;
        }
    }
    return result;
}

std::string shellQuote(const std::string &value)
{
    std::string result = "\"";
    for (char ch : value)
    {
        if (ch == '"')
        {
            result += '\\';
        }
        result += ch;
    }
    result += "\"";
    return result;
}

std::string readJsonString(const std::string &json, std::size_t keyStart)
{
    const auto colon = json.find(':', keyStart);
    if (colon == std::string::npos)
    {
        return "";
    }

    auto quote = json.find('"', colon + 1);
    if (quote == std::string::npos)
    {
        return "";
    }

    std::string result;
    for (std::size_t i = quote + 1; i < json.size(); ++i)
    {
        if (json[i] == '"')
        {
            return result;
        }

        if (json[i] != '\\' || i + 1 >= json.size())
        {
            result += json[i];
            continue;
        }

        const char escaped = json[++i];
        switch (escaped)
        {
        case 'n': result += '\n'; break;
        case 'r': result += '\r'; break;
        case 't': result += '\t'; break;
        case '"': result += '"'; break;
        case '\\': result += '\\'; break;
        case '/': result += '/'; break;
        default: result += escaped; break;
        }
    }
    return "";
}

std::string responseText(const std::string &json)
{
    const auto outputTextKey = json.find("\"output_text\"");
    if (outputTextKey != std::string::npos)
    {
        const auto value = readJsonString(json, outputTextKey);
        if (!value.empty())
        {
            return value;
        }
    }

    std::size_t search = 0;
    while (true)
    {
        const auto typeKey = json.find("\"type\"", search);
        if (typeKey == std::string::npos)
        {
            break;
        }

        if (readJsonString(json, typeKey) == "output_text")
        {
            const auto textKey = json.find("\"text\"", typeKey);
            if (textKey != std::string::npos)
            {
                return readJsonString(json, textKey);
            }
        }

        search = typeKey + 6;
    }

    const auto textKey = json.find("\"text\"");
    if (textKey != std::string::npos)
    {
        return readJsonString(json, textKey);
    }

    throw std::runtime_error("Gemini response did not contain output text");
}

std::string request(const std::string &prompt)
{
    const std::string key = env("GEMINI_API_KEY").empty() ? env("GOOGLE_API_KEY") : env("GEMINI_API_KEY");
    const std::string model = env("GEMINI_MODEL");
    if (key.empty() || model.empty())
    {
        throw std::runtime_error("Set GEMINI_API_KEY and GEMINI_MODEL, or set STORYLANG_AI_MOCK=1");
    }

    for (char ch : key)
    {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '-' && ch != '_' && ch != '.')
        {
            throw std::runtime_error("GEMINI_API_KEY contains unsupported characters; remove spaces or comments");
        }
    }

    for (char ch : model)
    {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '-' && ch != '_' && ch != '.')
        {
            throw std::runtime_error("GEMINI_MODEL contains an unsupported character");
        }
    }

    const auto stamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::string directory = env("TEMP");
    if (directory.empty()) directory = env("TMP");
    if (directory.empty()) directory = ".";
    if (directory.back() != '\\' && directory.back() != '/') directory += '\\';
    const std::string requestFile = directory + "storylang_ai_request_" + std::to_string(stamp) + ".json";
    const std::string responseFile = directory + "storylang_ai_response_" + std::to_string(stamp) + ".json";

    {
        std::ofstream file(requestFile);
        if (!file)
        {
            throw std::runtime_error("Could not create temporary AI request file");
        }
        file << "{\"contents\":[{\"parts\":[{\"text\":\"" << jsonEscape(prompt)
             << "\"}]}],\"generationConfig\":{\"maxOutputTokens\":64}}";
    }

    const std::string command = "curl.exe --fail-with-body --silent --show-error --max-time 60 "
        "--retry 2 --retry-delay 1 --retry-max-time 15 "
        "-H " + shellQuote("x-goog-api-key: " + key) + " "
        "-H " + shellQuote("Content-Type: application/json") + " "
        "--data-binary @" + shellQuote(requestFile) + " "
        "https://generativelanguage.googleapis.com/v1beta/models/" + model + ":generateContent "
        "--output " + shellQuote(responseFile);

    // shortcut: use the system curl executable; replace with a native HTTP client if portability matters.
    const int result = std::system(command.c_str());
    std::ifstream file(responseFile);
    std::string response((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::remove(requestFile.c_str());
    std::remove(responseFile.c_str());

    if (result != 0)
    {
        std::string detail = response.substr(0, 400);
        for (char &ch : detail)
        {
            if (ch == '\r' || ch == '\n') ch = ' ';
        }
        throw std::runtime_error("Gemini request failed: " + (detail.empty() ?
            "check API key, model, quota, or temporary service availability" : detail));
    }
    return responseText(response);
}

std::vector<std::string> readArguments(int argumentCount, va_list arguments)
{
    std::vector<std::string> values;
    for (int i = 0; i < argumentCount; ++i)
    {
        const char *value = va_arg(arguments, const char *);
        values.emplace_back(value == nullptr ? "" : value);
    }
    return values;
}
}

int story_ai_choose_values(const std::vector<std::string> &values)
{
    try
    {
        if (values.size() < 2)
        {
            throw std::runtime_error("AI.choose requires a prompt and at least one option");
        }

        if (mockEnabled())
        {
            return 0;
        }

        std::string prompt = values[0] + "\nReturn exactly one option: ";
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            if (i > 1) prompt += ", ";
            prompt += values[i];
        }

        const std::string answer = lower(trim(request(prompt)));
        for (std::size_t i = 1; i < values.size(); ++i)
        {
            if (answer == lower(values[i]))
            {
                return static_cast<int>(i - 1);
            }
        }

        throw std::runtime_error("AI.choose returned an option outside the allowed choices: " + answer);
    }
    catch (const std::exception &error)
    {
        std::cerr << "StoryLang AI error: " << error.what() << '\n';
        return -1;
    }
}

extern "C" int story_ai_choose(int argumentCount, ...)
{
    va_list arguments;
    va_start(arguments, argumentCount);
    const auto values = readArguments(argumentCount, arguments);
    va_end(arguments);
    return story_ai_choose_values(values);
}

void story_ai_generate_values(const std::vector<std::string> &values)
{
    try
    {
        if (values.size() != 1)
        {
            throw std::runtime_error("AI.generate requires exactly one prompt");
        }

        if (mockEnabled())
        {
            std::cout << "[mock AI] " << values[0] << '\n';
            return;
        }

        std::cout << request(values[0]) << '\n';
    }
    catch (const std::exception &error)
    {
        std::cerr << "StoryLang AI error: " << error.what() << '\n';
    }
}

extern "C" void story_ai_generate(int argumentCount, ...)
{
    va_list arguments;
    va_start(arguments, argumentCount);
    const auto values = readArguments(argumentCount, arguments);
    va_end(arguments);
    story_ai_generate_values(values);
}
