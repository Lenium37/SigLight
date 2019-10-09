#include "playlistviewstyle.h"

PlaylistViewStyle::PlaylistViewStyle(QStyle *style)
    : QProxyStyle(style)
{
}

void PlaylistViewStyle::drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == QStyle::PE_IndicatorItemViewItemDrop && !option->rect.isNull()){
            QStyleOption opt(*option);
            opt.rect.setLeft(0);
            if (widget) opt.rect.setRight(widget->width());
            QProxyStyle::drawPrimitive(element, &opt, painter, widget);
            return;
        }
        QProxyStyle::drawPrimitive(element, option, painter, widget);
}



