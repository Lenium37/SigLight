#include <iostream>
#include "custom_segments_dialog.h"
#include "ui_custom_segments_dialog.h"

CustomSegmentsDialog::CustomSegmentsDialog(std::vector<float> existing_segment_timestamps, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CustomSegmentsDialog) {
  ui->setupUi(this);

  this->ui->lE_segment_timestamp->setValidator( new QDoubleValidator(0, 10000, 2, this) );

  for(int i = 0; i < existing_segment_timestamps.size(); i++) {
    this->segment_timestamps.push_back(existing_segment_timestamps[i]);
    this->ui->segment_list->addItem(QString::fromStdString(std::to_string(existing_segment_timestamps[i])));
  }

}

CustomSegmentsDialog::~CustomSegmentsDialog() {
    delete ui;
}


static inline void ReplaceAllSubstrings2(std::string &str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
  }
}

void CustomSegmentsDialog::on_add_custom_segments_finished_clicked() {
  accept();
}

void CustomSegmentsDialog::on_pB_add_segment_clicked() {
//  if((this->ui->lE_segment_timestamp->text().toStdString().find(".") != std::string::npos && !this->ui->lE_segment_timestamp->text().toStdString().find(",") != std::string::npos)
//    || (!this->ui->lE_segment_timestamp->text().toStdString().find(".") != std::string::npos && this->ui->lE_segment_timestamp->text().toStdString().find(",") != std::string::npos)
//    || (!this->ui->lE_segment_timestamp->text().toStdString().find(".") != std::string::npos && !this->ui->lE_segment_timestamp->text().toStdString().find(",") != std::string::npos)) {


  if(!(this->ui->lE_segment_timestamp->text().toStdString().find(".") != std::string::npos && this->ui->lE_segment_timestamp->text().toStdString().find(",") != std::string::npos)) {
    if(std::find(this->segment_timestamps.begin(), this->segment_timestamps.end(), std::stof(this->ui->lE_segment_timestamp->text().toStdString())) == this->segment_timestamps.end()) {
      std::string timestamp = this->ui->lE_segment_timestamp->text().toStdString();

      ReplaceAllSubstrings2(timestamp, ",", ".");

      std::cout << "adding segment timestamp: " << timestamp << std::endl;
//    ui->fixture_selection->addItem(QString::fromStdString((std::next(fixtures.begin(), i))->get_name()));
      this->segment_timestamps.push_back(std::stof(timestamp));
      this->ui->segment_list->addItem(QString::fromStdString(timestamp));

//      for(int i = 0; i < this->segment_timestamps.size(); i++) {
//        std::cout << this->segment_timestamps[i] << std::endl;
//      }
      this->ui->lE_segment_timestamp->setText("");
    }
  }
}

void CustomSegmentsDialog::on_pB_delete_segment_clicked() {
  if(this->ui->segment_list->selectedItems().size() != 0) {
    std::cout << "deleting segment timestamp: " << this->ui->segment_list->currentItem()->text().toStdString()
              << std::endl;
    this->segment_timestamps.erase(std::remove(this->segment_timestamps.begin(),
                                               this->segment_timestamps.end(),
                                               std::stof(this->ui->segment_list->currentItem()->text().toStdString())),
                                   this->segment_timestamps.end());
    qDeleteAll(ui->segment_list->selectedItems());

//    for(int i = 0; i < this->segment_timestamps.size(); i++) {
//      std::cout << this->segment_timestamps[i] << std::endl;
//    }
  }

}

std::vector<float> CustomSegmentsDialog::get_custom_segments() {
  sort(this->segment_timestamps.begin(), this->segment_timestamps.end());
  return this->segment_timestamps;
}
