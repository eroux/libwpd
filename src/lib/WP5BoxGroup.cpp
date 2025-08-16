/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2007 Fridrich Strba (fridrich.strba@bluewin.ch)
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

#include "WP5BoxGroup.h"
#include "WPXListener.h"
#include "libwpd_internal.h"
#include "WP5FileStructure.h"
#include "WP5PrefixData.h"
#include "WP5Listener.h"
#include "WP5GraphicsInformationPacket.h"
#include "WP5SubDocument.h"
#include "WPXContentListener.h"

#include <cstdio>
#include <algorithm>
#include <vector>

WP5BoxGroup::WP5BoxGroup(librevenge::RVNGInputStream *input, WPXEncryption *encryption) :
	WP5VariableLengthGroup(),
	m_boxNumber(0),
	m_positionAndType(0),
	m_alignment(0),
	m_width(0),
	m_height(0),
	m_x(0),
	m_y(0),
	m_boxType(0),
	m_graphicsOffset(0),
	m_data(nullptr),
	m_textSubDocument(),
	m_userDefinedSubDocument()
{
	_read(input, encryption);
}

static const char *subGroupNameFor(unsigned char sg)
{
	switch (sg)
	{
	case WP5_TOP_BOX_GROUP_FIGURE:           return "FIGURE";
	case WP5_TOP_BOX_GROUP_TABLE:            return "TABLE";
	case WP5_TOP_BOX_GROUP_TEXT_BOX:         return "TEXT_BOX";
	case WP5_TOP_BOX_GROUP_USER_DEFINED_BOX: return "USER_DEFINED_BOX";
	case WP5_TOP_BOX_GROUP_EQUATION:         return "EQUATION";
	case WP5_TOP_BOX_GROUP_HORIZONTAL_LINE:  return "HORIZONTAL_LINE";
	case WP5_TOP_BOX_GROUP_VERTICAL_LINE:    return "VERTICAL_LINE";
	default: return "UNKNOWN";
	}
}

/* Helper: look ahead for a WP5 variable-length function header.
   Typical inline subdocument starts with: G (>=0xD0), S (<0x80), LenLE(2 bytes). */
static bool wp5LooksLikeVLHeader(librevenge::RVNGInputStream *input, WPXEncryption *encryption)
{
	long p = (long)input->tell();
	unsigned char g = 0, s = 0;
	unsigned short L = 0;
	// Use the same helpers that honor encryption
	// NOTE: caller must ensure at least 4 bytes remain; if not, this will simply mis-detect and we restore pos.
	g = readU8(input, encryption);
	s = readU8(input, encryption);
	L = readU16(input, encryption);
	input->seek(p, librevenge::RVNG_SEEK_SET);
	(void)L;
	return (g >= 0xD0) && (s < 0x80);
}

