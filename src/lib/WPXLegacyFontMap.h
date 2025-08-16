#ifndef WPX_LEGACY_FONT_MAP_H
#define WPX_LEGACY_FONT_MAP_H

#include <string>
#include <unordered_map>
#include <memory>

class WPXLegacyFontMap
{
public:
    WPXLegacyFontMap();

    struct FontMap {
        std::unordered_map<unsigned int, std::string> codepointMap; // byte -> UTF-8
    };

    static std::shared_ptr<WPXLegacyFontMap> loadFromJSON(const std::string &path, std::string &errMsg);

    bool hasFont(const std::string &fontNameLower) const;
    const FontMap *getFont(const std::string &fontNameLower) const;

private:
    // lowercased font name -> mapping
    std::unordered_map<std::string, FontMap> m_fontMappings;
};

#endif // WPX_LEGACY_FONT_MAP_H