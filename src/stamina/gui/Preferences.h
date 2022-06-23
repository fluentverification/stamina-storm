#ifndef STAMINA_PREFERENCES_H
#define STAMINA_PREFERENCES_H

#include <KXmlGuiWindow>
#include <QDialog>

#include "ui/ui_Preferences.h"

namespace stamina {
	namespace gui {
		class Preferences : public QDialog {
			Q_OBJECT
		public:
			Preferences(QWidget * parent = 0);
			void show();
			void hide();
		private:
			void setupActions();
			// Data members
			Ui::Preferences ui;
		};
	}
}

#endif // STAMINA_PREFERENCES_H
