#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <windows.h>
#include <vector>


namespace {
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

    BOOL CALLBACK GetWinOnMonitor(HWND hwnd, LPARAM param)
    {
        EnumWinParam *p = reinterpret_cast<EnumWinParam*>(param);
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        if (p->monitor == monitor && isProperWindow(hwnd))
            p->windows->push_back(hwnd);
        return TRUE;
    }


    BOOL repositionWin (HWND hwnd, PWINDOWINFO wi, LPRECT rect, UINT flags, bool fullscreen=false)
    {
        ShowWindow(hwnd, SW_NORMAL);
        // to counter the invisible borders / shadows on windows 10
        int borderXl = 0;
        int borderXr = wi->cxWindowBorders - 1;
        int borderY = 0;
        if (fullscreen) {
            borderXl = wi->cxWindowBorders - 1;
            borderY = wi->cyWindowBorders - 1;
        }

        return SetWindowPos(hwnd, NULL, rect->left - borderXl, rect->top, rect->right - rect->left + borderXr + borderXl, rect->bottom - rect->top + borderY, flags);
    }

    std::pair<HMONITOR, MONITORINFO> getMonitor(HWND hwnd)
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
        MONITORINFO mi;
        mi.cbSize = sizeof(MONITORINFO);
        GetMonitorInfo(monitor, &mi);
        return std::make_pair(monitor, mi);
    }

    std::vector<HWND> getWindowsOnMonitor(HMONITOR m)
    {
        std::vector<HWND> windows;
        EnumWinParam param;
        param.windows = &windows;
        param.monitor = m;
        EnumWindows(GetWinOnMonitor, reinterpret_cast<LPARAM>(&param));
        return windows;
    }
}



MainWindow::MainWindow (QWidget* parent) : QMainWindow (parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setAlignment(Qt::AlignTop);

    QPushButton* rearrBtn = new QPushButton("reorder layout");
    connect(rearrBtn, &QPushButton::clicked, this, &MainWindow::rearrangeScreen);
    layout->addWidget(rearrBtn);

    // todo: read & set macros from file
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

int MainWindow::isDocked()  // return 0 if not docked, 1 if at right screen edge and -1 at left edge
{
    int errorMargin = 2;    // in pixel
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(handle, &wi);
    std::pair<HMONITOR, MONITORINFO> m = getMonitor(handle);

    // test if window's right edge is within error margin of right monitor edge
    if (std::abs(int(wi.rcWindow.right - wi.cxWindowBorders - m.second.rcWork.right)) <= errorMargin)
        return 1;

    // test if within error margin of left monitor edge
    if (std::abs(int(wi.rcWindow.left + wi.cxWindowBorders - m.second.rcWork.left)) <= errorMargin)
        return -1;

    return 0;
}

void MainWindow::repositionOther(int widthLeftFree, int comparisonWidth, bool fullscreen)
{
    UINT dpi = GetDpiForWindow(handle);
    int widthDPI = widthLeftFree * dpi / USER_DEFAULT_SCREEN_DPI;  // account for different dpi for scaled displays

    std::pair<HMONITOR, MONITORINFO> monitor = getMonitor(handle);
    std::vector<HWND> windows = getWindowsOnMonitor(monitor.first);

    RECT rect;
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);

    for (int i=windows.size(); i>0; --i)    // traverse in reverse order to preserve z-order when using SetWindowPos
    {
        HWND hwnd = windows[i];

        if (hwnd != handle && GetWindowTextLength(hwnd)){
            GetWindowInfo(hwnd, &wi);
            if (fullscreen)
                rect = monitor.second.rcWork;
            else {
                rect.left = std::max(monitor.second.rcWork.left, wi.rcWindow.left);
                rect.bottom = wi.rcWindow.bottom;
                rect.top = wi.rcWindow.top;
            }
            rect.right = monitor.second.rcWork.right - widthDPI;

            LONG compareWith = rect.right - 1;
            if (comparisonWidth > 0)
                compareWith = monitor.second.rcWork.right - comparisonWidth * dpi / USER_DEFAULT_SCREEN_DPI;

            if (wi.rcWindow.right > compareWith) {  //exclude windows that don't overlap from updating
                repositionWin(hwnd, &wi, &rect, SWP_NOZORDER | SWP_NOACTIVATE, fullscreen);
            }
        }
    }
}

void MainWindow::repositionSelf(int newWidth)
{
    suppressResize = true;
    RECT rect;
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);
    UINT dpi = GetDpiForWindow(handle);
    int widthDPI = newWidth * dpi / USER_DEFAULT_SCREEN_DPI;  // account for different dpi for scaled displays
    std::pair<HMONITOR, MONITORINFO> monitor = getMonitor(handle);

    rect = monitor.second.rcWork;
    rect.left = rect.right - widthDPI;
    GetWindowInfo(handle, &wi);
    repositionWin(handle, &wi, &rect, SWP_SHOWWINDOW, true);
}

void MainWindow::rearrangeScreen()
{
    repositionSelf(win_width);
    repositionOther(win_width, 0, true);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    // todo: check if overlap for y direction as well
    if (isDocked() && !suppressResize)
        repositionOther(e->size().width(), e->oldSize().width(), false);
    suppressResize = false;
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
