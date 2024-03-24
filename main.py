import sys
from PySide6.QtWidgets import QMainWindow, QApplication, QWidget, QLabel, QPushButton, QVBoxLayout
from PySide6.QtGui import QCloseEvent
from PySide6.QtCore import Qt, QRect, Signal
import keyboard
import mouse


class Key(QPushButton):
    def __init__(self, keycode: str, name: str):
        super(Key, self).__init__()
        self.key = keycode
        self.name = name
        self.setText(self.name)
        self.clicked.connect(self.press_key)

    def __repr__(self):
        return self.name

    def press_key(self):
        keyboard.send(self.key)

    def release_key(self):
        keyboard.release(self.key)

    def deleteLater(self) -> None:
        self.release_key()
        super(Key, self).deleteLater()

    def closeEvent(self, event: QCloseEvent) -> None:
        self.release_key()
        super(Key, self).closeEvent(event)


class ModifierKey(Key):
    def __init__(self, keycode: str, name: str, default: bool = False):
        super(ModifierKey, self).__init__(keycode, name)
        self.setCheckable(True)
        self.setChecked(default)
        self.clicked.disconnect(self.press_key)
        self.clicked.connect(self.trigger_key)

    def __repr__(self):
        return self.name + (' active' if self.isChecked() else ' inactive')

    def trigger_key(self):
        if self.isChecked():
            keyboard.press(self.key)
        else:
            keyboard.release(self.key)

    def release_key(self):
        self.setChecked(False)
        self.trigger_key()


class ModifierMouse(ModifierKey):
    def __init__(self, keycode: str, name: str, default: bool = False):
        super(ModifierMouse, self).__init__(keycode, name, default)

    def press_key(self):
        mouse.press(self.key)

    def release_key(self):
        mouse.release(self.key)

    def trigger_key(self):
        if self.isChecked():
            mouse.press(self.key)
        else:
            mouse.release(self.key)


class MainWindow(QMainWindow):
    closing = Signal()

    def __init__(self, parent=None):
        super(MainWindow, self).__init__(parent)

        screen_geo = QApplication.primaryScreen().availableGeometry()
        width = 150
        height = screen_geo.height()
        x = screen_geo.right() - width
        y = screen_geo.top()
        self.setGeometry(QRect(x, y, width, height))

        self.setWindowFlags(Qt.WindowType.CustomizeWindowHint |
                            Qt.WindowType.WindowCloseButtonHint |
                            Qt.WindowType.Tool |
                            Qt.WindowType.WindowStaysOnTopHint |
                            Qt.WindowType.WindowTitleHint)
        self.setWindowTitle('TouchMacros')

        self.buttons = [ModifierKey('shift', 'Shift'), ModifierKey('ctrl', 'Control'),
                        ModifierKey('alt', 'Alt'), ModifierKey('windows', 'Windows'),
                        ModifierMouse('middle', 'Middle Mouse')]

        layout = QVBoxLayout(self)
        for b in self.buttons:
            layout.addWidget(b)
            self.closing.connect(b.release_key)

        central_widget = QWidget(self)
        self.setCentralWidget(central_widget)
        central_widget.setLayout(layout)
        layout.setAlignment(Qt.AlignmentFlag.AlignTop)

    def closeEvent(self, event: QCloseEvent) -> None:
        self.closing.emit()
        super(MainWindow, self).closeEvent(event)
        QApplication.quit()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = MainWindow()
    win.show()
    sys.exit(app.exec())
