#include "key.h"


Key::Key (QString name, std::vector<WORD> keycodes, QWidget* parent) : QPushButton(parent)
{
    setText(name);
    for (unsigned int i=0; i<keycodes.size(); i++) {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = keycodes[i];
        inputs.push_back(input);
    }
    connect(this, &QPushButton::clicked, this, &Key::onClick);
}

void Key::pressKey()
{
    SendInput(inputs.size(), &inputs[0], sizeof(INPUT));
}

void Key::releaseKey()
{
    for (unsigned int i=0; i<inputs.size(); i++)
        inputs[i].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(inputs.size(), &inputs[0], sizeof(INPUT));

    for (unsigned int i=0; i<inputs.size(); i++)
        inputs[i].ki.dwFlags = 0;
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


ModifierKey::ModifierKey (QString name, std::vector<WORD> keycodes, QWidget* parent) : Key(name, keycodes, parent)
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
