#include <iostream>
#include "createfixturedialog.h"
#include "ui_createfixturedialog.h"

CreateFixtureDialog::CreateFixtureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateFixtureDialog) {
  ui->setupUi(this);

  icons << "beam" << "fan" << "flower" << "hazer" << "lamp" << "laser" << "ledbar_beams" << "ledbar_pixels" << "movinghead" << "other" << "scanner" << "smoke" << "strobe";
  types /*<< "Action"*/ << "Ambient" << "Bass" << "Mid" << "High";

  for (QString icon : icons) {
   ui->cB_icons->addItem(QIcon(":/icons_svg/svg/" + icon + ".svg"), "");
  }
  ui->cB_type->addItems(types);


  ui->cB_icons->setCurrentIndex(4);
  ui->cB_type->setCurrentIndex(3);
  ui->l_start_channel->setVisible(false);
  ui->sB_start_cahnnel->setVisible(false);
  is_edit_view = false;
  this->setWindowTitle("Create Fixturepreset");

  QWidget *scrollarea_content = new QWidget(this);
  scrollarea_content->setLayout(new QVBoxLayout());
  QScrollArea *scrollArea = new QScrollArea;
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
  scrollArea->setWidgetResizable( true );
  scrollArea->setWidget( scrollarea_content );
  ui->vL_scrollarea->addWidget(scrollArea);
  on_pB_add_channel_clicked();
}

CreateFixtureDialog::~CreateFixtureDialog() {
    delete ui;
}

void CreateFixtureDialog::create_fixture_dialog_finished(std::string & name, std::string & type, std::string & description, QStringList & channels, std::string &icon)
{
    name = (ui->lE_name->text().size() == 0)? "" : ui->lE_name->text().toStdString();
    type = ui->cB_type->currentText().toStdString();
    description = (ui->lE_description->text().size() == 0)? "" : ui->lE_description->text().toStdString();
    icon = icons.at(ui->cB_icons->currentIndex()).toStdString();
    QStringList _channels;
    QWidget * scrollarea_content = qobject_cast<QScrollArea *>(ui->vL_scrollarea->itemAt(0)->widget())->widget();
    QVBoxLayout *vL_channels = qobject_cast<QVBoxLayout *>(scrollarea_content->layout());
    int channel_count = vL_channels->count();
    for (int i = 0; i < channel_count ; i++) {
        QString channel;
        QLayout * layout_channel = qobject_cast<QHBoxLayout *>(*std::next(vL_channels->children().begin(), i));
        int function_count = qobject_cast<QLayout*>(qobject_cast<QLayout *>(*layout_channel->children().begin())->children().at(1))->children().count();
        for (int k = 0;k < function_count; k++) {
            QString function;
            QLayout * function_layout = qobject_cast<QLayout*>(qobject_cast<QLayout *>(*layout_channel->children().begin())->children().at(1));
            for (int j = 0; j < 3; j++){
                if( !qobject_cast<QLineEdit *>(qobject_cast<QHBoxLayout *>(function_layout->children().at(k))->itemAt(0)->widget())->text().isEmpty()
                    && !qobject_cast<QLineEdit *>(qobject_cast<QHBoxLayout *>(function_layout->children().at(k))->itemAt(1)->widget())->text().isEmpty()
                    && !qobject_cast<QLineEdit *>(qobject_cast<QHBoxLayout *>(function_layout->children().at(k))->itemAt(2)->widget())->text().isEmpty())
                {
                    if (!function.isEmpty()) { function.append("~");}
                    function.append((qobject_cast<QLineEdit *>(qobject_cast<QHBoxLayout *>(function_layout->children().at(k))->itemAt(j)->widget())->text().isEmpty())
                    ? "" : qobject_cast<QLineEdit *>(qobject_cast<QHBoxLayout *>(function_layout->children().at(k))->itemAt(j)->widget())->text());
                }
            }
            if(!function.isEmpty()){
                if(!channel.isEmpty()){channel.append("|");}
                channel.append(function);
                function = "";
            }
        }
        if(!channel.isEmpty()) {
            _channels << channel;
            channel = "";
        }
    }
    channels = _channels;
}

