#include "flowlayout.h"
#include <QtWidgets>

FlowLayout::FlowLayout(QWidget *parent, Qt::Orientation o) : QLayout(parent)
{
    setOrientation(o);
}

Qt::Orientation FlowLayout::orientation() const
{
    return m_orientation;
}

void FlowLayout::setOrientation(Qt::Orientation o)
{
    m_orientation = o;
}

void FlowLayout::reorient(Qt::Orientation o)
{
    setOrientation(o);
    if (orientation() & Qt::Horizontal) {
        numRows = 1;
        numColumns = count();
    }
    else {
        numRows = count();
        numColumns = 1;
    }
}

int FlowLayout::columnWidth() const
{
    return horizontalSpacing * 2 + itemWidth;
}

int FlowLayout::rowHeight() const
{
    return verticalSpacing * 2 + itemHeight;
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
    QSize size(contentsMargins().left() + contentsMargins().right(), contentsMargins().top() + contentsMargins().bottom());

    if (orientation() & Qt::Horizontal){
        return size + QSize((count()/2 + (count()%2 != 0)) * columnWidth(), 2 * rowHeight());
    }
    else{
        return size + QSize(2 * columnWidth(), (count()/2 + (count()%2 != 0)) * rowHeight());
    }
}

QSize FlowLayout::minimumSize() const
{
    QSize size(contentsMargins().left() + contentsMargins().right(), contentsMargins().top() + contentsMargins().bottom());

    if (orientation() & Qt::Horizontal){
        int maxRowCount = std::max(contentsRect().height() / rowHeight(), 1);
        int maxColumnCount = count() / maxRowCount + (count() % maxRowCount != 0);
        size += QSize(maxColumnCount * columnWidth(), numRows * rowHeight());
    }
    else{
        int maxColumnCount = std::max(contentsRect().width() / columnWidth(), 1);
        int maxRowCount = count() / maxColumnCount + (count() % maxColumnCount != 0);
        size += QSize(numColumns * columnWidth(), maxRowCount * rowHeight());
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
    int numSections, w, h;
    QPoint start = contentsRect().topLeft();
    int contentsWidth = contentsRect().width();
    int contentsHeight = contentsRect().height();

    if (orientation() & Qt::Horizontal) {
        numColumns = std::min(contentsWidth/columnWidth(), count());
        numRows = (count() / numColumns + (count() % numColumns != 0));
        w = std::max(contentsWidth / numColumns, columnWidth());
        h = std::max(contentsHeight / numRows, rowHeight());
        numSections = numColumns;
    }
    else {
        numRows = std::min(contentsHeight/rowHeight(), count());
        numColumns = (count() / numRows + (count() % numRows != 0));
        w = std::max(contentsWidth / numColumns, columnWidth());
        h = std::max(contentsHeight / numRows, rowHeight());
        numSections = numRows;
    }

    int counter = 0;
    QRect itemRect(start.x(), start.y(), w, h);
    itemRect = itemRect.marginsRemoved(QMargins(horizontalSpacing, verticalSpacing, horizontalSpacing, verticalSpacing));

    for (const auto& item : itemList){
        if (counter >= numSections) {
            counter = 0;
            if (orientation() & Qt::Horizontal)
                itemRect.translate(0, h);
            else
                itemRect.translate(w, 0);
        }
        if (orientation() & Qt::Horizontal)
            itemRect.moveLeft(counter * w + start.x() + horizontalSpacing);
        else
            itemRect.moveTop(counter * h + start.y() + verticalSpacing);

        counter++;
        item->setGeometry(itemRect);
    }
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    positionItems();
    parentWidget()->updateGeometry();
}
