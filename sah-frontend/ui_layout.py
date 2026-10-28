import sys 
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGroupBox, 
    QLabel, QPushButton, QGridLayout, QListWidget
)
from PyQt6.QtCore import Qt

class SavunmaSistemiLayout(QWidget):
    def __init__(self):
        super().__init__()
        self.main_setup()

    def main_setup(self):
        self.main_layout = QHBoxLayout()
        self.right_panel = QVBoxLayout()

        self.camera = QLabel("NO SIGNAL")
        self.camera.setStyleSheet("""
        QLabel{border:2px solid #00ff41;background-color:black;color:#00ff41;font-size:20px;}""")
        self.camera.setMinimumSize(800,500)
        
        self.bottom_panel=QGroupBox("SİSTEM VERİLERİ")
        self.data_layout=QVBoxLayout()
        self.laser=QLabel("Lazer : ")
        self.activeMod= QLabel("Aktif Mod : ")
        self.coordinate=QLabel("Koordinatlar : ")

        self.data_layout.addWidget(self.laser)
        self.data_layout.addWidget(self.activedMod)
        self.data_layout.addWidget(self.coordinate)
        self.bottom_panel.setLayout(self.data_layout)

        self.modeGroup=QGroupBox("SİSTEM MODLARI")
        self.mod_grubu = QGroupBox("SİSTEM MODLARI")
        mode_layout = QGridLayout()
        self.btn_manuel = QPushButton("MANUEL")
        self.btn_manuel.setCheckable(True)
        self.btn_manuel.setChecked(True)
        self.btn_otonom = QPushButton("OTONOM")
        self.btn_otonom.setCheckable(True)
        self.btn_mod1 = QPushButton("MOD 1")
        self.btn_mod1.setCheckable(True)
        self.btn_mod2 = QPushButton("MOD 2")
        self.btn_mod2.setCheckable(True)

        mode_layout.addWidget(self.btn_manuel, 0, 0)
        mode_layout.addWidget(self.btn_otonom, 0, 1)
        mode_layout.addWidget(self.btn_mod1, 1, 0)
        mode_layout.addWidget(self.btn_mod2, 1, 1)
        self.mod_grubu.setLayout(mode_layout)

        self.log_system=QGroupBox("LOG SİSTEMİ")
        log_layout=QVBoxLayout()
        self.event_list=QListWidget()
        log_layout.addWidget(self.event_list)
        self.log_system.setLayout(log_layout)

        self.right_panel.addWidget(self.modeGroup)
        self.right_panel.addWidget(self.bottom_panel)
        self.right_panel.addWidget(self.log_system)
        self.main_layout.addWidget(self.camera,7)
        self.main_layout.addLayout(self.right_panel,3)
        self.setLayout(self.main_layout)

        


