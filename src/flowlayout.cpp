#include "flowlayout.h"
#include <QtWidgets>

FlowLayout::FlowLayout(QWidget *parent) : QLayout(parent)
{
    setOrientation(Qt::Vertical);
}

Qt::Orientation FlowLayout::orientation() const
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
    if (orientation() & Qt::Vertical){
        int maxRowCount = std::max(geometry().height() / rowHeight, 1);
        int maxColumnCount = count() / maxRowCount + (count() % maxRowCount != 0);
        return QSize(maxColumnCount * columnWidth, rowCount * rowHeight);
    }
    else{
        int maxColumnCount = std::max(geometry().width() / columnWidth, 1);
        int maxRowCount = count() / maxColumnCount + (count() % maxColumnCount != 0);
        return QSize(columnCount * columnWidth, maxRowCount * rowHeight);
    }
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
    int sectionNr, w, h;
    if (orientation() & Qt::Vertical) {
        sectionNr = std::min(geometry().width()/columnWidth, count());
        columnCount = std::min(geometry().width()/columnWidth, count());
        rowCount = (count() / sectionNr + (count() % sectionNr != 0));
        w = std::max(geometry().width() / sectionNr, columnWidth);
        h = geometry().height() / (count() / sectionNr + (count() % sectionNr != 0));
    }
    else {
        sectionNr = std::min(geometry().height()/rowHeight, count());
        columnCount = (count() / sectionNr + (count() % sectionNr != 0));
        rowCount = std::min(geometry().height()/rowHeight, count());
        h = geometry().height()/sectionNr;
        w = std::max(geometry().width() / (count() / sectionNr + (count() % sectionNr != 0)), columnWidth);
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
    update();
}
