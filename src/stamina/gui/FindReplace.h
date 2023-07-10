#ifndef STAMINA_FIND_REPLACE_H
#define STAMINA_FIND_REPLACE_H

#include "ui/ui_FindReplace.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>

namespace stamina {
	namespace gui {
		class FindReplace : public QWidget {
			Q_OBJECT
		public:
			FindReplace(QWidget * parent = nullptr);
			void place(QVBoxLayout * location = nullptr, QPlainTextEdit * editor = nullptr);
			void show(bool replace = false);
		protected:
			void setupActions();
			Ui::FindReplace ui;
			QWidget * hostWidget;
			QPlainTextEdit * editor;
		private slots:
			void find();
			void findNext();
			void replace();
			void replaceAll();
		};
	}
}

#endif // STAMINA_FIND_REPLACE_H
