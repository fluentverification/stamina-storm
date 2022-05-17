import sys

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

# Option input types
class ITypes:
	TEXT = 0
	CHECKBOX = 1
	CHOICE = 2

class MoreOptions(QWidget):
	OPTIONS_GENERAL = {
		"Reachability Threshold (Kappa)":ITypes.TEXT
		, "Kappa Reduction Factor":ITypes.TEXT
		, "Approximation Factor":ITypes.TEXT
		, "Probability Window":ITypes.TEXT
		# , "Property Refinement":ITypes.CHECKBOX
	}
	def __init__(self, parent=None):
		super(MoreOptions, self).__init__(parent)
		self.resize(600, 1)
		self.setWindowTitle("More Options for STAMINA")
		self.optionWidgets = {}
		self.mainLayout = QFormLayout()
		self.setLayout(self.mainLayout)
		self.setUpOptions(self.OPTIONS_GENERAL)

	def setUpOptions(self, opts):
		for description, iType in opts.items():
			if iType == ITypes.TEXT:
				label = QLabel(f"{description}:")
				field = QLineEdit()
				self.mainLayout.addRow(label, field)
				self.optionWidgets[description] = field
			elif iType == iTypes.CHECKBOX:
				field = QCheckBox(descriptions)
				self.mainLayout.addRow(field)
				self.optionWidgets[description] = field

if __name__=='__main__':
	app = QApplication(sys.argv)
	ex = MoreOptions()
	ex.show()
	sys.exit(app.exec_())
