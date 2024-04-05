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
    double layoutRatio = 0.10;
    Alignment alignment = Alignment::NONE;
    RECT screenspaceRect = RECT{0, 0, 0, 0};
    HMONITOR monitorHndl = NULL;
    bool suppressResize = false;
    static constexpr int BUTTON_COUNT = 5;
    std::vector<Key*> buttons;
    Key* buttonArray[BUTTON_COUNT];
    QLayout* mainLayout;

    void readProfileFile(QString filename=":/res/keycode_defaults");
    bool CheckAlignment();
    int ratioScreenRect();
    void rearrangeScreen(Alignment a=Alignment::RIGHT);
    void repositionOther(int sizeLeftFree, int sizeDiff=0);
    void repositionSelf(int width);
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
