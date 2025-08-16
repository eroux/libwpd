/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2024
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include "WPXLegacyFontMap.h"
#include "libwpd_internal.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

WPXLegacyFontMap::WPXLegacyFontMap()
{
}

WPXLegacyFontMap::~WPXLegacyFontMap()
{
}

bool WPXLegacyFontMap::loadFromFile(const std::string &filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		WPD_DEBUG_MSG(("WPXLegacyFontMap: Unable to open file: %s\n", filename.c_str()));
		return false;
	}
	
	// Read the entire file
	std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	file.close();
	
	// Simple JSON parser for our specific format
	// Expected format: { "fonts": { "TibetanMangala": { "33": "ཀ", ... } } }
	try
	{
		// Find the fonts object
		size_t fontsPos = content.find("\"fonts\"");
		if (fontsPos == std::string::npos)
		{
			WPD_DEBUG_MSG(("WPXLegacyFontMap: No 'fonts' section found in JSON\n"));
			return false;
		}
		
		// Find the opening brace for fonts
		size_t fontsBracePos = content.find("{", fontsPos);
		if (fontsBracePos == std::string::npos)
			return false;
			
		// Parse each font section
		size_t pos = fontsBracePos + 1;
		while (pos < content.length())
		{
			// Skip whitespace
			while (pos < content.length() && std::isspace(content[pos])) pos++;
			if (pos >= content.length() || content[pos] == '}') break;
			
			// Find font name
			if (content[pos] != '"') continue;
			pos++; // skip opening quote
			size_t fontNameStart = pos;
			size_t fontNameEnd = content.find('"', pos);
			if (fontNameEnd == std::string::npos) break;
			
			std::string fontName = content.substr(fontNameStart, fontNameEnd - fontNameStart);
			pos = fontNameEnd + 1;
			
			// Find font mapping object
			size_t fontObjStart = content.find("{", pos);
			if (fontObjStart == std::string::npos) break;
			pos = fontObjStart + 1;
			
			// Parse character mappings
			std::map<unsigned char, std::string> charMap;
			while (pos < content.length())
			{
				// Skip whitespace
				while (pos < content.length() && std::isspace(content[pos])) pos++;
				if (pos >= content.length() || content[pos] == '}') break;
				
				// Find character number
				if (content[pos] != '"') {
					pos++;
					continue;
				}
				pos++; // skip opening quote
				size_t charNumStart = pos;
				size_t charNumEnd = content.find('"', pos);
				if (charNumEnd == std::string::npos) break;
				
				std::string charNumStr = content.substr(charNumStart, charNumEnd - charNumStart);
				unsigned char charNum = (unsigned char)std::stoi(charNumStr);
				pos = charNumEnd + 1;
				
				// Skip colon
				while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ':')) pos++;
				
				// Find unicode string
				if (pos >= content.length() || content[pos] != '"') break;
				pos++; // skip opening quote
				size_t unicodeStart = pos;
				size_t unicodeEnd = content.find('"', pos);
				if (unicodeEnd == std::string::npos) break;
				
				std::string unicodeStr = content.substr(unicodeStart, unicodeEnd - unicodeStart);
				charMap[charNum] = unicodeStr;
				pos = unicodeEnd + 1;
				
				// Skip comma
				while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ',')) pos++;
			}
			
			if (!charMap.empty())
			{
				m_fontMappings[fontName] = charMap;
				WPD_DEBUG_MSG(("WPXLegacyFontMap: Loaded %zu mappings for font %s\n", charMap.size(), fontName.c_str()));
			}
			
			// Find end of font object
			while (pos < content.length() && content[pos] != '}') pos++;
			if (pos < content.length()) pos++; // skip closing brace
			
			// Skip comma
			while (pos < content.length() && (std::isspace(content[pos]) || content[pos] == ',')) pos++;
		}
	}
	catch (const std::exception &e)
	{
		WPD_DEBUG_MSG(("WPXLegacyFontMap: Error parsing JSON: %s\n", e.what()));
		return false;
	}
	
	return !m_fontMappings.empty();
}

std::string WPXLegacyFontMap::mapCharacter(const std::string &fontName, unsigned char sourceChar) const
{
	auto fontIt = m_fontMappings.find(fontName);
	if (fontIt == m_fontMappings.end())
		return "";
		
	auto charIt = fontIt->second.find(sourceChar);
	if (charIt == fontIt->second.end())
		return "";
		
	return charIt->second;
}

bool WPXLegacyFontMap::hasFontMapping(const std::string &fontName) const
{
	return m_fontMappings.find(fontName) != m_fontMappings.end();
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */