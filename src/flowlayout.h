#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QStyle>
#include <QRect>

class FlowLayout : public QLayout
{
public:
    FlowLayout(QWidget* parent = nullptr, Qt::Orientation o = Qt::Horizontal);
    void addItem(QLayoutItem *item) override;
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation o);
    int count() const override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;

protected:
    Qt::Orientation _orientation;
    int marginHorizontal = 2;
    int marginVertical = 2;
    int itemWidth = 75;
    int itemHeight = 25;
    int numRows = 1;
    int numColumns = 1;
    int columnWidth() const;
    int rowHeight() const;
    QList<QLayoutItem*> itemList;

    void positionItems();
};

#endif // FLOWLAYOUT_H
