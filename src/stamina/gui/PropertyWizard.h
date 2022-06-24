#ifndef STAMINA_PROPERTY_WIZARD_H
#define STAMINA_PROPERTY_WIZARD_H

#include <KXmlGuiWindow>
#include <QString>
#include <cstdint>

namespace stamina {
	namespace gui {
		enum OPERAND_TYPE {
			BINARY_OPERAND
			, UNARY_OPERAND
			, VARIABLE
		};
		typedef uint8_t operandType_t;
		struct OperandAndDescription {
			QString operand;
			QString description;
			OperandAndDescription(QString operand, QString description)
				: operand(operand), description(description) { /* Intentionally left empty */ }
		};
		struct OperandInformation {
			static OperandAndDescription[] binaryOperands = {
				// Logic operands
				OperandAndDescription("&", "AND Operand") // AND Operand
				, OperandAndDescription("|", "OR Operand") // OR Operand
				// Path-formula Operands
				, OperandAndDescription("F", "Eventually Operand") // Eventually Operand
				, OperandAndDescription("U", "Until Operand") // Until operand
				// Math Operands
				, "+" // Add
				, "-" // Subtract
				, "*" // Multiply
				, "/" // divide
			};
		};
		class PropertyWizard : KXmlGuiWindow {
			Q_OBJECT
		public:
			PropertyWizard(QObject * parent = nullptr);
		protected:
		private slots:
			/**
			 * Inserts into the operand tree. Will insert over "EMPTY" if selected. If no "EMPTY" is
			 * selected, raises messagebox
			 *
			 * @param opString The string of the operand
			 * @param opType The type of the Operand
			 * */
			void insertOperand(QString opString, operandType_t opType);
			/**
			 * Deletes the selected operand in the tree.
			 * */
			void deleteSelectedOperand();
		};
	}
}
#endif // STAMINA_PROPERTY_WIZARD_H
