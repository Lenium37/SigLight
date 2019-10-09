#ifndef NOT_EDITABLE_DELEGATE_H
#define NOT_EDITABLE_DELEGATE_H

#include <QItemDelegate>

class Not_editable_delegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit Not_editable_delegate(QObject *parent = 0)
        : QItemDelegate(parent)
    {}

protected:
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
    { return false; }
    QWidget* createEditor(QWidget *, const QStyleOptionViewItem &, const QModelIndex &) const
    { return Q_NULLPTR; }

};

#endif // NOT_EDITABLE_DELEGATE_H
