#ifndef STAMINA_LINE_NUMBER_AREA_H
#define STAMINA_LINE_NUMBER_AREA_H

#include <QWidget>
#include <ktextedit.h>

#include "CodeEditor.h"

namespace stamina {
	namespace gui {
		namespace addons {
			class LineNumberArea : public QWidget {
				Q_OBJECT
			public:
				LineNumberArea(CodeEditor * editor);
				QSize sizeHint() const override  {
					return QSize(editor->lineNumberAreaWidth(), 0);
				}
			protected:
				void paintEvent(QPaintEvent * event) override;
			private:
				CodeEditor * editor;
			};
		}
	}
}
#endif // STAMINA_LINE_NUMBER_AREA_H
