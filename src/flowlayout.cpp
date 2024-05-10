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

    int defaultNum = 3;
    if (orientation() & Qt::Vertical){
        size += QSize(defaultNum * columnWidth(), (count()/defaultNum + (count()%defaultNum != 0)) * rowHeight());
    }
    else{
        size += QSize((count()/defaultNum + (count()%defaultNum != 0)) * columnWidth(), defaultNum * rowHeight());
    }
    return size;
}

QSize FlowLayout::minimumSize() const
{
    if (contentsRect().isEmpty())
        return sizeHint();

    QSize size(contentsMargins().left() + contentsMargins().right(), contentsMargins().top() + contentsMargins().bottom());
    int maxRowCount, maxColumnCount;

    if (orientation() & Qt::Vertical){
        maxRowCount = std::clamp(contentsRect().height() / rowHeight(), 1, count());
        maxColumnCount = count() / maxRowCount + (count() % maxRowCount != 0);
        size += QSize(maxColumnCount * columnWidth(), numRows * rowHeight());
    }
    else{
        maxColumnCount = std::clamp(contentsRect().width() / columnWidth(), 1, count());
        maxRowCount = count() / maxColumnCount + (count() % maxColumnCount != 0);
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

    if (orientation() & Qt::Vertical) {
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
            if (orientation() & Qt::Vertical)
                itemRect.translate(0, h);
            else
                itemRect.translate(w, 0);
        }
        if (orientation() & Qt::Vertical)
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
