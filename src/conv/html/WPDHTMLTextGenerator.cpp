/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/* libwpd
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WPDHTMLTextGenerator.h"

WPDHTMLTextGenerator::WPDHTMLTextGenerator(librevenge::RVNGString &document) :
    librevenge::RVNGHTMLTextGenerator(document),
    m_isFirstPage(true)
{
}

WPDHTMLTextGenerator::~WPDHTMLTextGenerator()
{
}

void WPDHTMLTextGenerator::insertLineBreak()
{
    // Insert custom line break tag instead of standard <br>
    librevenge::RVNGString lineBreakTag;
    lineBreakTag.append("<wp5-line-break></wp5-line-break>");
    insertText(lineBreakTag);
}

void WPDHTMLTextGenerator::openPageSpan(const librevenge::RVNGPropertyList &propList)
{
    // Insert page break tag before opening new page span (except for first page)
    if (!m_isFirstPage)
    {
        insertCustomPageBreak();
    }
    m_isFirstPage = false;
    
    // Call parent implementation
    librevenge::RVNGHTMLTextGenerator::openPageSpan(propList);
}

void WPDHTMLTextGenerator::closePageSpan()
{
    // Call parent implementation
    librevenge::RVNGHTMLTextGenerator::closePageSpan();
}

void WPDHTMLTextGenerator::insertCustomPageBreak()
{
    // Insert custom page break tag
    librevenge::RVNGString pageBreakTag;
    pageBreakTag.append("<wp5-page-break></wp5-page-break>");
    insertText(pageBreakTag);
}

/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */