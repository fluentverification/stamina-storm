#include "PropertyWizard.h"

#include <iostream>

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

	connect(
		ui.expressionType
		, SIGNAL(clicked())
		, this
		, SLOT(updateValuesInExpressionOptions())
	);
}

void
PropertyWizard::insertOperand(QString opString, operandType_t opType) {

}
void
PropertyWizard::deleteSelectedOperand() {

}

void
PropertyWizard::getInfoAndInsertOperand() {

}

void
PropertyWizard::updateValuesInExpressionOptions() {
	int index = 0;
	ui.expressionOptions->setEditable(false);
	switch (ui.expressionOptions->currentIndex()) {
		case OPERAND_TYPE::BINARY_OPERAND:
			ui.expressionOptions->clear();
			for (OperandAndDescription opAndDesc : OperandInformation::binaryOperands) {
				ui.expressionOptions->insertItem(index, opAndDesc.description);
				index++;
			}
			break;
		case OPERAND_TYPE::UNARY_OPERAND:
			ui.expressionOptions->clear();
			for (OperandAndDescription opAndDesc : OperandInformation::unaryOperands) {
				ui.expressionOptions->insertItem(index, opAndDesc.description);
				index++;
			}
			break;
		case OPERAND_TYPE::VARIABLE:
			ui.expressionOptions->clear();
			for (QString var : variables) {
				ui.expressionOptions->insertItem(index, var);
				index++;
			}
			break;
		case OPERAND_TYPE::VALUE:
			ui.expressionOptions->clear();
			ui.expressionOptions->setEditable(true);
			ui.expressionOptions->insertItem(0, QString("True"));
			ui.expressionOptions->insertItem(0, QString("False"));
			break;
		default:
			std::cerr << "INVALID OPERAND TYPE!" << std::endl;
	}
}

} // namespace gui
} // namespace stamina
