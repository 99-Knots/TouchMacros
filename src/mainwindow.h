#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "key.h"

enum class Alignment
{
    NONE = 0,
    LEFT,
    TOP,
    RIGHT,
    BOTTOM
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    HWND handle;
    int win_width = 250;    // todo: find way to get good default value to
    Alignment alignment = Alignment::NONE;
    bool suppressResize = false;
    static constexpr int BUTTON_COUNT = 5;
    Key* buttonArray[BUTTON_COUNT];

    bool CheckAlignment();
    void rearrangeScreen();
    void repositionOther(int widthLeftFree, int comparisonWidth=0);
    void repositionSelf(int width);
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
