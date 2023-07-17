/*
 * Code editor with line numbers--extends QTextEdit ~~KDE's KTextEdit~~
 * */

#ifndef STAMINA_CODE_EDITOR_H
#define STAMINA_CODE_EDITOR_H

#include "ktextedit.h"
#include <cstdint>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QPainter>
#include <QCompleter>

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
			protected:
				void resizeEvent(QResizeEvent * event) override;
				void keyPressEvent(QKeyEvent * e) override;
				void focusInEvent(QFocusEvent * e) override;
			private slots:
				void updateLineNumberAreaWidth(uint16_t newBlockCount);
				void highlightCurrentLine();
				void updateLineNumberArea(const QRect & rect, int16_t dy);
				void insertCompletion(const QString & completion);
			private:
				QString textUnderCursor() const;

				QCompleter * c = nullptr;
				QWidget * lineNumberArea;
				highlighter::Highlighter * hl;
			};
		}
	}
}
#endif // STAMINA_CODE_EDITOR_H