void CreateFixtureDialog::edit_fixture_dialog(Fixture edit_fixture, std::list<int> blocked_channels, bool edit_preset)
{
    is_edit_view = true;
    if(!edit_preset){
        CreateFixtureDialog::blocked_channels = blocked_channels;
        last_channel = edit_fixture.get_channel_count();
        ui->l_start_channel->setVisible(true);
        ui->sB_start_cahnnel->setVisible(true);
        ui->sB_start_cahnnel->setRange(1,512);
        ui->sB_start_cahnnel->setValue(edit_fixture.get_start_channel());
        original_start_channel = edit_fixture.get_start_channel();
        this->setWindowTitle("Edit Fixture");
    } else {
        this->setWindowTitle("Edit Fixturepreset");
    }
    ui->lE_name->setText(QString::fromStdString(edit_fixture.get_name()));
    ui->cB_icons->setCurrentIndex(icons.indexOf(QString::fromStdString(edit_fixture.get_icon())));
    ui->cB_type->setCurrentIndex(types.indexOf(QString::fromStdString(edit_fixture.get_type())));
    ui->lE_description->setText(QString::fromStdString(edit_fixture.get_description()));

    QWidget * scrollarea_content = qobject_cast<QScrollArea *>(ui->vL_scrollarea->itemAt(0)->widget())->widget();
    QLayout *vL_channels = qobject_cast<QLayout *>(scrollarea_content->children().at(0));

    int channel_count_edit = edit_fixture.get_channel_count() ;
    Channel_gui channel =  *edit_fixture.get_channels().begin();

    for(int i = 0; i < channel_count_edit; i++) {
        if(i != 0){
            on_pB_add_channel_clicked();
        }
        QLayout * layout_channel = qobject_cast<QLayout *>(vL_channels->children().at(i));
        QLayout * function_layout = qobject_cast<QLayout *>(layout_channel->children().at(0)->children().at(1));

        auto functions = (channel).get_function_list();
        auto function = functions.begin();
        int function_count = functions.size();
        for(int j = 0; j < function_count; j++) {

            if(j != 0) {
                on_pB_add_function_generated_clicked(function_layout);
            }

            auto function_parameter = function->to_string().split("~");
            for(int k = 0; k < 3; k++){
                qobject_cast<QLineEdit *>(qobject_cast<QLayout *>(function_layout->children().at(j))->itemAt(k)->widget())->setText(function_parameter.at(k));
            }
            function++;
        }
        if(i < (channel_count_edit - 1)) {
            channel = *std::next(edit_fixture.get_channels().begin(), i + 1);
        }

    }
}

void CreateFixtureDialog::edit_fixture_finished(Fixture &edited_fixture)
{
    std::string name;
    std::string type;
    std::string description;
    std::string icon;
    QStringList channels;
    create_fixture_dialog_finished(name, type, description, channels, icon);
    edited_fixture.set_name(name);
    edited_fixture.set_type(type);
    edited_fixture.set_description(description);
    edited_fixture.set_channels(channels);
    edited_fixture.set_icon(icon);
    edited_fixture.set_start_channel(ui->sB_start_cahnnel->value());
}

void CreateFixtureDialog::on_buttonBox_accepted() {
    if(ui->lE_name->text().isEmpty() || ui->lE_description->text().isEmpty() || ui->lE_description->text().isEmpty()
            /*|| ui->lE_channel0->text().isEmpty()*/)
    {
        if(ui->lE_name->text().isEmpty()){ui->lE_name->setStyleSheet("border: 1px solid red");}
        else {ui->lE_name->setStyleSheet("border: 1px solid black");}

        if(ui->lE_description->text().isEmpty()) {ui->lE_description->setStyleSheet("border: 1px solid red");}
        else {ui->lE_description->setStyleSheet("border: 1px solid black");}

        /*if(ui->lE_channel0->text().isEmpty()) {ui->lE_channel0->setStyleSheet("border: 1px solid red");}
        else {ui->lE_channel0->setStyleSheet("border: 1px solid black");}*/
    } else if(is_edit_view && !is_allowed_start_channel()) {
        ui->sB_start_cahnnel->setStyleSheet("border: 1px solid red");
    } else
    {
     accept();
    }
}

void CreateFixtureDialog::on_buttonBox_rejected()
{
    reject();
}

