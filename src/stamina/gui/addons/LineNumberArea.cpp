#include "LineNumberArea.h"

namespace stamina {
namespace gui {
namespace addons {

LineNumberArea::LineNumberArea(CodeEditor * editor)
	: QWidget(editor)
	, editor(editor)
{
	// Intentionally left empty
}

void
LineNumberArea::paintEvent(QPaintEvent * event) {
	editor->lineNumberAreaPaintEvent(event);
}

}
}
}
