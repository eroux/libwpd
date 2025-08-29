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

#include "WP5UnsupportedFixedLengthGroup.h"
#include "WP5FileStructure.h"
#include "libwpd_internal.h"
#include <cstdio>
#include <vector>

WP5UnsupportedFixedLengthGroup::WP5UnsupportedFixedLengthGroup(librevenge::RVNGInputStream *input, WPXEncryption *encryption, unsigned char groupID)
	: WP5FixedLengthGroup(groupID)
{
	WPD_DEBUG_MSG(("WordPerfect: Handling an unsupported fixed length group\n"));
	_read(input, encryption);
	// After _read returns, bytes were consumed and trailer validated; for unsupported we can re-read for hex if desired.
}

void WP5UnsupportedFixedLengthGroup::_readContents(librevenge::RVNGInputStream *input, WPXEncryption *encryption)
{
	// Determine size from table
	unsigned char groupID = getGroup();
	if (groupID < 0xC0 || groupID > 0xCF)
		return;
	int size = WP5_FIXED_LENGTH_FUNCTION_GROUP_SIZE[groupID-0xC0];
	// We are positioned at start (after reading group id externally). We want to read size-2 payload, then the trailer byte is validated by caller.
	// Save position
	long start = input->tell();
	int payload = size - 2; // first byte (group id already consumed externally) + last byte (trailer) => payload bytes in between
	if (payload < 0) payload = 0;
	std::vector<unsigned char> bytes;
	bytes.reserve((unsigned)payload);
	for (int i = 0; i < payload; ++i)
		bytes.push_back(readU8(input, encryption));
	std::fprintf(stderr, "[WP5UnsupportedFixedLengthGroup] group=0x%02X size=%d payload=%d bytes\n", (unsigned)groupID, size, payload);
	// Hex dump
	unsigned offset = 0; unsigned len = (unsigned)bytes.size();
	while (offset < len)
	{
		unsigned lineLen = (len - offset) > 16 ? 16 : (len - offset);
		std::fprintf(stderr, "  %06x: ", offset);
		for (unsigned i = 0; i < lineLen; ++i) std::fprintf(stderr, "%02X ", bytes[offset + i]);
		for (unsigned i = lineLen; i < 16; ++i) std::fprintf(stderr, "   ");
		std::fprintf(stderr, " | ");
		for (unsigned i = 0; i < lineLen; ++i)
		{
			unsigned char c = bytes[offset + i];
			std::fprintf(stderr, "%c", (c >= 32 && c < 127) ? c : '.');
		}
		std::fprintf(stderr, "\n");
		offset += lineLen;
	}
	// Leave stream positioned at start+payload; caller will read trailer and validate.
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
