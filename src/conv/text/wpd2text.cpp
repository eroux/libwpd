/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2002-2003 William Lachance (wrlach@gmail.com)
 * Copyright (C) 2002-2004 Marc Maurer (uwog@uwog.net)
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

#include <stdio.h>
#include <string.h>
#include <librevenge-stream/librevenge-stream.h>
#include <librevenge-generators/librevenge-generators.h>
#include <libwpd/libwpd.h>
#include <sstream>
#include <vector>

// Forward declarations for font mapping
class WPXLegacyFontMap;
struct WPDExtractionOptions;

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef VERSION
#define VERSION "UNKNOWN VERSION"
#endif

using namespace libwpd;

namespace
{

int printUsage()
{
	printf("`wpd2text' converts WordPerfect documents to plain text.\n");
	printf("\n");
	printf("Usage: wpd2text [OPTION] FILE\n");
	printf("\n");
	printf("Options:\n");
	printf("\t--info                display document metadata instead of the text\n");
	printf("\t--help                show this help message\n");
	printf("\t--password PASSWORD   try to decrypt password protected document\n");
	printf("\t--version             print version and exit\n");
	printf("\t--follow-packets      follow box/figure text packets (default: on)\n");
	printf("\t--no-follow-packets   don't follow box/figure text packets\n");
	printf("\t--legacy-font-map=PATH\n");
	printf("\t                      load legacy font mapping from JSON file\n");
	printf("\t--legacy-font-name=NAMES\n");
	printf("\t                      comma-separated allowlist of font names for mapping\n");
	printf("\n");
	printf("Report bugs to <https://sourceforge.net/p/libwpd/tickets/> or <https://bugs.documentfoundation.org/>.\n");
	return -1;
}

int printVersion()
{
	printf("wpd2text %s\n", VERSION);
	return 0;
}

} // anonymous namespace

// Helper function to split comma-separated font names
std::vector<std::string> splitFontNames(const std::string &names)
{
	std::vector<std::string> result;
	std::stringstream ss(names);
	std::string item;
	while (std::getline(ss, item, ','))
	{
		// Trim whitespace
		item.erase(0, item.find_first_not_of(" \t"));
		item.erase(item.find_last_not_of(" \t") + 1);
		if (!item.empty())
			result.push_back(item);
	}
	return result;
}


int main(int argc, char *argv[])
{
	if (argc < 2)
		return printUsage();

	char *szInputFile = nullptr;
	bool isInfo = false;
	char *password = nullptr;
	bool followPackets = true;
	std::string legacyFontMapPath;
	std::vector<std::string> legacyFontNames;

	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "--password"))
		{
			if (i < argc - 1)
				password = argv[++i];
		}
		else if (!strncmp(argv[i], "--password=", 11))
			password = &argv[i][11];
		else if (!strcmp(argv[i], "--info"))
			isInfo = true;
		else if (!strcmp(argv[i], "--version"))
			return printVersion();
		else if (!strcmp(argv[i], "--follow-packets"))
			followPackets = true;
		else if (!strcmp(argv[i], "--no-follow-packets"))
			followPackets = false;
		else if (!strncmp(argv[i], "--legacy-font-map=", 18))
			legacyFontMapPath = &argv[i][18];
		else if (!strncmp(argv[i], "--legacy-font-name=", 19))
			legacyFontNames = splitFontNames(&argv[i][19]);
		else if (!szInputFile && strncmp(argv[i], "--", 2))
			szInputFile = argv[i];
		else
			return printUsage();
	}

	if (!szInputFile)
		return printUsage();

	librevenge::RVNGFileStream input(szInputFile);

	WPDConfidence confidence = WPDocument::isFileFormatSupported(&input);
	if (confidence != WPD_CONFIDENCE_EXCELLENT && confidence != WPD_CONFIDENCE_SUPPORTED_ENCRYPTION)
	{
		fprintf(stderr, "ERROR: Unsupported file format!\n");
		return 1;
	}

	if (confidence == WPD_CONFIDENCE_SUPPORTED_ENCRYPTION && !password)
	{
		fprintf(stderr, "ERROR: File is password protected! Use \"--password\" option!\n");
		return 1;
	}

	if (confidence == WPD_CONFIDENCE_SUPPORTED_ENCRYPTION && password && (WPD_PASSWORD_MATCH_OK != WPDocument::verifyPassword(&input, password)))
	{
		fprintf(stderr, "ERROR: The password does not match, or document is not encrypted!\n");
		return 1;
	}

	// TODO: Set up extraction options for legacy font mapping
	// (Currently disabled due to linking issues)
	
	librevenge::RVNGString document;
	librevenge::RVNGTextTextGenerator documentGenerator(document, isInfo);
	WPDResult error = WPDocument::parse(&input, &documentGenerator, password);

	if (error == WPD_FILE_ACCESS_ERROR)
		fprintf(stderr, "ERROR: File Exception!\n");
	else if (error == WPD_PARSE_ERROR)
		fprintf(stderr, "ERROR: Parse Exception!\n");
	else if (error == WPD_UNSUPPORTED_ENCRYPTION_ERROR)
		fprintf(stderr, "ERROR: File is password protected! (Unsupported encryption)\n");
	else if (error == WPD_OLE_ERROR)
		fprintf(stderr, "ERROR: File is an OLE document, but does not contain a WordPerfect stream!\n");
	else if (error != WPD_OK)
		fprintf(stderr, "ERROR: Unknown Error!\n");

	if (error != WPD_OK)
		return 1;

	printf("%s", document.cstr());
	
	// TODO: Clean up extraction options when implemented

	return 0;
}
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */
