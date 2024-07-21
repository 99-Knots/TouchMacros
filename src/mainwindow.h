#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "key.h"
#include "flowlayout.h"

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
    QList<Key*> buttons;
    FlowLayout* buttonLayout;
    QBoxLayout* mainLayout;
    QToolBar* toolbar;

    void readProfileFile(QString filename=":/res/keycode_defaults");
    bool CheckAlignment();
    int ratioScreenRect();
    void rearrangeScreen(Alignment a=Alignment::RIGHT);
    void repositionOther(int sizeLeftFree, int sizeDiff=0);
    void repositionSelf();
    void resizeEvent(QResizeEvent *e);
    void closeEvent(QCloseEvent *e);

signals:
    void rearranged(Qt::Orientation o);
};

#endif // MAINWINDOW_H
