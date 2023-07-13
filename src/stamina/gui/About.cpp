#include "About.h"

#include <QUrl>
#include <QDesktopServices>

namespace stamina {
namespace gui {

About::About(QWidget *parent)
	: QDialog(parent)
{
	setupActions();
}

void
About::setupActions() {
	ui.setupUi(this);
	/* connect(
		ui.homepageLink
		, &KUrlLabel::leftClickedUrl
		, this
		, [this] () { QDesktopServices::openUrl(QUrl("https://staminachecker.org")); }
	); */
	connect(
		ui.usageDocs
		, &QPushButton::clicked
		, this
		, [this] () { QDesktopServices::openUrl(QUrl("https://staminachecker.org/wiki")); }
	);
	connect(
		ui.apiDocs
		, &QPushButton::clicked
		, this
		, [this] () { QDesktopServices::openUrl(QUrl("https://staminachecker.org/documentation/stamina-storm/html/index.html")); }
	);
	connect(
		ui.caseStudies
		, &QPushButton::clicked
		, this
		, [this] () { QDesktopServices::openUrl(QUrl("https://staminachecker.org/results/qest23")); }
	);

}

void
About::show() {
	this->exec();
}

void
About::hide() {
// 	ui.hide();
}

}
}
