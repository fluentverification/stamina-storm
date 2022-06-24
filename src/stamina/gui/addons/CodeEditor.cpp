#include "CodeEditor.h"
#include "LineNumberArea.h"

#include "highlighter/PrismHighlighter.h"

#include <iostream>

#include <QPalette>


namespace stamina {
namespace gui {
namespace addons {

CodeEditor::CodeEditor(QWidget * parent)
	: QPlainTextEdit(parent)
	, hl(new highlighter::PrismHighlighter(this->document(), this->palette().color(QPalette::AlternateBase).black() > 100))
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
	connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
	connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

	updateLineNumberAreaWidth(0);
	highlightCurrentLine();

// 	std::cout << "Amount of black in color palette is: " << this->palette().color(QPalette::AlternateBase).black() << std::endl;
}

uint16_t
CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = std::max(1, blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}
	return 20 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void
CodeEditor::updateLineNumberAreaWidth(uint16_t newBlockCount) {
	setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void
CodeEditor::updateLineNumberArea(const QRect & rect, int16_t dy) {
	if (dy) {
		lineNumberArea->scroll(0, dy);
	}
	else {
		lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
	}
	if (rect.contains(viewport()->rect())) {
		updateLineNumberAreaWidth(0);
	}
}

void
CodeEditor::resizeEvent(QResizeEvent *e)
{
	QPlainTextEdit::resizeEvent(e);

	QRect cr = contentsRect();
	lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void
CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;

// 		QColor selectionColor = QPalette::Base; // selection.format.background().color();

		QColor lineColor(this->palette().color(QPalette::AlternateBase));

		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}

	setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), QColor(this->palette().color(QPalette::Window)).darker(100)); //  QColor(Qt::darkGray).darker(400)
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
	int bottom = top + qRound(blockBoundingRect(block).height());
	// Here is the key to obtain the y coordinate of the block start
	QTextCursor blockCursor(block);
	QRect blockCursorRect = this->cursorRect(blockCursor);
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(this->palette().color(QPalette::Text));
			painter.drawText(-5, top,
				lineNumberArea->width(), fontMetrics().height(),
				Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + qRound(blockBoundingRect(block).height());
		++blockNumber;
	}
}

}
}
}
