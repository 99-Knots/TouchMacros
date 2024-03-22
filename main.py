import sys
from PySide6.QtWidgets import QMainWindow, QApplication, QWidget, QLabel, QVBoxLayout
from PySide6.QtGui import QCloseEvent
from PySide6.QtCore import Qt, QRect


class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

        screen_geo = QApplication.primaryScreen().availableGeometry()
        width = 150
        height = screen_geo.height()
        x = screen_geo.right() - width
        y = screen_geo.top()
        self.setGeometry(QRect(x, y, width, height))

        self.setWindowFlags(Qt.WindowType.Tool |
                            Qt.WindowType.CustomizeWindowHint |
                            Qt.WindowType.WindowCloseButtonHint |
                            Qt.WindowType.WindowTitleHint)
        self.setWindowTitle('TouchMacros')

        layout = QVBoxLayout(self)
        layout.addWidget(QLabel('Test1'))
        layout.addWidget(QLabel('Test2'))
        layout.addWidget(QLabel('Test3'))

        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        central_widget.setLayout(layout)
        layout.setAlignment(Qt.AlignmentFlag.AlignTop)

    def closeEvent(self, event: QCloseEvent) -> None:
        super(MainWindow, self).closeEvent(event)
        QApplication.quit()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec())
