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
	ui.expressionType->insertItem(OPERAND_TYPE::BINARY_OPERAND, QString("Binary Operand"));
	ui.expressionType->insertItem(OPERAND_TYPE::UNARY_OPERAND, QString("Unary Operand"));
	ui.expressionType->insertItem(OPERAND_TYPE::VARIABLE, QString("Variable"));
	ui.expressionType->insertItem(OPERAND_TYPE::VALUE, QString("Value"));

	connect(
		ui.expressionType
		, SIGNAL(currentIndexChanged(int))
		, this
		, SLOT(updateValuesInExpressionOptions(int))
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
PropertyWizard::updateValuesInExpressionOptions(int oldIndex) {
	int index = 0;
	ui.expressionOptions->setEditable(false);
	std::cout << "Resetting values in combobox" << std::endl;
	switch (oldIndex) {
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
			ui.expressionOptions->insertItem(1, QString("False"));
			break;
		default:
			std::cerr << "INVALID OPERAND TYPE!" << std::endl;
	}
}

// OperandItem implementation

OperandItem::OperandItem(
	const QVector<QVariant> &data
	, OperandItem *parentItem
	, operandType_t opType
)
	: m_itemData(data)
	, m_parentItem(parentItem)
	, opType(opType)
{
	// Intentionally left empty
}

OperandItem::~OperandItem() {
	qDeleteAll(m_childItems);
}

void
OperandItem::appendChild(OperandItem * child) {
	m_childItems.append(child);
}

OperandItem *
OperandItem::child(int row) {
	if (row < 0 || row >= m_childItems.size()) {
		return nullptr;
	}
	return m_childItems.at(row);
}

int
OperandItem::childCount() {
	return m_childItems.count();
}

int
OperandItem::columnCount() {
	return m_itemData.count();
}

QVariant
OperandItem::data(int column) {
	if (column < 0 || column >= m_itemData.size()) {
		return QVariant();
	}
	return m_itemData.at(column);

}

int
OperandItem::row() {
	if (m_parentItem) {
		return m_parentItem->m_childItems.indexOf(const_cast<OperandItem *>(this));
	}
	return 0;
}

OperandItem *
OperandItem::parentItem() {
	return m_parentItem;
}

QString
OperandItem::createExpressionFromThisAndChildren() {
	switch (opType) {
		case OPERAND_TYPE::BINARY_OPERAND:
			if (childCount() != 2) {
				std::cerr << "Cannot create binary operand with child count " << childCount() << "!" << std::endl;
				return QString("OPERROR");
			}
			return QString("(  ")
				+ m_childItems.at(0)->createExpressionFromThisAndChildren()
				+ ' ' + data(1).toString() + ' '
				+ m_childItems.at(1)->createExpressionFromThisAndChildren()
				+ QString(" )");
		case OPERAND_TYPE::UNARY_OPERAND:
			if (childCount() != 1) {
				std::cerr << "Cannot create unary operand with child count " << childCount() << "!" << std::endl;
				return QString("OPERROR");
			}
			return QString("(  ")
				+ m_childItems.at(0)->createExpressionFromThisAndChildren()
				+ ' ' + data(1).toString() + ' '
				+ QString(" )");
		case OPERAND_TYPE::VARIABLE:
		case OPERAND_TYPE::VALUE:
			// fallthrough intentional
			if (childCount() != 0) {
				std::cerr << "Cannot create unary operand with nonzero child count " << childCount() << "!" << std::endl;
				return QString("OPERROR");
			}
			return data(1).toString();
		case OPERAND_TYPE::EMPTY:
			std::cerr << "Operand is empty!" << std::endl;
			return QString("EMPTY");
		default:
			return QString("OPERROR");
	}
}

// Implementation for PropertyTreeModel

PropertyTreeModel::PropertyTreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
	rootItem = new OperandItem({tr("Title"), tr("Summary")});
	setupModelData(data.split('\n'), rootItem);
}

PropertyTreeModel::~PropertyTreeModel() {
	delete rootItem;
}

QVariant
PropertyTreeModel::data(const QModelIndex &index, int role) {

}
Qt::ItemFlags
PropertyTreeModel::flags(const QModelIndex &index) {

}

QVariant
PropertyTreeModel::headerData(
	int section
	, Qt::Orientation orientation
	, int role
) {

}

QModelIndex
PropertyTreeModel::index(
	int row
	, int column
	, const QModelIndex & parent
) {

}

QModelIndex
PropertyTreeModel::parent(const QModelIndex &index) {

}
int
PropertyTreeModel::rowCount(const QModelIndex &parent) {

}
int
PropertyTreeModel::columnCount(const QModelIndex &parent) {

}

QString
PropertyTreeModel::toPropertyString() {

}

void
PropertyTreeModel::setupModelData(const QStringList &lines, OperandItem * parent) {

}

} // namespace gui
} // namespace stamina
