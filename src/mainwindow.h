#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "key.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    HWND handle;
    int win_width = 150;
    static constexpr int BUTTON_COUNT = 5;
    Key* buttonArray[BUTTON_COUNT];

    int isDocked();
    void rearrangeScreen();
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
