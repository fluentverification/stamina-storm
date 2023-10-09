#include "Settings.h"

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
				, // TODO: create widget based on type
			);
		}
		tabs->insertTab(index, formLayout, QLabel(category.name));
		index++;
	}
	this->addWidget(tabs);
}

} // namespace settings
} // namespace gui
} // namespace stamina
