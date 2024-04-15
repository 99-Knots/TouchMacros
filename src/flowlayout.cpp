#include "flowlayout.h"
#include <QtWidgets>

FlowLayout::FlowLayout(QWidget *parent) : QLayout(parent)
{
    setOrientation(Qt::Vertical);
}

Qt::Orientation FlowLayout::orientation()
{
    return _orientation;
}

void FlowLayout::setOrientation(Qt::Orientation o)
{
    _orientation = o;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int FlowLayout::count() const
{
    return itemList.size();
}

QSize FlowLayout::sizeHint() const
{
    QSize size(0, 0);
    for (const auto& item : itemList) {
        size = size.expandedTo(item->minimumSize());
    }
    return size;
}

QLayoutItem *FlowLayout::takeAt(int index) {
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    return nullptr;
}

QLayoutItem *FlowLayout::itemAt(int index) const {
    if (index >= 0 && index < itemList.size())
        return itemList.at(index);
    return nullptr;
}

void FlowLayout::positionItems()
{
    QRect geo = geometry();
    int columnNr = geo.width()/columnWidth;
    int columnCounter = 0;
    int w = geo.width()/columnNr;
    QRect itemRect(0, 0, w, rowHeight);
    itemRect = itemRect.marginsRemoved(QMargins(marginHorizontal, marginVertical, marginHorizontal, marginVertical));

    for (const auto& item : itemList){
        if (columnCounter >= columnNr) {
            columnCounter = 0;
            itemRect.translate(0, rowHeight);
        }
        itemRect.moveLeft(columnCounter * w + marginHorizontal);
        columnCounter++;
        item->setGeometry(itemRect);
    }
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    positionItems();
}