void WP5BoxGroup::_readContents(librevenge::RVNGInputStream *input, WPXEncryption *encryption)
{
	std::fprintf(stderr, "[WP5BoxGroup] \"%s\" box encountered (contentSize=%u)\n",
	             subGroupNameFor(getSubGroup()), (unsigned)getSize());

	switch (getSubGroup())
	{
	case WP5_TOP_BOX_GROUP_FIGURE:
	{
		m_boxNumber       = readU16(input, encryption);
		m_positionAndType = readU8(input, encryption);
		m_alignment       = readU8(input, encryption);
		m_width           = readU16(input, encryption);
		m_height          = readU16(input, encryption);
		m_x               = readU16(input, encryption);
		m_y               = readU16(input, encryption);

		input->seek(36, librevenge::RVNG_SEEK_CUR);
		m_boxType = readU8(input, encryption);
		if (m_boxType == 0x80)
		{
			input->seek(60, librevenge::RVNG_SEEK_CUR);
			m_graphicsOffset = readU16(input, encryption);
		}
		break;
	}

	case WP5_TOP_BOX_GROUP_TABLE:
		// Not handled here
		break;

	case WP5_TOP_BOX_GROUP_TEXT_BOX:
	{
		m_boxNumber       = readU16(input, encryption);
		m_positionAndType = readU8(input, encryption);
		m_alignment       = readU8(input, encryption);
		m_width           = readU16(input, encryption);
		m_height          = readU16(input, encryption);
		m_x               = readU16(input, encryption);
		m_y               = readU16(input, encryption);

		int remainingSize = static_cast<int>(getSize()) - 12;
		if (remainingSize < 0) remainingSize = 0;

		WPD_DEBUG_MSG(("WP5BoxGroup(TEXT_BOX): remaining content = %d\n", remainingSize));

		if (remainingSize > 0)
		{
			if (wp5LooksLikeVLHeader(input, encryption))
			{
				std::fprintf(stderr, "[WP5BoxGroup] TEXT_BOX: inline subdocument detected\n");
				m_textSubDocument.reset(new WP5SubDocument(input, encryption, (unsigned)remainingSize));
			}
			else
			{
				// Packet-referenced path: read the payload, scan for first VL header, build subdocument from that offset
				long payloadStart = (long)input->tell();
				std::vector<unsigned char> buf;
				buf.reserve(remainingSize);
				for (int i = 0; i < remainingSize; ++i)
					buf.push_back(readU8(input, encryption));

				int off = -1;
				for (int i = 0; i+3 < remainingSize; ++i)
				{
					unsigned char g  = buf[i];
					unsigned char s2 = buf[i+1];
					if (g >= 0xD0 && s2 < 0x80) { off = i; break; }
				}
				if (off < 0)
				{
					// Unknown payload layout: dump first up to 64 bytes for debugging
					int dumpN = remainingSize < 64 ? remainingSize : 64;
					std::fprintf(stderr, "[WP5BoxGroup] TEXT_BOX: no VL header; dumping first %d bytes:", dumpN);
					for (int i = 0; i < dumpN; ++i) std::fprintf(stderr, " %02X", buf[i]);
					std::fprintf(stderr, "\n");
					off = 0; // fallback: treat entire payload as subdocument
				}

				// Rewind to discovered offset and create subdocument
				input->seek(payloadStart + off, librevenge::RVNG_SEEK_SET);
				unsigned payLen = (unsigned)((remainingSize > off) ? (remainingSize - off) : 0);
				if (payLen > 0)
				{
					std::fprintf(stderr, "[WP5BoxGroup] TEXT_BOX: packet-referenced subdocument at +%d (len=%u)\n", off, payLen);
					m_textSubDocument.reset(new WP5SubDocument(input, encryption, payLen));
				}
				// Move stream to end of group content
				input->seek(payloadStart + remainingSize, librevenge::RVNG_SEEK_SET);
			}
		}
		break;
	}

	case WP5_TOP_BOX_GROUP_USER_DEFINED_BOX:
	{
		m_boxNumber       = readU16(input, encryption);
		m_positionAndType = readU8(input, encryption);
		m_alignment       = readU8(input, encryption);
		m_width           = readU16(input, encryption);
		m_height          = readU16(input, encryption);
		m_x               = readU16(input, encryption);
		m_y               = readU16(input, encryption);

		int remainingSize = static_cast<int>(getSize()) - 12;
		if (remainingSize < 0) remainingSize = 0;

		WPD_DEBUG_MSG(("WP5BoxGroup(USER_DEFINED_BOX): remaining content = %d\n", remainingSize));

		if (remainingSize > 0)
		{
			if (wp5LooksLikeVLHeader(input, encryption))
			{
				std::fprintf(stderr, "[WP5BoxGroup] USER_DEFINED_BOX: inline subdocument detected\n");
				m_userDefinedSubDocument.reset(new WP5SubDocument(input, encryption, (unsigned)remainingSize));
			}
			else
			{
				long payloadStart = (long)input->tell();
				std::vector<unsigned char> buf;
				buf.reserve(remainingSize);
				for (int i = 0; i < remainingSize; ++i)
					buf.push_back(readU8(input, encryption));

				int off = -1;
				for (int i = 0; i+3 < remainingSize; ++i)
				{
					unsigned char g  = buf[i];
					unsigned char s2 = buf[i+1];
					if (g >= 0xD0 && s2 < 0x80) { off = i; break; }
				}
				if (off < 0)
				{
					int dumpN = remainingSize < 64 ? remainingSize : 64;
					std::fprintf(stderr, "[WP5BoxGroup] USER_DEFINED_BOX: no VL header; dumping first %d bytes:", dumpN);
					for (int i = 0; i < dumpN; ++i) std::fprintf(stderr, " %02X", buf[i]);
					std::fprintf(stderr, "\n");
					off = 0;
				}

				input->seek(payloadStart + off, librevenge::RVNG_SEEK_SET);
				unsigned payLen = (unsigned)((remainingSize > off) ? (remainingSize - off) : 0);
				if (payLen > 0)
				{
					std::fprintf(stderr, "[WP5BoxGroup] USER_DEFINED_BOX: packet-referenced subdocument at +%d (len=%u)\n", off, payLen);
					m_userDefinedSubDocument.reset(new WP5SubDocument(input, encryption, payLen));
				}
				input->seek(payloadStart + remainingSize, librevenge::RVNG_SEEK_SET);
			}
		}
		break;
	}

	case WP5_TOP_BOX_GROUP_EQUATION:
	case WP5_TOP_BOX_GROUP_HORIZONTAL_LINE:
	case WP5_TOP_BOX_GROUP_VERTICAL_LINE:
	default:
		break;
	}
}

void WP5BoxGroup::parse(WP5Listener *listener)
{
	WPD_DEBUG_MSG(("WordPerfect: handling a Box group\n"));

	switch (getSubGroup())
	{
	case WP5_TOP_BOX_GROUP_FIGURE:
	{
		if (m_boxType != 0x80)
			break;

		if (listener->getGeneralPacketData(8))
			m_data = static_cast<const WP5GraphicsInformationPacket *>(
			             listener->getGeneralPacketData(8))->getImage(m_graphicsOffset);

		if (m_data)
		{
			listener->boxOn(m_positionAndType, m_alignment, m_width, m_height, m_x, m_y);
			listener->insertGraphicsData(m_data);
			listener->boxOff();
		}
		break;
	}

	case WP5_TOP_BOX_GROUP_TEXT_BOX:
	{
		listener->boxOn(m_positionAndType, m_alignment, m_width, m_height, m_x, m_y);

		if (m_textSubDocument)
		{
			m_textSubDocument->parse(listener);
		}
		else
		{
			std::fprintf(stderr, "[WP5BoxGroup] TEXT_BOX: no subdocument to parse\n");
		}

		listener->boxOff();
		break;
	}

	case WP5_TOP_BOX_GROUP_USER_DEFINED_BOX:
	{
		listener->boxOn(m_positionAndType, m_alignment, m_width, m_height, m_x, m_y);

		if (m_userDefinedSubDocument)
		{
			m_userDefinedSubDocument->parse(listener);
		}
		else
		{
			std::fprintf(stderr, "[WP5BoxGroup] USER_DEFINED_BOX: no subdocument to parse\n");
		}

		listener->boxOff();
		break;
	}

	case WP5_TOP_BOX_GROUP_TABLE:
	case WP5_TOP_BOX_GROUP_EQUATION:
	case WP5_TOP_BOX_GROUP_HORIZONTAL_LINE:
	case WP5_TOP_BOX_GROUP_VERTICAL_LINE:
	default:
		break;
	}
}
