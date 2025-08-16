#ifndef WPD_EXTRACTION_OPTIONS_H
#define WPD_EXTRACTION_OPTIONS_H

#include <memory>
#include <unordered_set>
#include <string>

class WPXLegacyFontMap; // forward declaration

struct WPDExtractionOptions
{
    bool followPackets;
    std::shared_ptr<WPXLegacyFontMap> legacyFontMap;
    std::unordered_set<std::string> allowedFontNames;

    WPDExtractionOptions()
        : followPackets(true)
        , legacyFontMap(nullptr)
        , allowedFontNames()
    {}
};

#endif // WPD_EXTRACTION_OPTIONS_H