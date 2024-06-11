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
#include <filesystem>

#include "MainWindow.h"
#include "MessageBridge.h"
#include "addons/highlighter/Highlighter.h"
#include "addons/CodeEditor.h"
#include "stamina/StaminaArgParse.h"
#include "stamina/core/StaminaMessages.h"

namespace stamina {
namespace gui {

const std::string PrefInfo::prefPath = ".xstaminarc";

Preferences::Preferences(QWidget * parent, MainWindow * parentWrapper)
	: QDialog(parent)
	, window((Ui::MainWindow * ) parent)
	, windowWrapper(parentWrapper)
{
	setupActions();
	readSettingsFromFile();
	setupColorSchemes();
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
Preferences::preloadColors() {
	StaminaMessages::info("Reading color scheme");
	// ui.editorBaseColor->setColor(
	auto colorScheme = window->modelFile->getColorsAsScheme();
	if (colorScheme) {
		ui.keywordColor->setColor(colorScheme->keyword);
		ui.commentColor->setColor(colorScheme->comment);
		ui.numberColor->setColor(colorScheme->number);
		ui.typeColor->setColor(colorScheme->type);
		ui.functionColor->setColor(colorScheme->function);
		ui.stringColor->setColor(colorScheme->string);
		ui.constantsColor->setColor(colorScheme->constant);
		// colorScheme is not a QObject!
		delete colorScheme;
	}
	else {
		StaminaMessages::warning("Cannot set color scheme!");
	}
}

void
Preferences::setColorsFromPrefs() {
	addons::highlighter::ColorScheme colorScheme(
		ui.keywordColor->color()
		, ui.commentColor->color()
		, ui.numberColor->color()
		, ui.typeColor->color()
		, ui.functionColor->color()
		, ui.stringColor->color()
		, ui.constantsColor->color()
	);
	window->modelFile->setColorsFromScheme(&colorScheme);
	window->propertiesEditor->setColorsFromScheme(&colorScheme);
}

void
Preferences::accept() {
	getPreferencesFromUI();
	StaminaMessages::info("Updating accepted preferences");
	setOptionsFromPreferences();
	setUIFromPreferences();
	writeSettingsToFile();
	setColorsFromPrefs();
	this->hide();
}

void
Preferences::setOptionsFromPreferences() {
	StaminaMessages::info("Setting options for model builder and checker.");
	// General options

	// ModelBuilding tab Options
	core::Options::kappa = PrefInfo::ModelBuilding::kappa;
	core::Options::reduce_kappa = PrefInfo:: ModelBuilding::rKappa;
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
	if (!ui.useDesktopDefaults->isChecked()) {
		QString editorStylesheet = "background-color: " + ui.backgroundColor->color().name() + ";\ncolor: " + ui.foregroundColor->color().name() + ";";
		QString uiStylesheet = "background-color: " + ui.uiBackground->color().name() + ";\ncolor: " + ui.uiForeground->color().name() + ";";
		window->modelFile->setStyleSheet(editorStylesheet);
		window->propertiesEditor->setStyleSheet(editorStylesheet);
		windowWrapper->setStyleSheet(uiStylesheet);
		addons::CodeEditor::lineNumberAreaColor = ui.backgroundColor->color().darker(100);
		addons::CodeEditor::lineColor = ui.backgroundColor->color().lighter(100);
		window->modelFile->viewport()->repaint();
		window->propertiesEditor->viewport()->repaint();
	}
	else {
		window->modelFile->setStyleSheet("");
		window->propertiesEditor->setStyleSheet("");
		windowWrapper->setStyleSheet("");
		addons::CodeEditor::lineNumberAreaColor = QColor(this->palette().color(QPalette::Window)).darker(100);
		addons::CodeEditor::lineColor = this->palette().color(QPalette::AlternateBase);
		window->modelFile->viewport()->repaint();
		window->propertiesEditor->viewport()->repaint();

	}
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
	settings.beginGroup("SyntaxColors");
	ui.keywordColor->setColor(
		QColor(settings.value(
			"keywords"
			, ui.keywordColor->color().name()
		).toString())
	);
	ui.commentColor->setColor(
		QColor(settings.value(
			"comments"
			, ui.commentColor->color().name()
		).toString())
	);
	ui.numberColor->setColor(
		QColor(settings.value(
			"numbers"
			, ui.numberColor->color()
		).toString())
	);
	ui.typeColor->setColor(
		QColor(settings.value(
			"types"
			, ui.typeColor->color()
		).toString())
	);
	ui.functionColor->setColor(
		QColor(settings.value(
			"functions"
			, ui.functionColor->color()
		).toString())
	);
	ui.stringColor->setColor(
		QColor(settings.value(
			"strings"
			, ui.stringColor->color()
		).toString())
	);
	ui.constantsColor->setColor(
		QColor(settings.value(
			"constants"
			, ui.stringColor->color()
		).toString())
	);
	ui.foregroundColor->setColor(
		QColor(settings.value(
			"editorForegroundColor"
			, ui.foregroundColor->color()
		).toString())
	);
	ui.backgroundColor->setColor(
		QColor(settings.value(
			"editorBackgroundColor"
			, ui.backgroundColor->color()
		).toString())
	);
	ui.uiForeground->setColor(
		QColor(settings.value(
			"uiForegroundColor"
			, ui.uiForeground->color()
		).toString())
	);
	ui.uiBackground->setColor(
		QColor(settings.value(
			"uiBackgroundColor"
			, ui.uiBackground->color()
		).toString())
	);


	bool desktopDefaultColors = settings.value("useCustomEditorColors", true) == "true";
	ui.useDesktopDefaults->setChecked(desktopDefaultColors);
	ui.foregroundColor->setEnabled(!desktopDefaultColors);
	ui.backgroundColor->setEnabled(!desktopDefaultColors);
	ui.uiForeground->setEnabled(!desktopDefaultColors);
	ui.uiBackground->setEnabled(!desktopDefaultColors);
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
	settings.beginGroup("SyntaxColors");

	settings.setValue("keywords", ui.keywordColor->color().name());
	settings.setValue("comments", ui.commentColor->color().name());
	settings.setValue("numbers", ui.numberColor->color().name());
	settings.setValue("types", ui.typeColor->color().name());
	settings.setValue("functions", ui.functionColor->color().name());
	settings.setValue("strings", ui.stringColor->color().name());
	settings.setValue("constants", ui.constantsColor->color().name());
	settings.setValue("editorBackgroundColor", ui.backgroundColor->color());
	settings.setValue("editorForegroundColor", ui.foregroundColor->color());
	settings.setValue("uiForegroundColor", ui.uiForeground->color());
	settings.setValue("uiBackgroundColor", ui.uiBackground->color());
	settings.setValue("useCustomEditorColors", ui.useDesktopDefaults->isChecked());

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

void
Preferences::setupColorSchemes() {
	MessageBridge::info("Setting up color scheme options");
	// Make sure we're empty
	ui.themeComboBox->clear();
	// Last custom theme
	this->themes.push_back(
		std::make_pair(
			QString("Last Custom Settings")
			, addons::highlighter::ColorScheme(
				ui.keywordColor->color()
				, ui.commentColor->color()
				, ui.numberColor->color()
				, ui.typeColor->color()
				, ui.functionColor->color()
				, ui.stringColor->color()
				, ui.constantsColor->color()
			)
		)
	);

	// Colorful theme
	this->themes.push_back(
		std::make_pair(
			QString("Colorful")
			, addons::highlighter::ColorScheme(
				QColor("#00aaff")
				, QColor("#919191")
				, QColor("#ff4747")
				, QColor("#0090bc")
				, QColor("#629aa8")
				, QColor("#ff9040")
				, QColor("#3cbc00")
			)
		)
	);
	// Desaturated theme
	this->themes.push_back(
		std::make_pair(
			QString("Desaturated")
			, addons::highlighter::ColorScheme(
				QColor("#b4e6ff")
				, QColor("#919191")
				, QColor("#ffb4b4")
				, QColor("#85afbc")
				, QColor("#506a70")
				, QColor("#ffd3b4")
				, QColor("#97bc85")
			)
		)
	);

	uint8_t index;
	for (auto const & themePair : this->themes) {
		QString themeName = themePair.first;
		ui.themeComboBox->insertItem(index, themeName);
		index++;
	}

	connect(
		ui.themeComboBox
		, QOverload<int>::of(&QComboBox::currentIndexChanged)
		, this
		, [this](int index) {
			addons::highlighter::ColorScheme & theme = this->themes[index].second;
			MessageBridge::info(this->themes[index].first.toStdString() + " is new color scheme");
			// window->modelFile->setColorsFromScheme(&theme);
			// window->propertiesEditor->setColorsFromScheme(&theme);
			ui.keywordColor->setColor(theme.keyword);
			ui.commentColor->setColor(theme.comment);
			ui.numberColor->setColor(theme.number);
			ui.typeColor->setColor(theme.type);
			ui.functionColor->setColor(theme.function);
			ui.stringColor->setColor(theme.string);
			ui.constantsColor->setColor(theme.constant);
		}
	);

	connect(
		ui.useDesktopDefaults
		, &QCheckBox::clicked
		, this
		, [this](bool checked) {
			ui.foregroundColor->setEnabled(!checked);
			ui.backgroundColor->setEnabled(!checked);
			ui.uiForeground->setEnabled(!checked);
			ui.uiBackground->setEnabled(!checked);
		}
	);

}

} // namespace gui
} // namespace stamina
