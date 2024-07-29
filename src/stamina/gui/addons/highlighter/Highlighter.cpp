/**
 * STAMINA - the [ST]ochasic [A]pproximate [M]odel-checker for [IN]finite-state [A]nalysis
 * Copyright (C) 2023 Fluent Verification, Utah State University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/.
 *
 **/

#include "Highlighter.h"

#include <QRegularExpressionMatchIterator>
#include <QRegularExpression>

#include <core/StaminaMessages.h>

namespace stamina {
namespace gui {
namespace addons {
namespace highlighter {

Highlighter::Highlighter(QTextDocument * parent)
	: QSyntaxHighlighter(parent)
{
	// Intentionally left empty
}

void
Highlighter::setColorsFromScheme(ColorScheme * colors) {
	keywordFormat.setForeground(colors->keyword);
	typeFormat.setForeground(colors->type);
	classFormat.setForeground(colors->type);
	singleLineCommentFormat.setForeground(colors->comment);
	multiLineCommentFormat.setForeground(colors->comment);
	quotationFormat.setForeground(colors->string);
	functionFormat.setForeground(colors->function);
	numberFormat.setForeground(colors->number);
	constFormat.setForeground(colors->constant);
	setupKeyWordPatterns();
	// colorsWereSetup = true;
}

ColorScheme *
Highlighter::getColorsAsScheme() {
	if (!colorsWereSetup) {
		StaminaMessages::warning("Colors were not set up!");
	}
	// StaminaMessages::info(keywordFormat.foreground().color().name().toStdString());
	return new ColorScheme(
		// Keywords
		keywordFormat.foreground().color()
		// Comments
		, singleLineCommentFormat.foreground().color()
		// Numbers
		, numberFormat.foreground().color()
		// Types
		, typeFormat.foreground().color()
		// Functions
		, functionFormat.foreground().color()
		// Strings
		, quotationFormat.foreground().color()
		// Constants
		, constFormat.foreground().color()
	);
}

void Highlighter::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
	setCurrentBlockState(0);
	int startIndex = 0;
	if (previousBlockState() != 1) {
		startIndex = text.indexOf(commentStartExpression);
	}
	while (startIndex >= 0) {
		QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
		int endIndex = match.capturedStart();
		int commentLength = 0;
		if (endIndex == -1) {
			setCurrentBlockState(1);
			commentLength = text.length() - startIndex;
		} else {
			commentLength = endIndex - startIndex
							+ match.capturedLength();
		}
		setFormat(startIndex, commentLength, multiLineCommentFormat);
		startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
	}
}

} // namespace highlighter
} // namespace addons
} // namespace gui
} // namespace stamina
