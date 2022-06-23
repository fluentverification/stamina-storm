#include "About.h"

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
}

void
About::show() {

}

void
About::hide() {

}

}
}
