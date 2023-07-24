/**
 * A simple bridge class to let StaminaMessages append text to
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
		};
	} // namespace gui
} // namespace stamina

#endif // STAMINA_GUI_MESSAGEBRIDGE
