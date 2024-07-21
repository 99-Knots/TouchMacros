#ifndef FLOWLAYOUT_H
#define FLOWLAYOUT_H

#include <QLayout>
#include <QStyle>
#include <QRect>

class FlowLayout : public QLayout
{
    Q_OBJECT

public:
    FlowLayout(QWidget* parent = nullptr, Qt::Orientation o = Qt::Vertical);
    void addItem(QLayoutItem *item) override;
    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation o);
    int count() const override;
    QSize minimumSize() const override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;
    QLayoutItem *itemAt(int index) const override;
    void setGeometry(const QRect &rect) override;
    int spacing() const;
    void setSpacing();

protected:
    int horizontalSpacing = 2;
    int verticalSpacing = 2;
    int itemWidth = 50;
    int itemHeight = 50;
    int numRows = 1;
    int numColumns = 1;
    int columnWidth() const;
    int rowHeight() const;
    QList<QLayoutItem*> itemList;

    void positionItems();

private:
    Qt::Orientation m_orientation;

public slots:
    void reorient(Qt::Orientation o);
};

#endif // FLOWLAYOUT_H
