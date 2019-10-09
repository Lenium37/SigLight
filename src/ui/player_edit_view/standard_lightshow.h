#ifndef STANDARD_LIGHTSHOW_H
#define STANDARD_LIGHTSHOW_H

#include <QGraphicsItem>
#include <QPainter>
#include <QWidget>
#include <QStyleOption>
#include <QGraphicsSceneMouseEvent>
#include <QDialog>
#include <QLabel>

class Standard_lightshow  : public QObject, public QGraphicsItem
{

    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    Standard_lightshow(int scene_width, int tracks_height);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /* Editing the Lightshow has been shut down
protected:
    void mouseDoubleClickEvent (QGraphicsSceneMouseEvent *);

signals:
    void item_double_clicked(Standard_lightshow *clicked_item);
*/

private:
    int width;
    int height;
    QString fixture_name;

};

#endif // STANDARD_LIGHTSHOW_H
