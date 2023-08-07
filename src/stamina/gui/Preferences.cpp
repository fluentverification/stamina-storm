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

#include "Preferences.h"

#include <QMessageBox>
#include <QSettings>

#include "stamina/StaminaArgParse.h"
#include "stamina/core/StaminaMessages.h"

namespace stamina {
namespace gui {

const std::string PrefInfo::prefPath = ".xstaminarc";

Preferences::Preferences(QWidget * parent)
	: QDialog(parent)
{
	setupActions();
	readSettingsFromFile();
}

void
Preferences::setupActions() {
	ui.setupUi(this);
	connect(ui.replaceAllIndentationButton, SIGNAL(clicked()), this, SLOT(replaceAllIndentation()));
}

void
Preferences::show(int tabIndex) {
	ui.prefTabs->setCurrentIndex(tabIndex);
	this->exec();
}

void
Preferences::accept() {
	getPreferencesFromUI();
	StaminaMessages::info("Updating accepted preferences");
	setOptionsFromPreferences();
	setUIFromPreferences();
	writeSettingsToFile();
	this->hide();
}

void
Preferences::setOptionsFromPreferences() {
	StaminaMessages::info("Setting options for model builder and checker.");
	// General options

	// ModelBuilding tab Options
	core::Options::kappa = PrefInfo::ModelBuilding::kappa;
	core::Options::reduce_kappa =PrefInfo:: ModelBuilding::rKappa;
	core::Options::prob_win = PrefInfo::ModelBuilding::window;
	core::Options::no_prop_refine = !PrefInfo::ModelBuilding::earlyTerminationProperty;
	// TODO: maxIterations
	core::Options::max_approx_count = PrefInfo::ModelBuilding::maxApproxIterations;
	if (PrefInfo::ModelBuilding::exportPerimeterStates && PrefInfo::ModelBuilding::perimeterStatesFile == "") {
		// TODO: Error
	}
	else if (PrefInfo::ModelBuilding::exportPerimeterStates) {
		core::Options::export_perimeter_states = PrefInfo::ModelBuilding::perimeterStatesFile;
	}
	core::Options::method = PrefInfo::ModelBuilding::truncationMethod;
	// core::Options::threads = PrefInfo::ModelBuilding::threads;
}

void
Preferences::setUIFromPreferences() {
	// Set the font first because the tab size uses the font size to determine
	// how many pixels the indent should be
	window->modelFile->setFont(PrefInfo::General::editorFont);
	window->propertiesEditor->setFont(PrefInfo::General::editorFont);
	window->modelFile->setTabWidth(PrefInfo::General::tabSize);
	window->propertiesEditor->setTabWidth(PrefInfo::General::tabSize);
	QString indentation = "";
	if (PrefInfo::General::useTabs) {
		indentation += "\t";
	}
	else {
		for (int i = 0; i < PrefInfo::General::tabSize; i++) {
			indentation += " ";
		}
	}
	// StaminaMessages::info("Setting indentation to '" + indentation.toStdString() + "' in CodeEditor");
	addons::CodeEditor::setIndent(indentation);
}

void
Preferences::getPreferencesFromUI() {
	// General Options
	PrefInfo::General::truncateModel = ui.truncateModel->isChecked();
	PrefInfo::General::generateCounterexamples = ui.genCounterexamples->isChecked();
	PrefInfo::General::createRefinedProperties = ui.createRefinedProperties->isChecked();
	PrefInfo::General::verboseLog = ui.verboseLog->isChecked();
	bool validFontSize;
	int fontSize = (int) ui.fontSizeComboBox->currentText().toInt(&validFontSize);
	if (validFontSize) {
		PrefInfo::General::editorFont = ui.fontComboBox->currentFont();
		PrefInfo::General::editorFont.setPointSize(fontSize);
	}
	else {
		// Text was not valid as an integer
		std::string msg = "The text \"" + ui.fontSizeComboBox->currentText().toStdString() + "\" could not be parsed to an integer!";
		StaminaMessages::error(msg);
		QMessageBox::critical(
			this
			, "Parsing Error"
			, QString::fromStdString(msg)
		);

	}
	bool validInteger;
	auto tSize = (uint8_t) ui.tabSize->currentText().toInt(&validInteger);
	if (validInteger) {
		PrefInfo::General::tabSize = tSize;
	}
	else {
		// Text was not valid as an integer
		std::string msg = "The text \"" + ui.tabSize->currentText().toStdString() + "\" could not be parsed to an integer!";
		StaminaMessages::error(msg);
		QMessageBox::critical(
			this
			, "Parsing Error"
			, QString::fromStdString(msg)
		);

	}
	PrefInfo::General::useTabs = ui.useTabs->isChecked();
	// Model Building options
	// These we will take in batches of related properties
	bool validKappa, validRKappa, validW;
	auto k = ui.kappa->text().toDouble(&validKappa);
	auto rk = ui.rKappa->text().toDouble(&validRKappa);
	auto w = ui.w->text().toDouble(&validW);
	if (validKappa && validRKappa && validW) {
		PrefInfo::ModelBuilding::kappa = k;
		PrefInfo::ModelBuilding::rKappa = rk;
		PrefInfo::ModelBuilding::window = w;
	}
	else {
		std::string msg = "Either kappa, rKappa, or the window cannot be parsed!";
		StaminaMessages::error(msg
			+ std::string("\nValid k: ") + (validKappa ? "true" : "false") + " (" + ui.kappa->text().toStdString() + ")"
			+ std::string("\nValid rk: ") + (validRKappa ? "true" : "false") + " (" + ui.rKappa->text().toStdString() + ")"
			+ std::string("\nValid w: ") + (validW ? "true" : "false") + " (" + ui.w->text().toStdString() + ")"
		);
		QMessageBox::critical(this, "Parsing Error", QString::fromStdString(msg));
	}
	PrefInfo::ModelBuilding::earlyTerminationProperty = ui.earlyTerminationProperty->isChecked();
	bool validMaxIters, validMaxApproxIters;
	auto mxItrs = ui.maxIterations->text().toInt(&validMaxIters);
	auto mxAprxItrs = ui.maxApproxIterations->text().toInt(&validMaxApproxIters);
	if (validMaxApproxIters && validMaxIters) {
		PrefInfo::ModelBuilding::maxIterations = (uint16_t) mxItrs;
		PrefInfo::ModelBuilding::maxApproxIterations = (uint8_t) mxAprxItrs;
	}
	else {
		std::string msg = "The max iteration count or the max approx iterations cannot be parsed";
		StaminaMessages::error(msg);
		QMessageBox::critical(this, "Parsing Error", QString::fromStdString(msg));
	}
	PrefInfo::ModelBuilding::exportTransitions = ui.traExport->isChecked();
	PrefInfo::ModelBuilding::transitionsFile = ui.traFilePath->text().toStdString();
	PrefInfo::ModelBuilding::exportPerimeterStates = ui.exportPerimeterStates->isChecked();
	PrefInfo::ModelBuilding::perimeterStatesFile = ui.perimPath->text().toStdString();
	if (ui.reExploring->isChecked()) {
		// set value
		PrefInfo::ModelBuilding::truncationMethod = STAMINA_METHODS::RE_EXPLORING_METHOD;
	}
	else if (ui.iterative->isChecked()) {
		// set value
		PrefInfo::ModelBuilding::truncationMethod = STAMINA_METHODS::ITERATIVE_METHOD;
	}
	else if (ui.priority->isChecked()) {
		// set value
		PrefInfo::ModelBuilding::truncationMethod = STAMINA_METHODS::PRIORITY_METHOD;
	}
// 	PrefInfo::ModelBuilding::thread = ui.numberThreads->value();
	// The ModelChecking tab
	// TODO
	// The Counterexamples tab
	// Add these when Counterexamples are supported in stamina
}

void
Preferences::readSettingsFromFile() {
	QSettings settings(QSettings::UserScope, "FLUENT Verification", "xSTAMINA");
	StaminaMessages::info("Reading preferences from: " + settings.fileName().toStdString());
	settings.beginGroup("General");
	getPreferencesFromUI();
	QString editorFontFamily = settings.value("editorFontFamily", PrefInfo::General::editorFont.family()).toString();
	int editorFontPoint = settings.value("editorFontSize", PrefInfo::General::editorFont.pointSize()).toInt();
	PrefInfo::General::editorFont.setFamily(editorFontFamily);
	PrefInfo::General::editorFont.setPointSize(editorFontPoint); // = QFont(editorFontFamily, editorFontPoint);
	ui.fontComboBox->setCurrentFont(PrefInfo::General::editorFont);
	ui.fontSizeComboBox->setCurrentText(QString::number(editorFontPoint));
	PrefInfo::General::tabSize = settings.value("tabSize", PrefInfo::General::tabSize).toInt();
	ui.tabSize->setCurrentText(QString::number(PrefInfo::General::tabSize));
	PrefInfo::General::useTabs = settings.value("useTabs", PrefInfo::General::useTabs) == "true";
	ui.useTabs->setChecked(PrefInfo::General::useTabs);
	settings.endGroup();
}

void
Preferences::writeSettingsToFile() {
	QSettings settings(QSettings::UserScope, "FLUENT Verification", "xSTAMINA");
	StaminaMessages::info("Writing preferences to: " + settings.fileName().toStdString());
	settings.beginGroup("General");
	settings.setValue("editorFontFamily", PrefInfo::General::editorFont.family());
	settings.setValue("editorFontSize", PrefInfo::General::editorFont.pointSize());
	settings.setValue("tabSize", PrefInfo::General::tabSize);
	settings.setValue("useTabs", PrefInfo::General::useTabs);
	settings.endGroup();
}

void
Preferences::replaceAllIndentation() {
	QString oldIndentation = addons::CodeEditor::getIndent();
	setUIFromPreferences();
	QString newIndent = addons::CodeEditor::getIndent();
	QTextCursor oldCursMod = window->modelFile->textCursor();
	window->modelFile->moveCursor(QTextCursor::Start);
	bool canReplaceMod = window->modelFile->find(oldIndentation);
	while (canReplaceMod) {
		QTextCursor curs = window->modelFile->textCursor();
		curs.removeSelectedText();
		curs.insertText(newIndent);
		canReplaceMod = window->modelFile->find(oldIndentation);
	}
	window->modelFile->setTextCursor(oldCursMod);

	QTextCursor oldCursProp = window->propertiesEditor->textCursor();
	window->propertiesEditor->moveCursor(QTextCursor::Start);
	bool canReplaceProp = window->propertiesEditor->find(oldIndentation);
	while (canReplaceProp) {
		QTextCursor curs = window->propertiesEditor->textCursor();
		curs.removeSelectedText();
		curs.insertText(newIndent);
		canReplaceProp = window->propertiesEditor->find(oldIndentation);
	}

	window->propertiesEditor->setTextCursor(oldCursProp);
}

} // namespace gui
} // namespace stamina