void CreateFixtureDialog::on_pB_add_channel_clicked()
{
    QWidget * scrollarea_content = qobject_cast<QScrollArea *>(ui->vL_scrollarea->itemAt(0)->widget())->widget();
    QVBoxLayout *vL_channels = qobject_cast<QVBoxLayout *>(scrollarea_content->layout());
    vL_channels->addLayout(new QHBoxLayout) ;
    QHBoxLayout * new_channel = qobject_cast<QHBoxLayout *>(vL_channels->children().back());

    QString channel_name = "Channel ";
    channel_name.append(QString::number(++channel_count) + ":");
    new_channel->addWidget(new QLabel(channel_name));

    new_channel->addLayout(new QVBoxLayout);
    QVBoxLayout * function_layout = qobject_cast<QVBoxLayout *> (new_channel->children().back());

    function_layout->addLayout(new QHBoxLayout);
    QHBoxLayout * name_layout = qobject_cast<QHBoxLayout *>(function_layout->children().back());
    name_layout->addWidget(new QLabel("Name"));
    //qobject_cast<QLabel*> (name_layout->children().back())->setAlignment(Qt::AlignBottom);
    name_layout->addWidget(new QLabel("Von"));
    name_layout->addWidget(new QLabel("Bis"));


    function_layout->addLayout(new QVBoxLayout);
    qobject_cast<QVBoxLayout *> ( function_layout->children().back())->addLayout(new QHBoxLayout);
    qobject_cast<QHBoxLayout *> ( qobject_cast<QVBoxLayout *> ( function_layout->children().back())->children().back())->addWidget(new QLineEdit(this));
    qobject_cast<QHBoxLayout *> ( qobject_cast<QVBoxLayout *> ( function_layout->children().back())->children().back())->addWidget(new QLineEdit(this));
    qobject_cast<QHBoxLayout *>( qobject_cast<QVBoxLayout *>( function_layout->children().back())->children().back())->addWidget(new QLineEdit(this));

    qobject_cast<QLayout *> ( function_layout->children().back())->addWidget(new QPushButton("+"));

    QPushButton * btn_new_function = qobject_cast<QPushButton *>( qobject_cast<QVBoxLayout *>( function_layout->children().back())->itemAt(1)->widget());
    QObject * btn_new_function_parent = ( function_layout->children().back());


    // I needed to give a parameter from a clicked signal, so I used a Signalmapper.

    // create Signalmapper and map the parent of the button to it.
    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(btn_new_function, btn_new_function_parent);

    // connecting the clicked signal with the map of the signalmapper.
    connect(btn_new_function, SIGNAL(clicked()), signalMapper, SLOT(map()));

    // connecting the mapped object wit the slot that uses the parent of the pressed button.
    connect(signalMapper, SIGNAL(mapped(QObject *)), this, SLOT(on_pB_add_function_generated_clicked(QObject *)));
}



void CreateFixtureDialog::on_pB_add_function_generated_clicked(QObject *parent)
{
    qobject_cast<QVBoxLayout *>(parent)->insertLayout(qobject_cast<QVBoxLayout *>(parent)->children().count(), new QHBoxLayout);
    qobject_cast<QHBoxLayout *>(qobject_cast<QVBoxLayout *>(parent)->children().last())->addWidget(new QLineEdit(this));
    qobject_cast<QHBoxLayout *>(qobject_cast<QVBoxLayout *>(parent)->children().last())->addWidget(new QLineEdit(this));
    qobject_cast<QHBoxLayout *>(qobject_cast<QVBoxLayout *>(parent)->children().last())->addWidget(new QLineEdit(this));
}

bool CreateFixtureDialog::is_allowed_start_channel()
{
    bool is_allowed = true;
    int current_channel = ui->sB_start_cahnnel->value();
    if((current_channel + last_channel - 1) > max_channel){
        is_allowed = false;
    } else {
        for (int i = 0; i < blocked_channels.size(); i=i+2) {
            if((current_channel >= *std::next(blocked_channels.begin(), i)
                && current_channel <= *std::next(blocked_channels.begin(), i +1)))
            {
                if(!(original_start_channel >= *std::next(blocked_channels.begin(), i)
                     && original_start_channel <= *std::next(blocked_channels.begin(), i +1))){
                is_allowed = false;
                break;
                }
            }
        }
    }
    return is_allowed;
}
