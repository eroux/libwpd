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

#ifndef WPXLEGACYFONTMAP_H
#define WPXLEGACYFONTMAP_H

#include <map>
#include <string>

class WPXLegacyFontMap
{
public:
	WPXLegacyFontMap();
	~WPXLegacyFontMap();
	
	bool loadFromFile(const std::string &filename);
	std::string mapCharacter(const std::string &fontName, unsigned char sourceChar) const;
	bool hasFontMapping(const std::string &fontName) const;
	
private:
	std::map<std::string, std::map<unsigned char, std::string>> m_fontMappings;
};

#endif /* WPXLEGACYFONTMAP_H */
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */