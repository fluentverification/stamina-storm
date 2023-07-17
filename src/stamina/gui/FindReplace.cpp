#include "FindReplace.h"

#include <core/StaminaMessages.h>

#include <KMessageBox>

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

	// When return is pressed, the findExpression textbox will call findNext
	connect(
		ui.findExpression
		, SIGNAL(returnPressed())
		, this
		, SLOT(findNext())
	);

	// For the replaceText textbox, we will call replace() (duh)
	connect(
		ui.replaceText
		, SIGNAL(returnPressed())
		, this
		, SLOT(replace())
	);
}

void
FindReplace::focusFind() {
	ui.findExpression->setFocus();
}

void
FindReplace::find() {
	StaminaMessages::info("find() callled");
	// TODO: move cursor to the beginning
	findNext();
}

bool
FindReplace::findNext(bool alertIfNotFound) {
	StaminaMessages::info("findNext() callled");
	QString expression = ui.findExpression->text();
	bool useRegex = ui.findType->currentIndex() == 1;
	if (useRegex) {
		QRegularExpression regexp(expression);
		editor->find(regexp);
	}
	else {
		bool found = editor->find(expression);
		if (alertIfNotFound && !found) {
			bool fromBeginning = KMessageBox::questionYesNo(nullptr, "The text could not be found! Search from the beginning?");
			if (fromBeginning) {
				editor->moveCursor(QTextCursor::Start);
				findNext();
			}
		}
		return found;
	}

}

void
FindReplace::replace() {
	StaminaMessages::info("replace() callled");
	bool canReplace = findNext(false);
	if (canReplace) {
		QTextCursor tc = editor->textCursor();
		QString replaceValue = ui.replaceText->text();
		tc.select(QTextCursor::WordUnderCursor);
		tc.removeSelectedText();
		tc.insertText(replaceValue);
	}
	else {
		bool fromBeginning = KMessageBox::questionYesNo(nullptr, "Cannot replace text! The text could not be found! Search from beginning?");
		if (fromBeginning) {
			editor->moveCursor(QTextCursor::Start);
			replace();
		}
	}
}

void
FindReplace::replaceAll() {
	StaminaMessages::info("replaceAll() callled");
	QString expression = ui.findExpression->text();
	QString replaceValue = ui.replaceText->text();
	bool useRegex = ui.findType->currentIndex() == 1;
	QString oldTxt = editor->toPlainText();
	if (useRegex) {
		QRegularExpression regexp(expression);
		QString newTxt = oldTxt.replace(regexp, replaceValue);
		editor->setPlainText(newTxt);
	}
	else {
		QString newTxt = oldTxt.replace(expression, replaceValue);
		editor->setPlainText(newTxt);
	}
}

} // namespace gui
} // namespace stamina
