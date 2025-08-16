#include "WPXLegacyFontMap.h"
#include <fstream>
#include <sstream>
#include <cctype>

// Explicitly initialize m_fontMappings (silences -Weffc++)
WPXLegacyFontMap::WPXLegacyFontMap()
    : m_fontMappings()
{}

static std::string toLower(const std::string &s)
{
    std::string r;
    r.reserve(s.size());
    for (char c : s)
        r.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return r;
}

std::shared_ptr<WPXLegacyFontMap> WPXLegacyFontMap::loadFromJSON(const std::string &path, std::string &errMsg)
{
    std::ifstream in(path.c_str(), std::ios::in | std::ios::binary);
    if (!in) {
        errMsg = "Cannot open legacy font map file: " + path;
        return nullptr;
    }
    std::ostringstream buf; buf << in.rdbuf();
    std::string text = buf.str();

    auto mapObj = std::shared_ptr<WPXLegacyFontMap>(new WPXLegacyFontMap());

    size_t pos = text.find("\"fonts\"");
    if (pos == std::string::npos) {
        errMsg = "No 'fonts' object in JSON";
        return nullptr;
    }

    size_t cursor = pos;
    while ((cursor = text.find('"', cursor)) != std::string::npos) {
        size_t keyStart = cursor + 1;
        size_t keyEnd = text.find('"', keyStart);
        if (keyEnd == std::string::npos) break;
        std::string key = text.substr(keyStart, keyEnd - keyStart);
        cursor = keyEnd + 1;

        size_t colon = text.find(':', cursor);
        if (colon == std::string::npos) break;
        cursor = colon + 1;

        if (key == "fonts")
            continue; // font names will follow

        size_t brace = text.find('{', cursor);
        if (brace == std::string::npos)
            continue;

        int depth = 0;
        size_t i = brace;
        for (; i < text.size(); ++i) {
            char ch = text[i];
            if (ch == '{') depth++;
            else if (ch == '}') {
                depth--;
                if (depth == 0) { ++i; break; }
            }
        }
        if (depth != 0)
            break;

        std::string body = text.substr(brace + 1, i - brace - 2);
        cursor = i;

        FontMap fm;
        size_t p = 0;
        while ((p = body.find('"', p)) != std::string::npos) {
            size_t kStart = p + 1;
            size_t kEnd = body.find('"', kStart);
            if (kEnd == std::string::npos) break;
            std::string codeStr = body.substr(kStart, kEnd - kStart);
            p = kEnd + 1;

            size_t colon2 = body.find(':', p);
            if (colon2 == std::string::npos) break;
            p = colon2 + 1;

            while (p < body.size() && std::isspace(static_cast<unsigned char>(body[p]))) ++p;
            if (p >= body.size() || body[p] != '"') continue;

            size_t vStart = p + 1;
            size_t vEnd = body.find('"', vStart);
            if (vEnd == std::string::npos) break;
            std::string val = body.substr(vStart, vEnd - vStart);
            p = vEnd + 1;

            try {
                unsigned int code = static_cast<unsigned int>(std::stoul(codeStr));
                fm.codepointMap[code] = val;
            } catch (...) {
                // ignore malformed entry
            }
        }

        if (!fm.codepointMap.empty()) {
            mapObj->m_fontMappings[toLower(key)] = std::move(fm);
        }
    }

    if (mapObj->m_fontMappings.empty()) {
        errMsg = "No font mappings parsed in " + path;
        return nullptr;
    }
    return mapObj;
}

bool WPXLegacyFontMap::hasFont(const std::string &fontNameLower) const
{
    return m_fontMappings.find(fontNameLower) != m_fontMappings.end();
}

const WPXLegacyFontMap::FontMap *WPXLegacyFontMap::getFont(const std::string &fontNameLower) const
{
    auto it = m_fontMappings.find(fontNameLower);
    if (it == m_fontMappings.end()) return nullptr;
    return &it->second;
}