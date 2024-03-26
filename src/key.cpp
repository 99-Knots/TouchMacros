#include "key.h"

Key::Key (QString name, WORD keycode, QWidget* parent) : QPushButton(parent)
{
    input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = keycode;
    setText(name);
    connect(this, &QPushButton::clicked, this, &Key::onClick);
}

void Key::pressKey ()
{
    SendInput(1, &input, sizeof(INPUT));
}

void Key::releaseKey()
{
    input.ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
    input.ki.dwFlags = 0;
}

void Key::onClick()
{
    pressKey();
    Sleep(100);
    releaseKey();
}

Key::~Key()
{
    releaseKey();
}


ModifierKey::ModifierKey (QString name, WORD keycode, QWidget* parent) : Key(name, keycode, parent)
{
    setCheckable(true);
}

void ModifierKey::onClick()
{
    if (isChecked())
        pressKey();
    else
        releaseKey();
}
