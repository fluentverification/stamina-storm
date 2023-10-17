#include "Settings.h"

#include <core/StaminaMessages.h>

namespace stamina {
namespace gui {
namespace settings {

Settings::Settings() {
	createSettings();
	createUi();
}

void
Settings::createSettings() {
	// Creates a list of settings that can be modified

}

void
Settings::createUi() {
	// Adds the settings to the UI
	QTabWidget * tabs = new QTabWidget(this);
	int index = 0;
	for (auto & category : categories) {
		QFormLayout * formLayout = new QFormLayout(tabs);
		for (auto & setting : category) {
			// Add setting
			formLayout->addRow(
				setting.name
				, setting.createWidget(this)
			);
		}
		tabs->insertTab(index, formLayout, QLabel(category.name));
		index++;
	}
	this->addWidget(tabs);
}

QWidget *
Settings::Setting::createWidget(QWidget * parent) {
	switch (this->type) {
	case INTEGER:
		QSpinBox * spinBox = new QSpinBox(parent);
		bool okay;
		int value = this->get().toInt(&okay);
		if (okay) {
			spinBox->setValue(value);
		}
		else {
			core::StaminaMessages::error("Could not set value for setting: " + this->name.toStdString());
		}
		get = [spinBox]() { return spinBox->value(); }
		connect(
			spinBox
			, &QSpinBox::textChanged
			, parent
			, [this, spinBox]() {
				this->set(QVariant(spinBox->value()));
			}
		);
		return (QSpinBox *) spinBox;
	case DOUBLE:
		QLineEdit * doubleLineEdit = new QLineEdit(parent);
		get = [doubleLineEdit]() {
			QString text = doubleLineEdit->text();
			bool valid;
			double value = text.toDouble(&valid);
			if (valid) {
				return QVariant(value);
			}
			else {
				StaminaMessages::error("Cannot convert value to double!");
				return QVariant(0.0);
			}
		}
		connect(
			doubleLineEdit
			, &QLineEdit::editingFinished
			, parent
			, [this, doubleLineEdit]() {
				QString text = doubleLineEdit->text();
				bool valid;
				double value = text.toDouble(&valid);
				if (valid) {
					this->set(QVariant(value));
				}
				else {
					StaminaMessages::error("Cannot convert value to double!");
					bool storedValueIsGood;
					double storedValue = this->get().toDouble(&storedValueIsGood);
					if (!storedValueIsGood) { StaminaMessages::errorAndExit("Stored double value cannot be parsed!"); }
					doubleLineEdit->setValue(QString::fromDouble(storedValue));
				}
			}
		);
		return doubleLineEdit;
	case BOOLEAN:
		QCheckBox * checkBox = new QCheckBox(parent);
		bool value = this->get().toBool();
		checkBox->setCheckState(value ? Qt::Checked | Qt::Unchecked);
		get = [checkBox]() = { return checkBox->checkState() == Qt::Checked; }
		connect(
			checkBox
			, &QCheckBox::stateChanged
			, parent
			, [this, checkBox]() {
				this->set(QVariant(checkBox->checkState() == Qt::Checked));
			}
		);
		return (QWidget *) checkBox;
	case STRING:
		QLineEdit * lineEdit = new QLineEdit(parent);
		get = [lineEdit]() { return lineEdit->text(); }
		connect(
			lineEdit
			, &QLineEdit::editingFinished
			, parent
			, [this, lineEdit]() {
				QString text = lineEdit->text();
				this->set(QVariant(value));
			}
		);
		return (QWidget *) lineEdit;
	case FILENAME:
		QHBoxLayout * layout             = new QHBoxLayout(parent);
		QLineEdit   * fileNameLineEdit   = new QLineEdit(parent);
		QPushButton * fileSelectorButton = new QPushButton(parent, "...");
		layout->addWidget(fileNameLineEdit);
		layout->addWidget(fileSelectorButton);
		get = [fileNameLineEdit]() { return fileNameLineEdit->text(); }
		connect(
			fileSelectorButton
			, &QPushButton::clicked
			, parent
			, [fileNameLineEdit]() {
				QString fileName = QFileDialog::getOpenFileName(parent, tr("Open File"));
				if (fileName != "") {
					fileNameLineEdit->setText(fileName);
				}
				this->set(QVariant(fileNameLineEdit->text()));
			}
		);
		connect(
			fileNameLineEdit
			, &QLineEdit::editingFinished
			, parent
			, [this, fileNameLineEdit]() {
				this->set(QVariant(fileNameLineEdit->text()));
			}
		);
		return (QWidget *) layout;
	}
}

} // namespace settings
} // namespace gui
} // namespace stamina
