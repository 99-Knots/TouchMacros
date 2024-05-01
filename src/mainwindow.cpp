#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QFile>
#include <QFrame>
#include <QScrollArea>
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


    void repositionWin (HWND hwnd, LPRECT rect, UINT flags, Alignment a, bool fullscreen=false)
    {
        WINDOWINFO wi;
        wi.cbSize = sizeof(WINDOWINFO);
        GetWindowInfo(hwnd, &wi);
        ShowWindow(hwnd, SW_NORMAL);
        // to counter the invisible borders / shadows on windows 10
        int borderXl = 0;
        int borderXr = 0;
        int borderY = 0;

        switch (a) {
            case Alignment::LEFT:
                borderXl =  wi.cxWindowBorders - 1;
                break;
            case Alignment::RIGHT:
                borderXr =  wi.cxWindowBorders - 1;
                break;
            case Alignment::BOTTOM:
                borderY =  wi.cyWindowBorders - 1;
                break;
            default:
                break;
        }

        if (fullscreen) {
            borderXl = wi.cxWindowBorders - 1;
            borderXr = wi.cxWindowBorders - 1;
            borderY = wi.cyWindowBorders - 1;
        }
        int width = rect->right-rect->left + borderXl + borderXr;
        int height = rect->bottom - rect->top + borderY;

        // try resizing window
        SetWindowPos(hwnd, NULL, rect->left - borderXl, rect->top, width, height, flags | SWP_NOMOVE);

        RECT r;
        GetWindowRect(hwnd, &r);
        // test if resize conflicts with min size of window -> if window did not resize to provided size
        if (r.right - r.left != width || r.bottom - r.top != height)    // don't move left and top if true
            SetWindowPos(hwnd, NULL, r.left, r.top, rect->right - r.left, rect->bottom - r.top, flags);
        else    // move to new position as well if false
            SetWindowPos(hwnd, NULL, rect->left - borderXl, rect->top, 0, 0, flags | SWP_NOSIZE);
    }

    std::pair<HMONITOR, MONITORINFO> getMonitor(HWND hwnd)
    {
        HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
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

    void addRectByAlignment(LPRECT target, LPRECT rect, int value, Alignment alignment, bool invertTarget=false, bool invertRect=false)
    {
        int addend = 0;
        switch (alignment) {
            case Alignment::LEFT:
                if (invertRect)
                    addend = rect->right + value;
                else
                    addend = rect->left - value;

                if (invertTarget)
                    target->right = addend;
                else
                    target->left = addend;
                break;

            case Alignment::TOP:
                if (invertRect)
                    addend = rect->bottom + value;
                else
                    addend = rect->top - value;

                if (invertTarget)
                    target->bottom = addend;
                else
                    target->top = addend;
                break;

            case Alignment::RIGHT:
                if (invertRect)
                    addend = rect->left - value;
                else
                    addend = rect->right + value;

                if (invertTarget)
                    target->left = addend;
                else
                    target->right = addend;
                break;

            case Alignment::BOTTOM:
                if (invertRect)
                    addend = rect->top - value;
                else
                    addend = rect->bottom + value;

                if (invertTarget)
                    target->top = addend;
                else
                    target->bottom = addend;
                break;
            default:
                break;
        };
    }
}



MainWindow::MainWindow (QWidget* parent) : QMainWindow (parent)
{
    mainLayout = new QBoxLayout(QBoxLayout::TopToBottom);
    buttonLayout = new FlowLayout();

    QPushButton* rearrBtn = new QPushButton("reorder layout");
    connect(rearrBtn, &QPushButton::clicked, this, [&](){rearrangeScreen(Alignment::RIGHT);});
    mainLayout->addWidget(rearrBtn);

    readProfileFile();
    QWidget* central_w = new QWidget(this);
    QFrame* frame = new QFrame();
    frame->setFrameStyle(QFrame::Panel);
    frame->setLayout(buttonLayout);
    mainLayout->addWidget(frame);
    central_w->setLayout(mainLayout);
    setCentralWidget(central_w);

    connect(this, &rearranged, buttonLayout, &FlowLayout::reorient);

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
    setWindowTitle("TouchMacros");

    handle = (HWND)winId();     // get window handle of the app from identifier
    SetWindowLong(handle, GWL_EXSTYLE, GetWindowLong(handle, GWL_EXSTYLE) | WS_EX_NOACTIVATE  | WS_EX_APPWINDOW);    // add NoActive to window style through Windows API to prevent it from drawing keyboard focus
}


