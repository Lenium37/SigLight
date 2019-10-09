#include "change_lightshow_dialog.h"
#include "ui_change_lightshow_dialog.h"

Change_lightshow_dialog::Change_lightshow_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Change_lightshow_dialog)
{
    ui->setupUi(this);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->comboBox->addItems(methods);
}

Change_lightshow_dialog::~Change_lightshow_dialog()
{
    delete ui;
}

void Change_lightshow_dialog::add_new_row()
{
    get_current_data();

    int rows = ui->tableWidget->rowCount();

    ui->tableWidget->insertRow(rows);

    start_time_widget_item = new QTableWidgetItem(temp_start_time, 0);
    end_time_widget_item = new QTableWidgetItem(temp_end_time, 0);
    method_widget_item = new QTableWidgetItem(temp_method, 0);
    ui->tableWidget->setItem(rows, 0, start_time_widget_item);
    ui->tableWidget->setItem(rows, 1, end_time_widget_item);
    ui->tableWidget->setItem(rows, 2, method_widget_item);

    ui->tableWidget->sortItems(Qt::AscendingOrder);
}

void Change_lightshow_dialog::on_buttonBox_rejected()
{
    this->close();
}

void Change_lightshow_dialog::on_buttonBox_accepted()
{

    this->close();
}

void Change_lightshow_dialog::get_current_data()
{
    temp_start_time_milliseconds = ui->start_time_edit->time().msec();
    temp_start_time = ui->start_time_edit->time().toString();

    change_list.append(temp_start_time_milliseconds);

    temp_end_time_milliseconds = ui->end_time_edit->time().msec();
    temp_end_time = ui->end_time_edit->time().toString();

    change_list.append(temp_end_time_milliseconds);

    temp_method_index = ui->comboBox->currentIndex();
    temp_method = ui->comboBox->currentText();
    change_list.append(temp_method_index);
}


void Change_lightshow_dialog::on_add_button_clicked()
{
    add_new_row();
}

void Change_lightshow_dialog::on_delete_button_clicked()
{
     int i = ui->tableWidget->selectionModel()->selectedRows()[0].row();
     ui->tableWidget->removeRow(i);
}
