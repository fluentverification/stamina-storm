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
	for (auto & category : categories) {
		QFormLayout * formLayout = new QFormLayout(tabs);
		for (auto & setting : category) {
			// Add setting
		}
		tabs->addWidget(formLayout);
	}
	this->addWidget(tabs);
}

} // namespace settings
} // namespace gui
} // namespace stamina
