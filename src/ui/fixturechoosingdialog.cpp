#include "fixturechoosingdialog.h"
#include "ui_fixturechoosingdialog.h"
#include <QtCore>

FixtureChoosingDialog::FixtureChoosingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FixtureChoosingDialog) {
    ui->setupUi(this);
    connect(ui->add_fixture_dialog, SIGNAL(finished(int)), this, SLOT(on_add_fixture_dialog_clicked()));
    ui->pB_delete_fixture->setVisible(false);
    this->setWindowTitle("Choose Fixture");
    FixtureChoosingDialog::is_delete = false;
}

FixtureChoosingDialog::FixtureChoosingDialog(QWidget *parent, list<Fixture> &fixtures, std::vector<std::string> color_palettes) :
    QDialog(parent),
    ui(new Ui::FixtureChoosingDialog) {
    ui->setupUi(this);
    types << "Ambient" << "Bass" << "Mid" << "High" << "color_change_beats" << "color_change_beats_action" << "color_change_onsets" << "onset_blink";
    for(std::string _colors: color_palettes)
      colors << QString::fromStdString(_colors);
    auto list_size = static_cast<double>(fixtures.size());
    for (int i = 0; i < list_size; i++) {
      ui->fixture_selection->addItem(QString::fromStdString((std::next(fixtures.begin(), i))->get_name()));
      types_of_fixtures << QString::fromStdString(std::next(fixtures.begin(), i)->get_type());
      end_channels.push_back(std::next(fixtures.begin(), i)->get_channel_count());
    }

    ui->fixture_selection->setCurrentRow(0);
    ui->cB_type->addItems(types);
    ui->cB_colors->addItems(colors);
    ui->sB_start_channel->setRange(1,max_channel);
    ui->pB_delete_fixture->setVisible(false);
    this->setWindowTitle("Choose Fixture");
    FixtureChoosingDialog::is_delete = false;
}

FixtureChoosingDialog::~FixtureChoosingDialog() {
    delete ui;
}

void FixtureChoosingDialog::set_up_dialog_options(std::list<int> blocked_channels)
{
    this->blocked_channels = blocked_channels;
    set_first_allowed_channel(0, true);
}

void FixtureChoosingDialog::get_fixture_options(int &fixture_id,int &start_channel, QString &type, std::string &colors)
{
    fixture_id = ui->fixture_selection->currentRow();
    start_channel = ui->sB_start_channel->value();
    type = ui->cB_type->currentText();
    colors = ui->cB_colors->currentText().toStdString();
}

void FixtureChoosingDialog::setup_for_edit()
{
    ui->l_type->setVisible(false);
    ui->l_start_channel->setVisible(false);
    ui->cB_type->setVisible(false);
    ui->sB_start_channel->setVisible(false);
    ui->pB_delete_fixture->setVisible(true);
    ui->add_fixture_dialog->setText("Edit");
}

int FixtureChoosingDialog::edit_preset_choosen() {
    return ui->fixture_selection->currentRow();
}

bool FixtureChoosingDialog::delete_fixture_preset()
{
    return is_delete;
}

void FixtureChoosingDialog::on_add_fixture_dialog_clicked() {
    if(is_allowed_start_channel(ui->sB_start_channel->value())){
        accept();
    } else {
        ui->sB_start_channel->setStyleSheet("border: 1px solid red");
    }
}

bool FixtureChoosingDialog::is_allowed_start_channel(int start_channel)
{
    bool is_allowed = true;
    if(((start_channel + *std::next(end_channels.begin(), ui->fixture_selection->currentRow())) - 1) > max_channel) {
        is_allowed = false;
    } else {
        for (int i = 0; i < blocked_channels.size(); i=i+2) {
            if(start_channel >= *std::next(blocked_channels.begin(), i)
                && start_channel <= *std::next(blocked_channels.begin(), i +1)) {
                is_allowed = false;
                break;
            }
        }
    }
    return is_allowed;
}

void FixtureChoosingDialog::set_first_allowed_channel(int current_Row, bool first_start)
{
    int end_value = *std::next(end_channels.begin(), current_Row);
    bool channels_left = false;
    bool channel_range_blocked = true;
    int calculated_end_value;
    for(int i = 1; i < max_channel; i++) {
        if(is_allowed_start_channel(i)) {
            calculated_end_value = i + end_value;
            for(int j = i; j < calculated_end_value; j++) {
                if((channel_range_blocked = !is_allowed_start_channel(j))) {
                    break;
                } else if (j > ui->sB_start_channel->maximum()) {
                    channel_range_blocked = true;
                    break;
                }
            }
            if(!channel_range_blocked) {
                ui->sB_start_channel->setValue(i);
                channels_left = true;
                break;
            }
        }
    }
    if(!channels_left && !first_start) {
        QMessageBox::question(this, "No free Channels left", "There are no more free Channels left for this Fixture.", QMessageBox::Ok);
    }
    ui->add_fixture_dialog->setEnabled(channels_left);
}

void FixtureChoosingDialog::on_fixture_selection_currentRowChanged(int currentRow)
{
    ui->cB_type->setCurrentIndex(ui->cB_type->findText(types_of_fixtures.at(currentRow)));
    set_first_allowed_channel(currentRow);
}

void FixtureChoosingDialog::on_pB_delete_fixture_clicked()
{
    FixtureChoosingDialog::is_delete = true;
    accept();
}
