##########################################################
# Simple GUI for STAMINA (while I try to get the kinks
# in the core worked out)
#
# Written by Josh Jeppson
##########################################################

import sys
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

class XStamina(QWidget):
    def __init__(self, parent=None):
        super(XStamina, self).__init__(parent)
        self.resize(800,400)
        self.setWindowTitle("STAMINA - State Space Truncator for CTMC")
        split = QSplitter(Qt.Horizontal)
        left = QFrame(self)
        left.setFrameShape(QFrame.StyledPanel)
        right = QFrame(self)
        right.setFrameShape(QFrame.StyledPanel)
        self.output = QPlainTextEdit()
        self.leftLayout = QFormLayout()
        left.setLayout(self.leftLayout)
        rightLayout = QHBoxLayout()
        rightLayout.addWidget(self.output)
        right.setLayout(rightLayout)
        split.addWidget(left)
        split.addWidget(right)
        mainLayout = QHBoxLayout()
        mainLayout.addWidget(split)
        self.setLayout(mainLayout)
        self.addFormButtons()
    
    def addFormButtons(self):
        self.modFile = QLineEdit()
        self.browseModFile = QPushButton("...")
        self.addRowWithBrowse(self.modFile, self.browseModFile, "Modules File: ")
        self.propFile = QLineEdit()
        self.browsePropFile = QPushButton("...")
        self.addRowWithBrowse(self.propFile, self.browsePropFile, "Properties File: ")
        self.consts = QLineEdit()
        self.leftLayout.addRow(QLabel("Constants: "), self.consts)
        self.export = QLineEdit()
        self.browseExport = QPushButton('...')
        self.addRowWithBrowse(self.export, self.browseExport, "Export file:")

    def addRowWithBrowse(self, widget, browse, label):
        hbox = QHBoxLayout()
        hbox.addWidget(widget)
        hbox.addWidget(browse)
        self.leftLayout.addRow(QLabel(label), hbox)



if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = XStamina()
    ex.show()
    sys.exit(app.exec_())