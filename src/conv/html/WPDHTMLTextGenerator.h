/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef WPDHTMLTEXTGENERATOR_H
#define WPDHTMLTEXTGENERATOR_H

#include <librevenge-generators/librevenge-generators.h>
#include <librevenge/librevenge.h>

/**
 * Custom HTML text generator for WordPerfect documents that provides explicit
 * line break and page break tags for better identification of break types.
 */
class WPDHTMLTextGenerator : public librevenge::RVNGHTMLTextGenerator
{
public:
    explicit WPDHTMLTextGenerator(librevenge::RVNGString &document);
    ~WPDHTMLTextGenerator() override;

    void insertLineBreak() override;
    void openPageSpan(const librevenge::RVNGPropertyList &propList) override;
    void closePageSpan() override;

private:
    void insertCustomPageBreak();
    bool m_isFirstPage;
    librevenge::RVNGString &m_documentRef;
};

#endif /* WPDHTMLTEXTGENERATOR_H */
/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */