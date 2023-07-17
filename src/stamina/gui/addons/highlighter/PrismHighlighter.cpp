#include "PrismHighlighter.h"

#include <QStringLiteral>
#include <QRegularExpressionMatchIterator>
#include <QRegularExpression>
#include <QFont>
#include <QColor>
#include <QPalette>

#include <iostream>

namespace stamina {
namespace gui {
namespace addons {
namespace highlighter {

PrismHighlighter::PrismHighlighter(QTextDocument * parent, bool darkMode)
	: Highlighter(parent)
	, darkMode(darkMode)
{
	setupKeyWordPatterns();
// 	bool darkMode = parent()->palette().color(QPalette::AlternateBase).dark() < 100;
// 	std::cout << "Dark for this color scheme is " << this->palette().color(QPalette::AlternateBase).dark() << std::endl;
}

void
PrismHighlighter::setupKeyWordPatterns() {
	ColorScheme * cs;
	if (darkMode) {
		cs = &ColorSchemes::darkMode;
	}
	else {
		cs = &ColorSchemes::lightMode;
	}
	HighlightingRule rule;

	keywordFormat.setForeground(cs->keyword);
	keywordFormat.setFontWeight(QFont::Bold);
	const QString keywordPatterns[] = {
		QStringLiteral("\\bA\\b")
		, QStringLiteral("\\bclock\\b")
		, QStringLiteral("\\bconst\\b")
		, QStringLiteral("\\bctmc\\b")
		, QStringLiteral("\\bC\\b")
		, QStringLiteral("\\bdtmc\\b")
		, QStringLiteral("\\bE\\b")
		, QStringLiteral("\\bendinit\\b")
		, QStringLiteral("\\bendinvariant\\b")
		, QStringLiteral("\\bendmodule\\b")
		, QStringLiteral("\\bendobservables\\b")
		, QStringLiteral("\\bendrewards\\b")
		, QStringLiteral("\\bendsystem\\b")
		, QStringLiteral("\\bfalse\\b")
		, QStringLiteral("\\bfilter\\b")
		, QStringLiteral("\\bfunc\\b")
		, QStringLiteral("\\bF\\b")
		, QStringLiteral("\\bglobal\\b")
		, QStringLiteral("\\bG\\b")
		, QStringLiteral("\\binit\\b")
		, QStringLiteral("\\bI\\b")
		, QStringLiteral("\\bmax\\b")
		, QStringLiteral("\\bmdp\\b")
		, QStringLiteral("\\bmin\\b")
		, QStringLiteral("\\bmodule\\b")
		, QStringLiteral("\\bX\\b")
		, QStringLiteral("\\bnondeterministic\\b")
		, QStringLiteral("\\bobservable\\b")
		, QStringLiteral("\\bobservables\\b")
		, QStringLiteral("\\bof\\b")
		, QStringLiteral("\\bPmax\\b")
		, QStringLiteral("\\bPmin\\b")
		, QStringLiteral("\\bP\\b")
		, QStringLiteral("\\bpomdp\\b")
		, QStringLiteral("\\bpopta\\b")
		, QStringLiteral("\\bprobabilistic\\b")
		, QStringLiteral("\\bprob\\b")
		, QStringLiteral("\\bpta\\b")
		, QStringLiteral("\\brate\\b")
		, QStringLiteral("\\brewards\\b")
		, QStringLiteral("\\bRmax\\b")
		, QStringLiteral("\\bRmin\\b")
		, QStringLiteral("\\bR\\b")
		, QStringLiteral("\\bS\\b")
		, QStringLiteral("\\bstochastic\\b")
		, QStringLiteral("\\bsystem\\b")
		, QStringLiteral("\\btrue\\b")
		, QStringLiteral("\\bU\\b")
		, QStringLiteral("\\bW\\b")
	};

	typeFormat.setForeground(cs->type);
	typeFormat.setFontWeight(QFont::Bold);
	typeFormat.setFontItalic(true);
	const QString typePatterns[] = {
		// Primitive types
		QStringLiteral("\\bbool\\b")
		, QStringLiteral("\\bdouble\\b")
		, QStringLiteral("\\bint\\b")
		// PRISM types
		, QStringLiteral("\\binvariant\\b")
		, QStringLiteral("\\bformula\\b")
		, QStringLiteral("\\blabel\\b")

	};

	// String expressions
	classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(cs->string);
	rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
	rule.format = classFormat;
	highlightingRules.append(rule);

	// Numbers
	numberFormat.setFontWeight(QFont::Bold);
	numberFormat.setForeground(cs->number);
	rule.pattern = QRegularExpression(QStringLiteral("\\b([eE\\.]?\\d+)"));
	rule.format = numberFormat;
	highlightingRules.append(rule);

	// String literals
	quotationFormat.setForeground(cs->string);
	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	// Functions
	functionFormat.setFontItalic(true);
	functionFormat.setForeground(cs->function);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	rule.format = functionFormat;
	highlightingRules.append(rule);

	// Constants
	constFormat.setFontItalic(true);
	constFormat.setForeground(cs->constant);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Z_]+\\b"));
	rule.format = constFormat;
	highlightingRules.append(rule);

	// Keywords have highest priority, with the exception of comments and types
	for (const QString &pattern : keywordPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	// Types have less priority than comments
	for (const QString &pattern : typePatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = typeFormat;
		highlightingRules.append(rule);
	}

	// Single line comments
	singleLineCommentFormat.setForeground(cs->comment);
	rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	// Multiline comments
	multiLineCommentFormat.setForeground(cs->comment);

	commentStartExpression = QRegularExpression(QStringLiteral("/\\*"));
	commentEndExpression = QRegularExpression(QStringLiteral("\\*/"));
}


} // namespace highlighter
} // namespace addons
} // namespace gui
} // namespace stamina
