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

    bool isProperWindow(HWND hwnd)
    {
        if (!IsWindowVisible(hwnd))
            return false;

        if (!GetWindowTextLength(hwnd))
            return false;

        TITLEBARINFO ti;
        ti.cbSize = sizeof(ti);
        GetTitleBarInfo(hwnd, &ti);
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


    BOOL repositionWin (HWND hwnd, PWINDOWINFO wi, LPRECT rect, UINT flags, Alignment a, bool fullscreen=false)
    {
        // todo: prevent windows from being pushed out of screen
        ShowWindow(hwnd, SW_NORMAL);
        // to counter the invisible borders / shadows on windows 10
        int borderXl = 0;
        int borderXr = 0;
        int borderY = 0;

        switch (a) {
            case Alignment::LEFT:
                borderXl =  wi->cxWindowBorders - 1;
                break;
            case Alignment::RIGHT:
                borderXr =  wi->cxWindowBorders - 1;
                break;
            case Alignment::BOTTOM:
                borderY =  wi->cyWindowBorders - 1;
                break;
            default:
                break;
        }

        if (fullscreen) {
            borderXl = wi->cxWindowBorders - 1;
            borderXr = wi->cxWindowBorders - 1;
            borderY = wi->cyWindowBorders - 1;
        }

        return SetWindowPos(hwnd, NULL, rect->left - borderXl, rect->top, rect->right - rect->left + borderXl + borderXr, rect->bottom - rect->top + borderY, flags);
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

bool MainWindow::CheckAlignment()
{
    int errorMargin = 2;    // in pixel
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(handle, &wi);
    std::pair<HMONITOR, MONITORINFO> m = getMonitor(handle);

    // test if window actually touches edge it is supposed to be attached to
    if (alignment == Alignment::LEFT && !(std::abs(long(wi.rcWindow.left + wi.cxWindowBorders - m.second.rcWork.left)) <= errorMargin))
        alignment = Alignment::NONE;

    if (alignment == Alignment::TOP && !(std::abs(long(wi.rcWindow.top - m.second.rcWork.top)) <= errorMargin))
        alignment = Alignment::NONE;

    if (alignment == Alignment::RIGHT && !(std::abs(long(wi.rcWindow.right - wi.cxWindowBorders - m.second.rcWork.right)) <= errorMargin))
        alignment = Alignment::NONE;

    if (alignment == Alignment::BOTTOM && !(std::abs(long(wi.rcWindow.bottom - wi.cyWindowBorders - m.second.rcWork.bottom)) <= errorMargin))
        alignment = Alignment::NONE;

    return (alignment!=Alignment::NONE);
}

void MainWindow::repositionOther(int widthLeftFree, int comparisonWidth)
{
    UINT dpi = GetDpiForWindow(handle);
    int widthDPI = widthLeftFree * dpi / USER_DEFAULT_SCREEN_DPI;  // account for different dpi for scaled displays

    std::pair<HMONITOR, MONITORINFO> monitor = getMonitor(handle);
    std::vector<HWND> windows = getWindowsOnMonitor(monitor.first);

    RECT newWinRect;
    WINDOWINFO wi;
    wi.cbSize = sizeof(WINDOWINFO);

    WINDOWINFO appWi;
    appWi.cbSize = sizeof(WINDOWINFO);
    GetWindowInfo(handle, &appWi);
    RECT appRect;
    GetWindowRect(handle, &appRect);
    // todo: fix resize bug where other window don't follow if moving to fast
    //if (comparisonWidth > 0){
    //    appRect.left = appRect.right - comparisonWidth * dpi / USER_DEFAULT_SCREEN_DPI;
    //}
    //appRect.left -= 2;

    for (int i=windows.size(); i>0; --i)    // traverse in reverse order to preserve z-order when using SetWindowPos
    {
        HWND hwnd = windows[i];
        GetWindowInfo(hwnd, &wi);

        WINDOWPLACEMENT wplc;
        wplc.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &wplc);
        bool fullscreen = wplc.showCmd == SW_MAXIMIZE;

        RECT r;
        if (hwnd != handle && IntersectRect(&r, &appRect, &wi.rcWindow)){
            if (fullscreen)
                newWinRect = monitor.second.rcWork;
            else
                newWinRect = wi.rcWindow;
            //newWinRect.right = monitor.second.rcWork.right - widthDPI;
            //newWinRect.top = r.bottom - appWi.cyWindowBorders + r.top - newWinRect.top;
            //if (alignment == Alignment::RIGHT)
            //    newWinRect.right = monitor.second.rcWork.right - widthDPI - 1;

            switch (alignment) {
                case Alignment::LEFT:
                    newWinRect.left = monitor.second.rcWork.left + widthDPI - 1;
                    break;
                case Alignment::TOP:
                    newWinRect.top = monitor.second.rcWork.top + widthDPI - 1;
                    break;
                case Alignment::RIGHT:
                    newWinRect.right = monitor.second.rcWork.right - widthDPI - 1;
                    break;
                case Alignment::BOTTOM:
                    newWinRect.bottom = monitor.second.rcWork.bottom - widthDPI - 1;;
                    break;
                default:
                    break;
            }
            repositionWin(hwnd, &wi, &newWinRect, SWP_NOZORDER | SWP_NOACTIVATE, alignment, fullscreen);
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
    switch (alignment) {
        case Alignment::LEFT:
            rect.right = rect.left + widthDPI;
            break;
        case Alignment::TOP:
            rect.bottom = rect.top + widthDPI;
            break;
        case Alignment::RIGHT:
            rect.left = rect.right - widthDPI;
            break;
        case Alignment::BOTTOM:
            rect.top = rect.bottom - widthDPI;
            break;
        default:
            break;
    }
    GetWindowInfo(handle, &wi);
    repositionWin(handle, &wi, &rect, SWP_SHOWWINDOW, alignment, true);
}

void MainWindow::rearrangeScreen()
{
    alignment = Alignment::RIGHT;
    repositionSelf(win_width);
    repositionOther(win_width);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (CheckAlignment() && !suppressResize){
        if (alignment == Alignment::LEFT || alignment == Alignment::RIGHT)
            repositionOther(e->size().width(), e->oldSize().width());
        else
            repositionOther(frameGeometry().height(), frameGeometry().height() + e->size().height() - e->oldSize().height());
    }
    suppressResize = false;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QApplication::quit();   //quit on closure -> prevent app from continuing to run in the background after gui is discarded; important if app is Tool window
    QMainWindow::closeEvent(e);
}

MainWindow::~MainWindow ()
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        delete buttonArray[i];
    }
}