void MainWindow::readProfileFile(QString filename)
{
    QFile file(filename);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "Could not open the file for reading";
        return;
    }

    buttons.clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QStringList line;
        line = in.readLine().split(';');
        bool validKeycode = false;

        std::vector<WORD> keycodes;
        QStringList keyStrings = line[1].trimmed().split('+');
        QString mode = line[2].trimmed();

        for (const auto& key : keyStrings) {
            keycodes.push_back(key.toInt(&validKeycode, 16));
        }
        if (mode == "K"){
            buttons.append(new Key(line[0].trimmed(), keycodes));
        }
        if (mode == "M") {
            buttons.append(new ModifierKey(line[0].trimmed(), keycodes));
        }

        if (validKeycode) {
            buttonLayout->addWidget(buttons.back());
        }
    }

    file.close();
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


void MainWindow::repositionOther(int sizeLeftFree, int compareOffset)
{
    UINT dpiFactor = GetDpiForWindow(handle) / USER_DEFAULT_SCREEN_DPI;
    int sizeDPI = sizeLeftFree * dpiFactor;  // account for different dpi for scaled displays

    std::vector<HWND> windows = getWindowsOnMonitor(monitorHndl);

    RECT newWinRect;
    RECT appRect;
    GetWindowRect(handle, &appRect);

    // ensure windows don't ignore resized app
    if (compareOffset > 0){
            addRectByAlignment(&appRect, &appRect, compareOffset * dpiFactor, alignment, true, true);
    }

    for (int i=windows.size(); i>0; --i)    // traverse in reverse to preserve z-order when using SetWindowPos
    {
        HWND hwnd = windows[i];
        GetWindowRect(hwnd, &newWinRect);

        WINDOWPLACEMENT wplc;
        wplc.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &wplc);
        bool fullscreen = wplc.showCmd == SW_MAXIMIZE;

        RECT r;
        if (hwnd != handle && IntersectRect(&r, &appRect, &newWinRect)){

            if (fullscreen)
                CopyRect(&newWinRect, &screenspaceRect);

            addRectByAlignment(&newWinRect, &screenspaceRect, -sizeDPI + 1, alignment);
            repositionWin(hwnd, &newWinRect, SWP_NOZORDER | SWP_NOACTIVATE, alignment, fullscreen);
        }
    }
}


void MainWindow::repositionSelf()
{
    RECT rect;
    int sizeDPI = ratioScreenRect() * GetDpiForWindow(handle) / USER_DEFAULT_SCREEN_DPI;  // account for different dpi for scaled displays

    CopyRect(&rect, &screenspaceRect);
    addRectByAlignment(&rect, &screenspaceRect, -sizeDPI, alignment, true);

    repositionWin(handle, &rect, SWP_SHOWWINDOW, alignment, true);
    updateGeometry();
}


int MainWindow::ratioScreenRect()
{
    double minW, minH;
    double dpiFactor = layoutRatio / GetDpiForWindow(handle) * USER_DEFAULT_SCREEN_DPI;
    switch (alignment) {
    case Alignment::LEFT:
    case Alignment::RIGHT:
        minW = frameGeometry().width() - width() + minimumWidth() + 2;
        return std::max((screenspaceRect.right - screenspaceRect.left) * dpiFactor, minW);
    case Alignment::TOP:
    case Alignment::BOTTOM:
        minH = frameGeometry().height() - height() + minimumHeight() + 2;
        return std::max((screenspaceRect.bottom - screenspaceRect.top) * dpiFactor, minH);
    default:
        return (size().width());
    };
}


void MainWindow::rearrangeScreen(Alignment a)
{
    alignment = a;
    if (alignment == Alignment::LEFT || alignment == Alignment::RIGHT){
        mainLayout->setDirection(QBoxLayout::TopToBottom);
        emit rearranged(Qt::Vertical);
    }
    else{
        mainLayout->setDirection(QBoxLayout::LeftToRight);
        emit rearranged(Qt::Horizontal);
    }
    std::pair<HMONITOR, MONITORINFO> monitor = getMonitor(handle);
    monitorHndl = monitor.first;
    screenspaceRect = monitor.second.rcWork;

    repositionSelf();
}


void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (CheckAlignment()){
        if (alignment == Alignment::LEFT || alignment == Alignment::RIGHT)
            repositionOther(e->size().width(), e->oldSize().width() - e->size().width());
        else
            repositionOther(frameGeometry().height(), e->oldSize().height() - e->size().height());
    }
}


void MainWindow::closeEvent(QCloseEvent *e)
{
    QApplication::quit();   //quit on closure -> prevent app from continuing to run in the background after gui is discarded; important if app is Tool window
    QMainWindow::closeEvent(e);
}


MainWindow::~MainWindow ()
{
    for (unsigned int i = 0; i < buttons.size(); i++)
        delete buttons[i];
}
