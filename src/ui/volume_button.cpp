#include "volume_button.h"

#include <QtWidgets>

Volume_button::Volume_button(QWidget *parent) :
    QToolButton(parent)
{
    setIcon(QIcon(":/icons_svg/svg/iconfinder_014_-_Volume_2949882.svg"));
    setPopupMode(QToolButton::InstantPopup);

    QWidget *popup = new QWidget(this);
    popup->resize(300, 150);

    slider = new QSlider(Qt::Horizontal, popup);
    slider->setRange(0, 100);
    slider->setValue(50);

    connect(slider, &QAbstractSlider::valueChanged, this, &Volume_button::volume_changed);

    label = new QLabel(popup);
    label->setAlignment(Qt::AlignCenter);
    label->setNum(50);
    label->setMinimumWidth(label->sizeHint().width());

    connect(slider, &QAbstractSlider::valueChanged, label, QOverload<int>::of(&QLabel::setNum));

    QBoxLayout *popupLayout = new QHBoxLayout(popup);
    popupLayout->setMargin(2);
    popupLayout->addWidget(slider);
    popupLayout->addWidget(label);


    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(popup);


    menu = new QMenu(this);
    menu->addAction(action);
    setMenu(menu);
}

void Volume_button::increase_volume()
{
    slider->triggerAction(QSlider::SliderPageStepAdd);
}

void Volume_button::descrease_volume()
{
    slider->triggerAction(QSlider::SliderPageStepSub);
}


int Volume_button::volume() const
{
    return slider->value();
}


void Volume_button::set_volume(int volume)
{
    slider->setValue(volume);
}
