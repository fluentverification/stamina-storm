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

#include "MessageBridge.h"

#include <QString>

#include <core/StaminaMessages.h>
#include <ANSIColors.h>
#include <KMessageBox>

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
	StaminaMessages::criticalCallback = std::bind(
		&MessageBridge::critical
		, std::placeholders::_1
	);
	StaminaMessages::functionsSetup = true;
	StaminaMessages::raiseExceptionsRatherThanExit = true;
}

void
MessageBridge::error(std::string err) {
	if (logOutput) {
		logOutput->append("<b style='color: #c11a1a'>[ERROR]:</b>&nbsp;&nbsp;&nbsp;<span class=error>"
			+ sanitize(QString::fromStdString(err)) + "</span>");
	}
	if (statusBar) {
		statusBar->showMessage(QString::fromStdString(err));
	}
}

void
MessageBridge::warning(std::string warn) {
	if (logOutput) {
		logOutput->append("<b style='color: #c19718'>[WARNING]:</b>&nbsp;<span class=warning>"
			+ sanitize(QString::fromStdString(warn)) + "</span>");
	}
	if (statusBar) {
		statusBar->showMessage(QString::fromStdString(warn));
	}
}

void
MessageBridge::info(std::string info) {
	if (logOutput) {
		logOutput->append("<b style='color: #2073c1'>[INFO]:</b>&nbsp;&nbsp;&nbsp;&nbsp;<span class=info>"
			+ sanitize(QString::fromStdString(info)) + "</span>");
	}
	if (statusBar) {
		statusBar->showMessage(QString::fromStdString(info));
	}
}
void
MessageBridge::good(std::string good) {
	if (logOutput) {
		logOutput->append("<b style='color: #28c11d'>[MESSAGE]:</b>&nbsp;<span class=good>"
			+ sanitize(QString::fromStdString(good)) + "</span>");
	}
	if (statusBar) {
		statusBar->showMessage(QString::fromStdString(good));
	}
}

void
MessageBridge::critical(std::string crit) {
	error(crit);
	KMessageBox::error(nullptr, QString::fromStdString(crit));
	throw std::exception();
}

} // namespace gui
} // namespace stamina
