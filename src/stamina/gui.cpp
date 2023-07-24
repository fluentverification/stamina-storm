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

#include <cstdlib>

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include "gui/MainWindow.h"

#include "core/Options.h"
#include "StaminaArgParse.h"

namespace stamina {

/**
 * Sets our default values
 * */
void
set_default_values() {
	core::Options::kappa = 1.0;
	core::Options::reduce_kappa = 1.25; // 2.0;
	core::Options::approx_factor = 2.0;
	core::Options::fudge_factor = 1.0;
	core::Options::prob_win = 1.0e-3;
	core::Options::max_approx_count = 10;
	core::Options::no_prop_refine = false;
	core::Options::cudd_max_mem = "1g";
	core::Options::export_trans = "";
	core::Options::rank_transitions = false;
	core::Options::max_iterations = 10000;
	core::Options::method = STAMINA_METHODS::ITERATIVE_METHOD;
	core::Options::threads = 1;
	core::Options::preterminate = false;
	core::Options::event = EVENTS::UNDEFINED;
	core::Options::distance_weight = 1.0;
	core::Options::quiet = false;
}

} // namespace stamina

int main (int argc, char *argv[])
{
	stamina::set_default_values();
	QApplication app(argc, argv);

	KLocalizedString::setApplicationDomain("stamina");

	KAboutData aboutData(
						 // The program name used internally. (componentName)
						 QStringLiteral("stamina"),
						 // A displayable program name string. (displayName)
						 i18n("STAMINA"),
						 // The program version string. (version)
						 QStringLiteral("2.5"),
						 // Short description of what the app does. (shortDescription)
						 i18n("Infinite state space truncation tool")
						 // The license this code is released under
						 , KAboutLicense::GPL,
						 // Copyright Statement (copyrightStatement = QString())
						 i18n("(c) 2022"),
						 // Optional text shown in the About box.
						 // Can contain any information desired. (otherText)
						 i18n("Developed at Utah State University"),
						 // The program homepage string. (homePageAddress = QString())
						 QStringLiteral("https://staminachecker.org/"),
						 // The bug report email address
						 QStringLiteral("[email protected]"));
	aboutData.addAuthor(i18n("Joshua Jeppson"), i18n("Principle Developer"), QStringLiteral("[email protected]"),
						QStringLiteral("https://ifndefjosh.github.io"), QStringLiteral("OSC Username"));
	KAboutData::setApplicationData(aboutData);

	QCommandLineParser parser;
	parser.addHelpOption();
	parser.addVersionOption();
	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	stamina::gui::MainWindow * window = new stamina::gui::MainWindow();
	window->show();

	return app.exec();
}
