#!/usr/bin/env python3

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
import subprocess

EXECUTABLE = "../../build/stamina-cplusplus"

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
        self.output.setReadOnly(True)
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
        self.addRowWithBrowse(self.modFile, self.browseModFile, "Modules File: ", True, {'prism':"Prism Files", 'sm':"Prism Files (Legacy)"})
        self.propFile = QLineEdit()
        self.browsePropFile = QPushButton("...")
        self.addRowWithBrowse(self.propFile, self.browsePropFile, "Properties File: ", True, {'csl':"Prism Properties File"})
        self.consts = QLineEdit()
        self.leftLayout.addRow(QLabel("Constants: "), self.consts)
        self.export = QLineEdit()
        self.browseExport = QPushButton('...')
        self.addRowWithBrowse(self.export, self.browseExport, "Export file:", False)
        self.moreOptions = QPushButton("More Options...")
        self.leftLayout.addRow(self.moreOptions)
        self.start = QPushButton("Analyse Model")
        self.leftLayout.addRow(self.start)
        self.start.clicked.connect(self.run)

    def addRowWithBrowse(self, widget, browse, label, open=True, allowedExts={"txt":"Text Files"}):
        hbox = QHBoxLayout()
        hbox.addWidget(widget)
        hbox.addWidget(browse)
        self.leftLayout.addRow(QLabel(label), hbox)
        if open:
            browse.clicked.connect(
                lambda state, textbox=widget, allowedExtensions=allowedExts : self.getImportFilePath(textbox, allowedExtensions)
            )
        else:
            browse.clicked.connect(
                lambda state, textbox=widget, allowedExtensions=allowedExts : self.getExportFilePath(textbox, allowedExtensions)
            )

    def getImportFilePath(self, textbox, allowedExtensions):
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        exs = self.createExtensionMask(allowedExtensions)
        fileName, _ = QFileDialog.getOpenFileName(self,"Open File", "",exs, options=options)
        if fileName:
            textbox.setText(fileName)

    def getExportFilePath(self, textbox, allowedExtensions):
        options = QFileDialog.Options()
        options |= QFileDialog.DontUseNativeDialog
        exs = self.createExtensionMask(allowedExtensions)
        fileName, _ = QFileDialog.getOpenFileName(self,"Export File", "",exs, options=options)
        if fileName:
            textbox.setText(fileName)

    #
    # Creates a file extension mask for QFileDialog
    #
    def createExtensionMask(self, allowedExtensions, allFiles = True):
        rstr = ""
        for ex, desc in allowedExtensions.items():
            rstr += desc + " (*." + ex + ");;"
        if allFiles:
            rstr += "All Files (*)"
        else:
            rstr = rstr[:len(rstr) - 2]
        return rstr

    def runStamina(self):
        command = f"{EXECUTABLE} {self.modFile.text()} {self.propFile.text()}"
        proc = subprocess.Popen(
            command
            , stdout = subprocess.PIPE
            , stderr = subprocess.STDOUT
            , universal_newlines = True
        )
        return proc

    def trace(self, proc):
        while proc.poll() is None:
            line = proc.stdout.readline()
            if line:
                self.output.appendPlainText(line)

    def run(self):
        self.output.appendPlainText("[INFO] Creating STAMINA subprocess to stamina-cplusplus executable...\n")
        # self.output.appendPlainText("[ERROR] The GUI is not finished yet. Please use command line.\n")
        proc = self.runStamina()
        self.trace(proc)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    ex = XStamina()
    ex.show()
    sys.exit(app.exec_())
