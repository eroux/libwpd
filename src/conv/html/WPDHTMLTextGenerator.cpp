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
    m_isFirstPage(true),
    m_documentRef(document)
{
}

WPDHTMLTextGenerator::~WPDHTMLTextGenerator()
{
}

void WPDHTMLTextGenerator::insertLineBreak()
{
    // Insert custom line break tag directly into HTML as raw HTML
    m_documentRef.append("<wp5-line-break></wp5-line-break>");
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
    // Insert custom page break tag directly into HTML as raw HTML
    m_documentRef.append("<wp5-page-break></wp5-page-break>");
}

/* vim:set shiftwidth=4 softtabstop=4 noexpandtab: */