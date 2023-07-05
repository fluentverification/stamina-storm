#include <cstdlib>

#include <QApplication>
#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include "gui/MainWindow.h"

int main (int argc, char *argv[])
{
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
