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

#ifndef STAMINA_GUI_SETTINGS_SETTINGS_H
#define STAMINA_GUI_SETTINGS_SETTINGS_H

#include <QVariant>
#include <QString>
#include <QSet>

#include <functional>

namespace stamina {
	namespace gui {
		namespace settings {
			class Settings : QWidget {
				Q_OBJECT
			public:
				/**
				* An individual setting in the list
				* */
				class Setting {
				public:
					/**
					 * Constructor. Creates a setting
					 *
					 * @param name The name of the setting
					 * @param description A description of the setting
					 * @param get Function pointer to getter of the method
					 * @param set Function pointer to setter of the method
					 * */
					Setting(
						QString name
						, QString description
						, std::function<QVariant(void)> get
						, std::function<void(QVariant)> set
					) : name(name)
						, description(description)
						, get(get)
						, set(set)
					{  /* Intentionally left empty */ }
					QString name;
					QString description;
					/**
					 * Allows runtime definition of getter and setter.
					 * */
					std::function<QVariant(void)> get;
					std::function<void(QVariant)> set;
				};
				class Category {
				public:
					Category(QString name)
						: name(name)
					{ /* Intentionally left empty */ }
					QString name;
					QSet<Setting> settings;
				};
				/**
				 * Constructor. Sets up UI
				 * */
				Settings();
				virtual void createSettings();
				void createUI();

				QSet<Category> categories;
			};
		}
	}
}

#endif // STAMINA_GUI_SETTINGS_SETTINGS_H
