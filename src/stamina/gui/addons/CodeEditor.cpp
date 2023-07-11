#include "CodeEditor.h"
#include "LineNumberArea.h"

#include "highlighter/PrismHighlighter.h"

#include <core/StaminaMessages.h>

#include <iostream>

#include <QPalette>
#include <QAbstractItemView>
#include <QScrollBar>

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
CodeEditor::setCompleter(QCompleter * completer) {
	if (c) {
		c->disconnect(this);
	}
	c = completer;
	if (!completer) {
		return;
	}
	completer->setWidget(this);
	completer->setCompletionMode(QCompleter::PopupCompletion);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	QObject::connect(
		completer
		, QOverload<const QString &>::of(&QCompleter::activated)
		, this
		, &CodeEditor::insertCompletion
	);
}

QCompleter *
CodeEditor::completer() const {
	return c;
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

void
CodeEditor::insertCompletion(const QString & completion) {
	if (c->widget() != this) { return; }
	QTextCursor tc = textCursor();
	int remaining = completion.length() - c->completionPrefix().length();
	tc.movePosition(QTextCursor::Left);
	tc.movePosition(QTextCursor::EndOfWord);
	tc.insertText(completion.right(remaining));
	setTextCursor(tc);
}

QString
CodeEditor::textUnderCursor() const {
	QTextCursor tc = textCursor();
	tc.select(QTextCursor::WordUnderCursor);
	return tc.selectedText();
}

void
CodeEditor::focusInEvent(QFocusEvent * e) {
	if (c) {
		c->setWidget(this);
	}
	QPlainTextEdit::focusInEvent(e);
}

void
CodeEditor::keyPressEvent(QKeyEvent * e) {
	if (c && c->popup()->isVisible()) {
		// Use these keys for the completer
		switch (e->key()) {
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_Escape:
		case Qt::Key_Tab:
		case Qt::Key_Backtab:
			e->ignore();
			return;
		default:
			break;
		}
	}

	// Detect Ctl + E shortcut
	const bool isShortcut = (
		// Control key is pressed
		e->modifiers().testFlag(Qt::ControlModifier)
		// And E key is pressed
		&& e->key() == Qt::Key_E
	);

	if (!c || !isShortcut) {
		// Forward onto the super class
		QPlainTextEdit::keyPressEvent(e);
	}

	// Other events
	const bool ctlOrShift = e->modifiers().testFlag(Qt::ControlModifier)
						|| e->modifiers().testFlag(Qt::ShiftModifier);

	if (!c || (ctlOrShift && e->text().isEmpty())) { return; }

	// Test if end of word
	static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
	const bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctlOrShift;

	// Get prefix for completion
	QString completionPrefix = textUnderCursor();
	if (!isShortcut && (
			hasModifier // If there is a modifier
			|| e->text().isEmpty() // Or there is no input
			|| completionPrefix.length() < 2 // Or the prefix has a length less than 2
			|| eow.contains(e->text().right(1)) // Or we are at EOW
		)
	) {
		// Hide the popup
		c->popup()->hide();
		return;
	}

	if (completionPrefix != c->completionPrefix()) {
		c->setCompletionPrefix(completionPrefix);
		c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
	}

	QRect cr = cursorRect();
	cr.setWidth(c->popup()->sizeHintForColumn(0)
			+ c->popup()->verticalScrollBar()->sizeHint().width());
	// Show completor popup
	c->complete(cr);
}

} // namespace addons
} // namespace gui
} // namespace stamina
