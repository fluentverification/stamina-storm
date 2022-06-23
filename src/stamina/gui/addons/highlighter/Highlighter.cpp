
#include "Highlighter.h"

#include <QRegularExpressionMatchIterator>

namespace stamina {
namespace gui {
namespace addons {
namespace highlighter {

void Highlighter::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
}

} // namespace highlighter
} // namespace addons
} // namespace gui
} // namespace stamina
