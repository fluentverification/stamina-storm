#include "MessageBridge.h"

#include <QString>

#include <core/StaminaMessages.h>
#include <ANSIColors.h>

namespace stamina {
namespace gui {

using namespace stamina::core;

QString sanitize(QString input) {
	return input.replace("<", "&lt;")
			.replace(">", "&gt;")
			.replace("\x1B[1m", "")
			.replace(KMAG, "")
			.replace(RST, "")
			.replace("\n", "<br>")
			.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
}

void
MessageBridge::initMessageBridge() {
	StaminaMessages::errCallback = std::bind(
		&MessageBridge::error
		, std::placeholders::_1
	);
	StaminaMessages::warnCallback =	std::bind(
		&MessageBridge::warning
		, std::placeholders::_1
	);
	StaminaMessages::infoCallback = std::bind(
		&MessageBridge::info
		, std::placeholders::_1
	);
	StaminaMessages::goodCallback = std::bind(
		&MessageBridge::good
		, std::placeholders::_1
	);
	StaminaMessages::functionsSetup = true;
}

void
MessageBridge::error(std::string err) {
	if (logOutput) {
		logOutput->append("<b style='color: #c11a1a'>[ERROR]:</b>&nbsp;&nbsp;&nbsp;<span class=error>"
			+ sanitize(QString::fromStdString(err)) + "</span>");
	}
}

void
MessageBridge::warning(std::string warn) {
	if (logOutput) {
		logOutput->append("<b style='color: #c19718'>[WARNING]:</b>&nbsp;<span class=warning>"
			+ sanitize(QString::fromStdString(warn)) + "</span>");
	}
}

void
MessageBridge::info(std::string info) {
	if (logOutput) {
		logOutput->append("<b style='color: #2073c1'>[INFO]:</b>&nbsp;&nbsp;&nbsp;&nbsp;<span class=info>"
			+ sanitize(QString::fromStdString(info)) + "</span>");
	}
}
void
MessageBridge::good(std::string good) {
	if (logOutput) {
		logOutput->append("<b style='color: #28c11d'>[MESSAGE]:</b>&nbsp;<span class=good>"
			+ sanitize(QString::fromStdString(good)) + "</span>");
	}
}

} // namespace gui
} // namespace stamina
