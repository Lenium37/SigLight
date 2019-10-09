#ifndef CHANGE_LIGHTSHOW_DIALOG_H
#define CHANGE_LIGHTSHOW_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QTableWidgetItem>
#include <QLinkedList>

namespace Ui {
class Change_lightshow_dialog;
}

class Change_lightshow_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Change_lightshow_dialog(QWidget *parent = nullptr);
    ~Change_lightshow_dialog();

    void add_new_row();

private slots:
    void on_buttonBox_rejected();

    void on_buttonBox_accepted();

    void on_add_button_clicked();

    void on_delete_button_clicked();

private:
    Ui::Change_lightshow_dialog *ui;
    QComboBox *method_combobox;
    int temp_start_time_milliseconds;
    QString temp_start_time;
    int temp_end_time_milliseconds;
    QString temp_end_time;
    int temp_method_index;
    QString temp_method;

    QLinkedList<int> change_list;
    QStringList methods = {"Method1", "Method2", "Method3", "Method4", "Method5", "Method6"};

    QTableWidgetItem *start_time_widget_item;
    QTableWidgetItem *end_time_widget_item;
    QTableWidgetItem *method_widget_item;

    void get_current_data();

};

#endif // CHANGE_LIGHTSHOW_DIALOG_H
