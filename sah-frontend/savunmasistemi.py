import sys
import random
from PyQt6.QtWidgets import (
    QApplication, 
    QMainWindow, 
    QWidget, 
    QVBoxLayout, 
    QHBoxLayout,
    QGroupBox,
    QLabel, 
    QLCDNumber, 
    QProgressBar,
    QListWidget,
    QPushButton,
    QGridLayout,
    QButtonGroup
)
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QPixmap
class HavaSavunmaIzlemePaneli(QMainWindow):

    def __init__(self):
        super().__init__()

        self.merkez_widget = QWidget()
        self.setCentralWidget(self.merkez_widget)

        self.ana_duzen = QHBoxLayout()
        self.sag_taraf = QVBoxLayout()  

        self.kamera = QLabel("NO SİGNAL")
        self.kamera.setStyleSheet("""
        QLabel{
        border: 2px solid #00FF41;
        background-color: black;
        color: #00FF41;
        font-size: 20px;
        }
        """)
        self.kamera.setMinimumSize(800,500)
        self.alt_panel = QGroupBox("SİSTEM VERİLERİ")
        self.verilerLayout = QVBoxLayout()
        self.lazer=QLabel("Lazer Gücü : ")
        self.aktifmod=QLabel("Aktif Mod : ")
        self.koordinat=QLabel("Koordinatlar : ")
        self.verilerLayout.addWidget(self.lazer)
        self.verilerLayout.addWidget(self.aktifmod)
        self.verilerLayout.addWidget(self.koordinat)
        self.alt_panel.setStyleSheet("""
        QGroupBox{
        border:2px solid #00ff41;
        color:#00ff41;
        }
        QLabel{
        color:#00ff41;
        font-size:14px;
        }
""")
    
        self.alt_panel.setMinimumHeight(100)

        self.alt_panel.setLayout(self.verilerLayout)

        self.mod_grubu= QGroupBox("SİSTEM MODLARI") 
        mod_layout = QGridLayout() 

        self.btn_manuel = QPushButton("MANUEL")
        self.btn_manuel.setCheckable(True)
        self.btn_manuel.setChecked(True)

        self.btn_otonom = QPushButton("OTONOM")
        self.btn_otonom.setCheckable(True)

        self.btn_mod1 = QPushButton("MOD 1")
        self.btn_mod1.setCheckable(True)

        self.btn_mod2 = QPushButton("MOD 2")
        self.btn_mod2.setCheckable(True)

        mod_layout.addWidget(self.btn_manuel , 0 , 0 )
        mod_layout.addWidget(self.btn_otonom , 0 , 1 )
        mod_layout.addWidget(self.btn_mod1 , 1 , 0 )
        mod_layout.addWidget(self.btn_mod2, 1 , 1 )
        self.mod_grubu.setLayout(mod_layout) 
        # log sistemi
        self.log_sistemi = QGroupBox("LOG SİSTEMİ")
        log_layout = QVBoxLayout()
        self.olay_listesi = QListWidget()
        log_layout.addWidget(self.olay_listesi)
        self.log_sistemi.setLayout(log_layout)
        self.log_sistemi.setMaximumHeight(250)
        self.log_sistemi.setMinimumHeight(150)
        self.log_ekle("SİSTEM TARANIYOR")

        self.sag_taraf.addWidget(self.mod_grubu)
        self.sag_taraf.addWidget(self.alt_panel)
        self.sag_taraf.addWidget(self.log_sistemi)
        
        self.ana_duzen.addWidget(self.kamera,7)
        self.ana_duzen.addLayout(self.sag_taraf,3)

        self.merkez_widget.setLayout(self.ana_duzen)

        #renk

        arka_plan = "black"
        on_plan = "#00ff41"
        self.setStyleSheet(f"""
            QMainWindow {{
                background-color: {arka_plan};
            }}
            QGroupBox {{
                color: {on_plan};
                border: 2px solid {on_plan};
                border-radius: 8px;
                margin-top: 15px;
                font-size: 14px;
                font-weight: bold;
            }}
            QGroupBox::title {{
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }}
            QPushButton {{
                background-color: rgb(50, 60, 75);
                color: white;
                border: 1px solid {on_plan};
                padding: 8px;
                border-radius: 4px;
                font-weight: bold;
            }}
            QPushButton:checked {{
                background-color: {on_plan};
                color: {arka_plan};
            }}
            QListWidget {{
                background-color: rgb(30, 35, 45);
                color: {on_plan};
                border: none;
                font-family: 'Consolas';  
            }}
            QLabel {{
                color: white;
                font-weight: bold;
            }}
    """)
        #arka plan
        self.arka_plan_label = QLabel(self.merkez_widget)
        self.arka_plan_pixmap = QPixmap("arka_plan2.png")
        self.arka_plan_label.setPixmap(self.arka_plan_pixmap)
        self.arka_plan_label.setScaledContents(True)
        #karartma
        self.overlay = QLabel(self.merkez_widget)
        self.overlay.setStyleSheet("background-color: rgba(0, 0, 0, 60);")

        self.overlay.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        #baslangıc boyutları
        self.arka_plan_label.setGeometry(self.merkez_widget.rect())
        self.overlay.setGeometry(self.merkez_widget.rect())

        self.arka_plan_label.lower()
        self.overlay.raise_()
        #zamanlayıcı
        self.zamanlayici = QTimer()
        self.zamanlayici.timeout.connect(self.verileri_guncelle)
        self.zamanlayici.start(2000)
    
        # buton kontrolu
        self.mod_mantigi = QButtonGroup()
        self.mod_mantigi.addButton(self.btn_manuel)
        self.mod_mantigi.addButton(self.btn_otonom)
        self.mod_mantigi.addButton(self.btn_mod1)
        self.mod_mantigi.addButton(self.btn_mod2)
        self.mod_mantigi.setExclusive(True)
        self.mod_mantigi.buttonClicked.connect(self.mod_degisti)
        
    def mod_degisti(self, button):
        self.log_ekle(f"MOD DEĞİŞTİ : {button.text()}")

    def log_ekle(self,mesaj):
        self.olay_listesi.addItem(mesaj)
        if self.olay_listesi.count()>200:
            self.olay_listesi.takeItem(0)
        self.olay_listesi.scrollToBottom()

    def verileri_guncelle(self):
        self.log_ekle("SİSTEM TARANIYOR")

    def resizeEvent(self,event):
        self.arka_plan_label.setGeometry(self.merkez_widget.rect())
        self.overlay.setGeometry(self.merkez_widget.rect())
        super().resizeEvent(event)
if __name__ == "__main__":
    app = QApplication(sys.argv)

    pencere = HavaSavunmaIzlemePaneli()
    pencere.show()

    sys.exit(app.exec())