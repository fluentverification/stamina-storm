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
		, "Maximum Iterations":ITypes.TEXT
		, "Property Refinement":ITypes.CHECKBOX
	}
	OPTIONS_STORM = {
		"Export Modified Model":ITypes.CHECKBOX
		, "Export Modified Properties":ITypes.CHECKBOX
		, "Use STAMINA Iterative method":ITypes.CHOICE
		, "Use STAMINA Priority Method":ITypes.CHOICE
	}
	OPTIONS_PRISM = {
		"Rank Transitions":ITypes.CHECKBOX
		, "Use Default Method":ITypes.CHOICE
		, "Use Power Method":ITypes.CHOICE
		, "Use Jacobi Method":ITypes.CHOICE
		, "Use Gauss-Seidel Method":ITypes.CHOICE
		, "Use Backwards Gauss-Seidel Method":ITypes.CHOICE
	}
	# Default values
	DEFAULT_VALUES = {
		"Reachability Threshold (Kappa)":1
		, "Kappa Reduction Factor":1.25
		, "Approximation Factor":2
		, "Probability Window":1e-3
		, "Maximum Iterations":10000
		, "Property Refinement":False
		, "Export Modified Model":False
		, "Export Modified Properties":False
		, "Rank Transitions":False
		, "Use Default Method":True
		, "Use Power Method":False
		, "Use Jacobi Method":False
		, "Use Gauss-Seidel Method":False
		, "Use Backwards Gauss-Seidel Method":False
		, "Use STAMINA Iterative method":True
		, "Use STAMINA Priority Method":False
	}
	SETTINGS_TYPES = {
		"Reachability Threshold (Kappa)":float
		, "Kappa Reduction Factor":float
		, "Approximation Factor":float
		, "Probability Window":float
		, "Maximum Iterations":int
		, "Property Refinement":bool
		, "Export Modified Model":bool
		, "Export Modified Properties":bool
		, "Rank Transitions":bool
		, "Use Default Method":bool
		, "Use Power Method":bool
		, "Use Jacobi Method":bool
		, "Use Gauss-Seidel Method":bool
		, "Use Backwards Gauss-Seidel Method":bool
		, "Use STAMINA Iterative method":bool
		, "Use STAMINA Priority Method":bool
	}

	def __init__(self, parent=None):
		super(MoreOptions, self).__init__(parent)
		self.resize(600, 1)
		self.setWindowTitle("More Options for STAMINA")
		self.optionWidgets = {}
		self.mainLayout = QVBoxLayout()
		self.setLayout(self.mainLayout)
		self.setUpOptions(self.OPTIONS_GENERAL, "General Options")
		self.stormOptions = self.setUpOptions(self.OPTIONS_STORM, "STORM Options")
		self.prismOptions = self.setUpOptions(self.OPTIONS_PRISM, "PRISM Options")
		self.setupFormButtons()
		self.setInitValues()

	def setInitValues(self):
		for description, value in self.DEFAULT_VALUES.items():
			widget = self.optionWidgets[description]
			if type(value) == int or type(value) == float or type(value) == str:
				widget.setText(str(value))
			elif type(value) == bool:
				widget.setChecked(value)


	def setupFormButtons(self):
		panel = QFrame()
		self.defaultsButton = QPushButton("Defaults")
		self.doneButton = QPushButton("Check Options")
		layout = QHBoxLayout()
		panel.setLayout(layout)
		layout.addWidget(self.defaultsButton)
		layout.addWidget(self.doneButton)
		self.mainLayout.addWidget(panel)
		self.doneButton.clicked.connect(
			lambda state : (self.checkValues(), self.close())
		)
		self.defaultsButton.clicked.connect(
			self.setInitValues
		)

	def setUpOptions(self, opts, groupName = None):
		groupBox = QGroupBox(groupName)
		groupLayout = QFormLayout()
		groupBox.setLayout(groupLayout)
		self.mainLayout.addWidget(groupBox)
		for description, iType in opts.items():
			if iType == ITypes.TEXT:
				label = QLabel(f"{description}:")
				field = QLineEdit()
				groupLayout.addRow(label, field)
				self.optionWidgets[description] = field
			elif iType == ITypes.CHECKBOX:
				field = QCheckBox(description)
				groupLayout.addRow(field)
				self.optionWidgets[description] = field
			elif iType == ITypes.CHOICE:
				field = QRadioButton(description)
				groupLayout.addRow(field)
				self.optionWidgets[description] = field
		return groupBox

	def checkValues(self):
		for key, valType in self.SETTINGS_TYPES.items():
			widget = self.optionWidgets[key]
			# If this is a boolean value, we can only use radio buttons
			# and checkboxes so we don't need to check
			if valType is bool:
				continue
			valString = widget.text()
			print(f"Value String is {valString}")
			if (valType is int and not valString.isnumeric()):
				err = QMessageBox()
				err.setIcon(QMessageBox.Critical)
				err.setText("Parameter Error")
				err.setInformativeText(
					f"The following parameter is incorrect:\n{key}"
				)
				err.setWindowTitle("STAMINA Error")
				err.exec_()
			elif valType is float:
				try:
					float(valString)
				except ValueError:
					err = QMessageBox()
					err.setIcon(QMessageBox.Critical)
					err.setText("Parameter Error")
					err.setInformativeText(
						f"The following parameter is incorrect:\n{key}"
					)
					err.setWindowTitle("STAMINA Error")
					err.exec_()


if __name__=='__main__':
	app = QApplication(sys.argv)
	ex = MoreOptions()
	ex.show()
	sys.exit(app.exec_())
