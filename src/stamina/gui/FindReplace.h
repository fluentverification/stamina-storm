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
			void focusFind();
		protected:
			void setupActions();
			Ui::FindReplace ui;
			QWidget * hostWidget;
			QPlainTextEdit * editor;
		private slots:
			void find();
			bool findNext(bool alertIfNotFound=true);
			void replace();
			void replaceAll();
		};
	}
}

#endif // STAMINA_FIND_REPLACE_H
