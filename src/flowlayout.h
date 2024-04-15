#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QStyle>
#include <QRect>

class FlowLayout : public QLayout
{
public:
    FlowLayout(QWidget* parent = nullptr);
    void addItem(QLayoutItem *item) override;
    Qt::Orientation orientation();
    void setOrientation(Qt::Orientation o);
    int count() const override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;

protected:
    Qt::Orientation _orientation = Qt::Vertical;
    int marginHorizontal = 2;
    int marginVertical = 2;
    int columnWidth = 75;
    int rowHeight = 25;
    QList<QLayoutItem*> itemList;

    void positionItems();
};

#endif // FLOWLAYOUT_H
