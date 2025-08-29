/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2003 William Lachance (wrlach@gmail.com)
 * Copyright (C) 2003 Marc Maurer (uwog@uwog.net)
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

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "WP5UnsupportedVariableLengthGroup.h"
#include "libwpd_internal.h"
#include <cstdio>
#include <vector>

static void dumpHexBytes(const unsigned char *data, unsigned len)
{
	if (!data || !len) return;
	unsigned offset = 0;
	while (offset < len)
	{
		unsigned lineLen = (len - offset) > 16 ? 16 : (len - offset);
		std::fprintf(stderr, "  %06x: ", offset);
		for (unsigned i = 0; i < lineLen; ++i)
			std::fprintf(stderr, "%02X ", data[offset + i]);
		for (unsigned i = lineLen; i < 16; ++i)
			std::fprintf(stderr, "   ");
		std::fprintf(stderr, " | ");
		for (unsigned i = 0; i < lineLen; ++i)
		{
			unsigned char c = data[offset + i];
			std::fprintf(stderr, "%c", (c >= 32 && c < 127) ? c : '.');
		}
		std::fprintf(stderr, "\n");
		offset += lineLen;
	}
}

WP5UnsupportedVariableLengthGroup::WP5UnsupportedVariableLengthGroup(librevenge::RVNGInputStream *input, WPXEncryption *encryption) :
	WP5VariableLengthGroup()
{
	WPD_DEBUG_MSG(("WordPerfect: Handling an unsupported variable length group\n"));
	_read(input, encryption);
}

void WP5UnsupportedVariableLengthGroup::_readContents(librevenge::RVNGInputStream *input, WPXEncryption *encryption)
{
	// We are positioned just after subgroup + size (handled by WP5VariableLengthGroup::_read)
	// getSize() returns total size including 4 bytes overhead (codes). We already consumed 3 bytes (subgroup + size(2))
	unsigned toRead = (getSize() > 4) ? (getSize() - 4 - 0) : 0; // subtract 4 bytes overhead, but all remaining is payload here
	if (!toRead) return;
	long start = input->tell();
	std::vector<unsigned char> bytes;
	bytes.reserve(toRead);
	for (unsigned i = 0; i < toRead; ++i)
		bytes.push_back(readU8(input, encryption));
	std::fprintf(stderr, "[WP5UnsupportedVariableLengthGroup] subgroup=0x%02X size=%u payload=%u bytes\n", (unsigned)getSubGroup(), (unsigned)getSize(), toRead);
	dumpHexBytes(bytes.data(), (unsigned)bytes.size());
	// Seek back so base class validation reads trailer correctly
	input->seek(start, librevenge::RVNG_SEEK_SET);
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
