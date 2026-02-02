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

#ifndef STAMINA_GUI_ADDONS_HIGHLIGHTER_H
#define STAMINA_GUI_ADDONS_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpressionMatchIterator>
#include <QRegularExpression>
#include <QTextDocument>

#include "ColorSchemes.h"

namespace stamina {
	namespace gui {
		namespace addons {
			namespace highlighter {
				class Highlighter : public QSyntaxHighlighter {
					Q_OBJECT
				public:
					Highlighter(QTextDocument * parent = nullptr);
					void setColorsFromScheme(ColorScheme * colors);
					ColorScheme * getColorsAsScheme();
				protected:
					// Rules
					virtual void setupKeyWordPatterns() = 0;

					void highlightBlock(const QString & text) override;

					struct HighlightingRule {
						QRegularExpression pattern;
						QTextCharFormat format;
					};
					QVector<HighlightingRule> highlightingRules;

					QRegularExpression commentStartExpression;
					QRegularExpression commentEndExpression;

					QTextCharFormat keywordFormat;
					QTextCharFormat typeFormat;
					QTextCharFormat classFormat;
					QTextCharFormat singleLineCommentFormat;
					QTextCharFormat multiLineCommentFormat;
					QTextCharFormat quotationFormat;
					QTextCharFormat functionFormat;
					QTextCharFormat numberFormat;
					QTextCharFormat constFormat;

					bool colorsWereSetup = false;
				};
			}
		}
	}
}

#endif // STAMINA_GUI_ADDONS_HIGHLIGHTER_H
