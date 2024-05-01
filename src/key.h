#ifndef KEY_H
#define KEY_H

#include <QPushButton>
#include <windows.h>


class Key : public QPushButton
{
    Q_OBJECT

public:
    Key(QString name = "None", std::vector<WORD> keycodes = {0x99}, QWidget* parent = nullptr);
    ~Key();

protected:
    std::vector<INPUT> inputs;

    void pressKey();
    void releaseKey();

protected slots:
    virtual void onClick();
};


class ModifierKey : public Key{
    Q_OBJECT

public:
    ModifierKey(QString name = "None", std::vector<WORD> keycodes = {0x99}, QWidget* parent = nullptr);

protected slots:
    void onClick() override;
};

#endif // KEY_H
