/**
 * @file flowLayout.cpp
 * @brief Implementation of FlowLayout: wrapping row layout.
 */
#include "gui/flowLayout.hpp"

#include <QApplication>
#include <QStyle>
#include <QWidget>
#include <algorithm>

FlowLayout::FlowLayout(QWidget* parent)
    : QLayout(parent)
{}

FlowLayout::~FlowLayout()
{
    while (QLayoutItem* item = takeAt(0))
        delete item;
}

void FlowLayout::addItem(QLayoutItem* item)
{
    m_items.append(item);
}

int FlowLayout::count() const
{
    return static_cast<int>(m_items.size());
}

QLayoutItem* FlowLayout::itemAt(int index) const
{
    return m_items.value(index);
}

QLayoutItem* FlowLayout::takeAt(int index)
{
    return (index >= 0 && index < m_items.size()) ? m_items.takeAt(index) : nullptr;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    return doLayout(QRect(0, 0, width, 0), true);
}

/// @brief Minimum size: the largest item, so the layout can wrap down to one item per line.
QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (const QLayoutItem* item : m_items)
        size = size.expandedTo(item->minimumSize());
    const QMargins m = contentsMargins();
    return size + QSize(m.left() + m.right(), m.top() + m.bottom());
}

/// @brief Preferred size: all items on a single line.
QSize FlowLayout::sizeHint() const
{
    int width  = 0;
    int height = 0;
    for (const QLayoutItem* item : m_items)
    {
        if (width > 0)
            width += spacingFor(item, Qt::Horizontal);
        width += item->sizeHint().width();
        height = std::max(height, item->sizeHint().height());
    }
    const QMargins m = contentsMargins();
    return QSize(width, height) + QSize(m.left() + m.right(), m.top() + m.bottom());
}

void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

/// @brief Place the items line by line within @p rect; returns the height used.
int FlowLayout::doLayout(const QRect& rect, bool testOnly) const
{
    const QMargins m    = contentsMargins();
    const QRect    area = rect.marginsRemoved(m);
    int            x    = area.x();
    int            y    = area.y();
    int            lineHeight = 0;

    for (QLayoutItem* item : m_items)
    {
        // An item wider than the line shrinks to it (down to its minimum width)
        const QSize hint  = item->sizeHint();
        const int   width = std::max(item->minimumSize().width(), std::min(hint.width(), area.width()));

        if (x > area.x() && x + width > area.x() + area.width())
        {
            x = area.x();
            y += lineHeight + spacingFor(item, Qt::Vertical);
            lineHeight = 0;
        }
        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), QSize(width, hint.height())));

        x += width + spacingFor(item, Qt::Horizontal);
        lineHeight = std::max(lineHeight, hint.height());
    }
    return y + lineHeight - rect.y() + m.bottom();
}

/// @brief Spacing set on the layout, or else the style's spacing for this kind of widget.
int FlowLayout::spacingFor(const QLayoutItem* item, Qt::Orientation orientation) const
{
    if (spacing() >= 0)
        return spacing();
    const QWidget* widget = item->widget();
    const QStyle*  style  = widget ? widget->style() : QApplication::style();
    const auto     type   = widget ? widget->sizePolicy().controlType() : QSizePolicy::DefaultType;
    return std::max(0, style->layoutSpacing(type, type, orientation));
}
