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
			protected:
				void resizeEvent(QResizeEvent * event) override;
			private slots:
				void updateLineNumberAreaWidth(uint16_t newBlockCount);
				void highlightCurrentLine();
				void updateLineNumberArea(const QRect & rect, int16_t dy);

			private:
				QWidget * lineNumberArea;
				highlighter::Highlighter * hl;
			};
		}
	}
}
#endif // STAMINA_CODE_EDITOR_H
