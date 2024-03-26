#include "mainwindow.h"
#include <QApplication>
#include <QVBoxLayout>


MainWindow::MainWindow (QWidget* parent) : QMainWindow (parent)
{
    //resize(720,720);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    buttonArray[0] = new Key("Y", 0x59);
    buttonArray[1] = new Key("Q", 0x51);
    buttonArray[2] = new ModifierKey("Shift", VK_SHIFT);
    buttonArray[3] = new ModifierKey("Control", VK_CONTROL);
    buttonArray[4] = new ModifierKey("Alt", VK_MENU);
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        layout->addWidget(buttonArray[i]);
    }

    QWidget* central_w = new QWidget(this);
    central_w->setLayout(layout);
    setCentralWidget(central_w);

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    setWindowTitle("TouchMacros");

    HWND winhandle = (HWND)winId();     // get window handle of the app from identifier
    SetWindowLong(winhandle, GWL_EXSTYLE, GetWindowLong(winhandle, GWL_EXSTYLE) | WS_EX_NOACTIVATE  | WS_EX_APPWINDOW);    // add NoActive to window style through Windows API to prevent it from drawing keyboard focus
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QApplication::quit();   //quit on closure -> prevent app from continuing to run in the background after gui is discarded; important if app is Tool window
    QMainWindow::closeEvent(e);
}

MainWindow::~MainWindow ()
{
    for (int i = 0; i < BUTTON_COUNT; ++i) {
        delete buttonArray[i];
    }
}
