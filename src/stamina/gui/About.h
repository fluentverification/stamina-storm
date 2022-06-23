#ifndef STAMINA_ABOUT_H
#define STAMINA_ABOUT_H

#include <KXmlGuiWindow>
#include <QDialog>

#include "ui/ui_About.h"

namespace stamina {
	namespace gui {
		class About : public QDialog {
			Q_OBJECT
		public:
			About(QWidget * parent = 0);
			void show();
			void hide();
		private:
			void setupActions();
			// Data members
			Ui::About ui;
		};
	}
}

#endif // STAMINA_ABOUT_H
