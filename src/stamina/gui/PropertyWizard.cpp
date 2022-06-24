#include "PropertyWizard.h"

namespace stamina {
namespace gui {

PropertyWizard::PropertyWizard(QWidget * parent)
: QDialog(parent)
{
	setupActions();
}

void
PropertyWizard::setupActions() {
	ui.setupUi(this);
}

void
PropertyWizard::insertOperand(QString opString, operandType_t opType) {

}
void
PropertyWizard::deleteSelectedOperand() {

}

} // namespace gui
} // namespace stamina
