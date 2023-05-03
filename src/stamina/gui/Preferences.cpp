#include "Preferences.h"

namespace stamina {
namespace gui {

const std::string PrefInfo::prefPath = ".xstaminarc";

Preferences::Preferences(QWidget *parent)
	: QDialog(parent)
{
	setupActions();
}

void
Preferences::setupActions() {
	ui.setupUi(this);
}

void
Preferences::show() {

}

void
Preferences::hide() {

}

}
}
