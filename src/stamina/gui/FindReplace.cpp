#include "FindReplace.h"

#include <core/StaminaMessages.h>

#include <QRegularExpression>

namespace stamina {
namespace gui {

FindReplace::FindReplace(QWidget * parent)
	: QWidget(parent)
	, hostWidget(nullptr)
{
	/* Does not setup actions until later */
}

void
FindReplace::place(QVBoxLayout * location, QPlainTextEdit * editor) {
	this->hostWidget = new QWidget(this);
	this->ui.setupUi(hostWidget);
	if (!location || !editor) {
		StaminaMessages::error("Could not initialize find/replace! (Address: " + std::to_string((uint64_t) location) + ")");
	}
	else {
		location->addWidget(hostWidget);
		setupActions();
		// starts hidden
		hostWidget->hide();
		this->editor = editor;
	}
}

void
FindReplace::show(bool replace) {
	this->hostWidget->show();
	if (replace) {
		ui.replaceAllButton->show();
		ui.replaceButton->show();
		// ui.replaceSpacer->show();
		ui.replaceText->show();
	}
	else {
		ui.replaceAllButton->hide();
		ui.replaceButton->hide();
		// ui.replaceSpacer->hide();
		ui.replaceText->hide();
	}
}

void
FindReplace::setupActions() {
	connect(
		ui.closeButton
		, &QPushButton::clicked
		, this
		, [this] () {
			this->hostWidget->hide();
		}
	);

	connect(
		ui.findButton
		, SIGNAL(clicked())
		, this
		, SLOT(find())
	);

	connect(
		ui.findNextButton
		, SIGNAL(clicked())
		, this
		, SLOT(findNext())
	);

	connect(
		ui.replaceButton
		, SIGNAL(clicked())
		, this
		, SLOT(replace())
	);

	connect(
		ui.replaceAllButton
		, SIGNAL(clicked())
		, this
		, SLOT(replaceAll())
	);
}

void
FindReplace::find() {
	StaminaMessages::info("find() callled");
	// TODO: move cursor to the beginning
	findNext();
}

void
FindReplace::findNext() {
	StaminaMessages::info("findNext() callled");
	QString expression = ui.findExpression->text();
	bool useRegex = ui.findType->currentIndex() == 1;
	if (useRegex) {
		QRegularExpression regexp(expression);
		editor->find(regexp);
	}
	else {
		editor->find(expression);
	}

}

void
FindReplace::replace() {
	StaminaMessages::info("replace() callled");
}

void
FindReplace::replaceAll() {
	StaminaMessages::info("replaceAll() callled");
}

} // namespace gui
} // namespace stamina
