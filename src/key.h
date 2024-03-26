#ifndef KEY_H
#define KEY_H

#include <QPushButton>
#include <windows.h>


class Key : public QPushButton
{
    Q_OBJECT

public:
    Key (QString name = "None", WORD keycode = 0x99, QWidget* parent = nullptr);
    ~Key();

protected:
    INPUT input;

    void pressKey();
    void releaseKey();

protected slots:
    virtual void onClick();
};


class ModifierKey : public Key{
    Q_OBJECT

public:
    ModifierKey(QString name = "None", WORD keycode = 0x99, QWidget* parent = nullptr);

protected slots:
    void onClick() override;
};


#endif // KEY_H
