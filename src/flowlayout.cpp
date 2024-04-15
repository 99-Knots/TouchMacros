#include "flowlayout.h"
#include <QtWidgets>

FlowLayout::FlowLayout(QWidget *parent) : QLayout(parent)
{
    setOrientation(Qt::Horizontal);
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
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    return QSize(columnWidth, rowHeight);
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
    int sectionNr;
    int w = columnWidth;
    int h = rowHeight;
    if (orientation() & Qt::Vertical) {
        sectionNr = geometry().width()/columnWidth;
        w = geometry().width()/std::min(sectionNr, count());
    }
    else {
        sectionNr = geometry().height()/rowHeight;
        h = geometry().height()/std::min(sectionNr, count());
    }
    int counter = 0;
    QRect itemRect(0, 0, w, h);
    itemRect = itemRect.marginsRemoved(QMargins(marginHorizontal, marginVertical, marginHorizontal, marginVertical));

    for (const auto& item : itemList){
        if (counter >= sectionNr) {
            counter = 0;
            if (orientation() & Qt::Vertical)
                itemRect.translate(0, h);
            else
                itemRect.translate(w, 0);
        }
        if (orientation() & Qt::Vertical)
            itemRect.moveLeft(counter * w + marginHorizontal);
        else
            itemRect.moveTop(counter * h + marginVertical);
        counter++;
        item->setGeometry(itemRect);
    }
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    positionItems();
}
