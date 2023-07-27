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

/**
 * A simple bridge class to let StaminaMessages append text to the log viewer
 * in the GUI.
 * */
#ifndef STAMINA_GUI_MESSAGEBRIDGE_H
#define STAMINA_GUI_MESSAGEBRIDGE_H

#include "MainWindow.h"

#include <string>

namespace stamina {
	namespace gui {
		class MessageBridge {
		public:
			inline static QTextEdit * logOutput = nullptr;
			static void initMessageBridge();
			static void error(std::string err);
			static void warning(std::string warn);
			static void info(std::string info);
			static void good(std::string good);
			// This one also displays a messagebox
			static void critical(std::string crit);
		};
	} // namespace gui
} // namespace stamina

#endif // STAMINA_GUI_MESSAGEBRIDGE
