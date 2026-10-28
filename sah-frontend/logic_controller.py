import sys
import random
from PyQt6.QtCore import QTimer, Qt
from PyQt6.QtWidgets import QButtonGroup, QMainWindow, QLabel
from PyQt6.QtGui import QPixmap
from ui_layout import SavunmaSistemiLayout

class controller(QMainWindow):
    def __init__(self):
        super().__init__()
        
        self.ui=SavunmaSistemiLayout()
        self.setCentralWidget(self.ui)

        self.apply_styles()
        self.setup_background_overlays(self.ui)

        self.mode_logic=QButtonGroup()
        self.mode_logic.addButton(self.ui.btn_manuel)
        self.mode_logic.addButton(self.ui.btn_otonom)
        self.mode_logic.addButton(self.ui.btn_mod1)
        self.mode_logic.addButton(self.ui.btn_mod2)
        self.mode_logic.setExclusive(True)
        self.mode_logic.buttonClicked.connect(self.mod_change)

        self.timer=QTimer()
        self.timer.timeout.connect(self.data_change)
        self.timer.start(2000)
        self.log_add("TARANIYOR")
    def apply_styles(self):
        background="black"
        foreground="#00FF41"
        self.setStyleSheet(f"""
        
        
""")




        
