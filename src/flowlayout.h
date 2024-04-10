#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>

class FlowLayout : public QLayout
{
    Q_OBJECT
public:
    FlowLayout();

protected:
    QList<QLayoutItem> itemList;
};

#endif // FLOWLAYOUT_H
