#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <windows.h>
#include <vector>


struct EnumWinParam
{
    std::vector<HWND> *windows;
    HMONITOR monitor;
};

bool isProperWindow(HWND handle)
{
    if (!IsWindowVisible(handle))
        return false;

    TITLEBARINFO ti;
    ti.cbSize = sizeof(ti);
    GetTitleBarInfo(handle, &ti);
    if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
        return false;

    return true;
}

BOOL CALLBACK EnumWinProc(HWND handle, LPARAM param)
{
    EnumWinParam *p = reinterpret_cast<EnumWinParam*>(param);
    HMONITOR monitor = MonitorFromWindow(handle, MONITOR_DEFAULTTONEAREST);
    if (p->monitor == monitor && isProperWindow(handle))
        p->windows->push_back(handle);
    return TRUE;
}

MainWindow::MainWindow (QWidget* parent) : QMainWindow (parent)
{
    //resize(720,720);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    QPushButton* rearrBtn = new QPushButton("reorder layout");
    connect(rearrBtn, &QPushButton::clicked, this, &MainWindow::rearrangeScreen);
    layout->addWidget(rearrBtn);

    buttonArray[0] = new Key("Y", 0x59);
    buttonArray[1] = new Key("Q", 0x51);
    buttonArray[2] = new ModifierKey("Shift", VK_SHIFT);
    buttonArray[3] = new ModifierKey("Control", VK_CONTROL);
    buttonArray[4] = new ModifierKey("Alt", VK_MENU);
    for (int i = 0; i < BUTTON_COUNT; i++) {
        layout->addWidget(buttonArray[i]);
    }

    QWidget* central_w = new QWidget(this);
    central_w->setLayout(layout);
    setCentralWidget(central_w);

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    setWindowTitle("TouchMacros");

    handle = (HWND)winId();     // get window handle of the app from identifier
    SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_NOACTIVATE  | WS_EX_APPWINDOW);    // add NoActive to window style through Windows API to prevent it from drawing keyboard focus
}

void MainWindow::rearrangeScreen()
{
    HMONITOR monitor = MonitorFromPoint(POINT{pos().x(), pos().y()}, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfo(monitor, &mi);
    std::vector<HWND> windows;
    EnumWinParam param;
    param.windows = &windows;
    param.monitor = monitor;
    EnumWindows(EnumWinProc, reinterpret_cast<LPARAM>(&param));
    RECT rect;

    for (int i=windows.size(); i>0; --i)    // traverse in reverse order to preserve z-order when using SetWindowPos
    {
        HWND hwnd = windows[i];
        rect = mi.rcWork;
        rect.right -= win_width;
        TCHAR win_text[200];
        GetWindowText(hwnd, win_text, 200);
        QString title = QString::fromWCharArray(win_text);
        if (hwnd != handle && !title.isEmpty()){
            // get right edge of window including frame and decorations
            RECT clRect;
            GetClientRect(hwnd, &clRect);
            POINT topRight = {clRect.right, clRect.top};
            ClientToScreen(hwnd, &topRight);

            if (topRight.x>rect.right-2) {  //exclude windows that don't overlap from updating
                ShowWindow(hwnd, SW_NORMAL);
                AdjustWindowRectEx(&rect, GetWindowLongW(hwnd, GWL_STYLE), FALSE, GetWindowLong(hwnd, GWL_EXSTYLE));    // need to adjust rect to account for window shadow, frame, etc...
                SetWindowPos(hwnd, NULL, rect.left + 1, mi.rcWork.top + 1, rect.right - rect.left - 3, rect.bottom - mi.rcWork.top - 2, SWP_NOZORDER | SWP_NOACTIVATE); // keep old top coordinate because AdjustWinRect crops off titlebar
            }
        }
    }
    // reposition mainwindow to right screen edge
    rect = mi.rcWork;
    rect.left = rect.right - win_width;
    AdjustWindowRectEx(&rect, GetWindowLongW(handle, GWL_STYLE), FALSE, GetWindowLong(handle, GWL_EXSTYLE));
    SetWindowPos(handle, NULL, rect.left + 1, mi.rcWork.top, rect.right - rect.left - 2, rect.bottom - mi.rcWork.top - 2, SWP_SHOWWINDOW);
    // todo: resizeEvent writes new width to win_width before calling rearrange to scale other windows as well
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
