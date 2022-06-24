#!/bin/sh

# Main Window
rm ui/ui_MainWindow.h
uic ui/MainWindow.ui >> ui/ui_MainWindow.h

# About
rm ui/ui_About.h
uic ui/About.ui >> ui/ui_About.h

# Preferences
rm ui/ui_Preferences.h
uic ui/Preferences.ui >> ui/ui_Preferences.h

# Property Wizard
rm ui/ui_PropertyWizard.h
uic ui/PropertyWizard.ui >> ui/ui_PropertyWizard.h
