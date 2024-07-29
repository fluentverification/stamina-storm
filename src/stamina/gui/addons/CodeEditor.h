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

/*
 * Code editor with line numbers--extends QPlainTextEdit ~~KDE's KTextEdit~~
 * */

#ifndef STAMINA_GUI_ADDONS_CODE_EDITOR_H
#define STAMINA_GUI_ADDONS_CODE_EDITOR_H

#include <cstdint>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPainter>
#include <QCompleter>
#include <QGuiApplication>

#include "highlighter/Highlighter.h"

namespace stamina {
	namespace gui {
		namespace addons {
			class CodeEditor : public QPlainTextEdit {
				Q_OBJECT
			public:
				CodeEditor(QWidget * parent = nullptr);

				void lineNumberAreaPaintEvent(QPaintEvent * event);
				uint16_t lineNumberAreaWidth();
				void setCompleter(QCompleter * completer);
				QCompleter * completer() const;
				void setTabWidth(int numChars);
				void addWordToModel(QString word);
				void setColorsFromScheme(highlighter::ColorScheme * colors);
				highlighter::ColorScheme * getColorsAsScheme();
				void refresh() { highlightCurrentLine(); }

				static void setIndent(QString idt) { indent = idt; }
				static QString getIndent() { return indent; }

				inline static QColor lineNumberAreaColor;
				inline static QColor lineColor = QGuiApplication::palette().color(QPalette::AlternateBase);// QColor("#FF2222");
			public slots:
				void changeIndent(bool increase = true);
				void changeComment(bool uncomment = false);
			protected:
				void resizeEvent(QResizeEvent * event) override;
				void keyPressEvent(QKeyEvent * e) override;
				void focusInEvent(QFocusEvent * e) override;
			private slots:
				void updateLineNumberAreaWidth(uint16_t newBlockCount);
				void highlightCurrentLine();
				void updateLineNumberArea(const QRect & rect, int16_t dy);
				void insertCompletion(const QString & completion);
				void indentNextLine();
			private:
				QString textUnderCursor() const;
				int getNextPosition(QString findPattern);

				int lastBlockCount = 0; // So we can see on block count changed whether we added or removed.
				QCompleter * c = nullptr;
				QWidget * lineNumberArea;
				highlighter::Highlighter * hl;
				inline static QString indent = "\t";
				inline static QString comment = "//";
				inline static QStringList indentables = {"module", "init", "invariant", "observables", "rewards", "system"};
			};
		}
	}
}
#endif // STAMINA_GUI_ADDONS_CODE_EDITOR_H
