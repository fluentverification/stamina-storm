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
		return /* TODO */;
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
		return (QSpinBox *) checkBox;
	case STRING:
		QLineEdit * lineEdit = new QLineEdit(parent);
	case FILENAME:
		// TODO
	}
}

} // namespace settings
} // namespace gui
} // namespace stamina
