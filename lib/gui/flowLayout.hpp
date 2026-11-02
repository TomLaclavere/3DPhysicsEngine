/**
 * @file flowLayout.hpp
 * @brief Layout that places widgets left to right and wraps them onto new lines when space runs out.
 */
#pragma once
#include <QLayout>
#include <QList>

/**
 * @class FlowLayout
 * @brief Row layout that wraps its items, so a row of widgets fits any available width.
 *
 * Items keep their preferred size and move to the next line when the current one is full.
 * The minimum width is that of the widest item, and the height follows the width
 * (height-for-width).
 */
class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget* parent = nullptr);
    ~FlowLayout() override;

    void                addItem(QLayoutItem* item) override;
    int                 count() const override;
    QLayoutItem*        itemAt(int index) const override;
    QLayoutItem*        takeAt(int index) override;
    Qt::Orientations    expandingDirections() const override;
    bool                hasHeightForWidth() const override;
    int                 heightForWidth(int width) const override;
    QSize               minimumSize() const override;
    QSize               sizeHint() const override;
    void                setGeometry(const QRect& rect) override;

private:
    int doLayout(const QRect& rect, bool testOnly) const;
    int spacingFor(const QLayoutItem* item, Qt::Orientation orientation) const;

    QList<QLayoutItem*> m_items;
};
