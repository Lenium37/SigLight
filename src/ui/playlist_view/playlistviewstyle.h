#ifndef PLAYLISTVIEWSTYLE_H
#define PLAYLISTVIEWSTYLE_H

#include <QProxyStyle>
#include <QStyleOption>

class PlaylistViewStyle : public QProxyStyle
{

public:
    PlaylistViewStyle(QStyle* style = nullptr);

private:
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const;
};

#endif // PLAYLISTVIEWSTYLE_H
