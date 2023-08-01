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

#include "CodeEditor.h"
#include "LineNumberArea.h"

#include "highlighter/PrismHighlighter.h"

#include <core/StaminaMessages.h>

#include <iostream>

#include <QPalette>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>

#define END_OF_DOCUMENT 99999999

namespace stamina {
namespace gui {
namespace addons {

CodeEditor::CodeEditor(QWidget * parent)
	: QPlainTextEdit(parent)
	, hl(new highlighter::PrismHighlighter(this->document(), this->palette().color(QPalette::AlternateBase).black() > 100))
{
	lineNumberArea = new LineNumberArea(this);

	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
	connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::indentNextLine);
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
CodeEditor::setTabWidth(int numChars) {
	QFontMetrics fm(this->font());
	qreal width = numChars * fm.horizontalAdvance(QChar::Nbsp);
	this->setTabStopDistance(width);
}

void
CodeEditor::addWordToModel(QString word) {
	if (!c) {
		StaminaMessages::warning("Cannot add word to model when no completer exists!");
		return;
	}
	QStringListModel * model = qobject_cast<QStringListModel*>(c->model());
	if (model) {
		bool matched = false;
		for (QString w : model->stringList()) {
			if (w == word) {
				matched = true;
				break;
			}
		}
		if (!matched) {
			StaminaMessages::info("Adding word to model: " + word.toStdString());
			model->setStringList(model->stringList() << word);
		}
	}
	else {
		StaminaMessages::warning("Cast to QStringListModel * failed! (got NULL)");
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
	else {
		bool cursorHasSelection = textCursor().hasSelection();
		if (cursorHasSelection && e->key() == Qt::Key_Tab) {
			e->ignore();
			changeIndent();
			return;
		}
		else if (e->key() == Qt::Key_Backtab) {
			e->ignore();
			changeIndent(false);
			return;
		}
		// Detect Ctl + / Shortcut
		else if (// Control key is pressed
			e->modifiers().testFlag(Qt::ControlModifier)
			// and '/' key is pressed
			&& e->key() == Qt::Key_Slash) {
			const bool uncomment = e->modifiers().testFlag(Qt::ShiftModifier);
			StaminaMessages::info("Commenting/uncommenting");
			changeComment(uncomment);
			return;
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

void
CodeEditor::changeIndent(bool increase) {
	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::LineUnderCursor);
	}

	// Get the start and end position
	// anchor() is the start of selection and position() is the end
	int startPos = cursor.anchor();
	int endPos = cursor.position();
	if (startPos > endPos) {
		std::swap(startPos, endPos);
	}

	// Get the start and end blocks
	cursor.setPosition(startPos, QTextCursor::MoveAnchor);
	int startBlock = cursor.block().blockNumber();
	cursor.setPosition(endPos, QTextCursor::MoveAnchor);
	int endBlock = cursor.block().blockNumber();

	// Do indent
	int blockDiff = endBlock - startBlock;

	cursor.setPosition(startPos, QTextCursor::MoveAnchor);
	cursor.beginEditBlock();

	for (int i = 0; i <= blockDiff; i++) {
		cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		if (increase) {
			cursor.insertText(CodeEditor::indent);
		}
		else {
			for (int j = 0; j < CodeEditor::indent.size(); j++) {
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
			}
			if (cursor.selectedText() == CodeEditor::indent) {
				cursor.removeSelectedText();
			}
		}
		cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
	}

	cursor.endEditBlock();
	this->setFocus(Qt::TabFocusReason);
}

void
CodeEditor::changeComment(bool uncomment) {
	QTextCursor cursor = textCursor();
	if (!cursor.hasSelection()) {
		cursor.select(QTextCursor::LineUnderCursor);
	}

	int startPos = cursor.anchor();
	int endPos = cursor.position();
	if (startPos > endPos) {
		std::swap(startPos, endPos);
	}

	// Get the start and end blocks
	cursor.setPosition(startPos, QTextCursor::MoveAnchor);
	int startBlock = cursor.block().blockNumber();
	cursor.setPosition(endPos, QTextCursor::MoveAnchor);
	int endBlock = cursor.block().blockNumber();

	// Do comment
	int blockDiff = endBlock - startBlock;

	cursor.setPosition(startPos, QTextCursor::MoveAnchor);
	cursor.beginEditBlock();

	for (int i = 0; i <= blockDiff; i++) {
		cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
		if (!uncomment) {
			cursor.insertText(CodeEditor::comment);
		}
		else {
			for (int j = 0; j < CodeEditor::comment.size(); j++) {
				cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
			}
			if (cursor.selectedText() == CodeEditor::comment) {
				cursor.removeSelectedText();
			}
		}
		cursor.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
	}

	cursor.endEditBlock();
	this->setFocus(Qt::ShortcutFocusReason);
}

void
CodeEditor::indentNextLine() {
	int bc = blockCount();
	if (bc < lastBlockCount) {
		lastBlockCount = blockCount();
		return;
	}
	lastBlockCount = blockCount();

	QTextCursor cursor = textCursor();

	// Get previous line and count of indentation at the beginning of it
	int indentationCount = 0;
	QTextBlock before = cursor.block().previous();
	QString lastLine = before.text();
	while (lastLine.startsWith(CodeEditor::indent)) {
		indentationCount++;
		lastLine.remove(0, CodeEditor::indent.size());
	}

	// Indent next line if line starts with 'module' or another indentable
	QTextBlockFormat format;
	for (QString & indentable : CodeEditor::indentables) {
		if (lastLine.startsWith(indentable)) {
			// we can insert 'endmodule' or the other end+indentable here too
			int cursorPosition = cursor.position();
			QString endIndentable = "end" + indentable;
			int nextEndPos = getNextPosition(endIndentable);
			int nextIndentPos = getNextPosition(indentable);
			// If the next instance of `module` is sooner than the next instance of endmodule
			if (nextIndentPos < nextEndPos || nextEndPos == END_OF_DOCUMENT) {
				StaminaMessages::info("Line starts with: "
					+ lastLine.toStdString() + " so inserting end (positions: "
					+ std::to_string(nextIndentPos) + " / " + std::to_string(nextEndPos) + ")");
				// Insert indentation
				cursor.insertBlock(format);
				for (int i = 0; i < indentationCount; i++) {
					cursor.insertText(CodeEditor::indent);
				}
				cursor.insertText(endIndentable);
				cursor.setPosition(cursorPosition);
			}
			indentationCount++;
			break;
		}
	}
	// cursor.insertBlock(format);
	// Insert indentation
	for (int i = 0; i < indentationCount; i++) {
		cursor.insertText(CodeEditor::indent);
	}

	this->setTextCursor(cursor);
}

int
CodeEditor::getNextPosition(QString findPattern) {
	QTextCursor cursor = textCursor();
	int position = cursor.position();
	bool found = this->find(findPattern);
	int nextPosition = found ? cursor.position() : END_OF_DOCUMENT;
	cursor.setPosition(position);
	this->setTextCursor(cursor);
	return nextPosition;
}

} // namespace addons
} // namespace gui
} // namespace stamina
