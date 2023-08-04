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
#include <regex>
#include <filesystem>

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>
#include <KIO/Job>

#include "gui/MainWindow.h"

#include "core/Options.h"
#include "core/StaminaMessages.h"
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

namespace gui {

void
parse_positional_arguments(const QStringList & args, MainWindow * window) {
	if (args.size() > 0) {
		core::StaminaMessages::info("Opening model file: " + args[0].toStdString());
		window->setActiveModelFileName(args[0]);
		KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(args[0]));
		QApplication::connect(job, SIGNAL(result(KJob *)), window, SLOT(downloadFinishedModel(KJob*)));
		job->exec();
		std::string baseFileName = std::regex_replace(args[0].toStdString(), std::regex("\\.prism|\\.sm"), "");
		std::string propFileName = baseFileName + ".csl";
		if (args.size() > 1) {
			core::StaminaMessages::info("Opening property file: " + args[1].toStdString());
			window->setActivePropFileName(args[1]);
			KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(args[1]));
			QApplication::connect(job, SIGNAL(result(KJob *)), window, SLOT(downloadFinishedProperty(KJob*)));
			job->exec();
		}
		else if (std::filesystem::exists(propFileName)) {
			QString propFileQString = QString::fromStdString(propFileName);
			window->setActivePropFileName(propFileQString);
			core::StaminaMessages::info("Opening property file: " + propFileName);
			KIO::Job * job = KIO::storedGet(QUrl::fromLocalFile(propFileQString));
			QApplication::connect(job, SIGNAL(result(KJob *)), window, SLOT(downloadFinishedProperty(KJob*)));
			job->exec();
		}
	}
}

} // namespace gui

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
						 QStringLiteral("0.2.5"),
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
	parser.addPositionalArgument("model-file", "The model file in PRISM model definition syntax");
	parser.addPositionalArgument("properties-file", "The properties file in PRISM CSL or PCTL syntax");
	aboutData.setupCommandLine(&parser);
	parser.process(app);
	aboutData.processCommandLine(&parser);

	const QStringList args = parser.positionalArguments();

	stamina::gui::MainWindow * window = new stamina::gui::MainWindow();
	parse_positional_arguments(args, window);
	window->show();

	return app.exec();
}
