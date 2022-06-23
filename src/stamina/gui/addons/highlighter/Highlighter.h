#ifndef STAMINA_GUI_ADDONS_HIGHLIGHTER_H
#define STAMINA_GUI_ADDONS_HIGHLIGHTER_H

#include <QSyntaxHighlighter>

namespace stamina {
	namespace gui {
		namespace addons {
			namespace highlighter {
				class Highlighter : public QSyntaxHighlighter {
					Q_OBJECT
				public:
					Highlighter(QTextDocument * parent = nullptr);
				protected:
					// Rules
					virtual void setupKeyWordPatterns() = 0;

					void highlightBlock(const QString & text) override;

					struct HighlightingRule
					{
						QRegularExpression pattern;
						QTextCharFormat format;
					};
					QVector<HighlightingRule> highlightingRules;

					QRegularExpression commentStartExpression;
					QRegularExpression commentEndExpression;

					QTextCharFormat keywordFormat;
					QTextCharFormat classFormat;
					QTextCharFormat singleLineCommentFormat;
					QTextCharFormat multiLineCommentFormat;
					QTextCharFormat quotationFormat;
					QTextCharFormat functionFormat;
				};
			}
		}
	}
}

#endif // STAMINA_GUI_ADDONS_HIGHLIGHTER_H
