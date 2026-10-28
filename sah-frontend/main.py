import sys 
from PyQt6.QtWidgets import QApplication

class SavunmaSistemi:
    def __init__(self):
        self.app=QApplication(sys.argv)

    def run(self):
        sys.exit(self.app.exec())
    
if __name__ == "__main__" : 
    system=SavunmaSistemi()
    system.run()