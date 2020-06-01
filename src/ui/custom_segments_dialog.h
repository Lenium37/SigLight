#ifndef CUSTOMSEGMENTSDIALOG_H
#define CUSTOMSEGMENTSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <list>
#include <QLayout>
#include <QSignalMapper>
//#include <QScrollArea>
//#include "fixturemanager/fixture.h"

namespace Ui {
class CustomSegmentsDialog;
}

class CustomSegmentsDialog : public QDialog {
 Q_OBJECT

 public:
  explicit CustomSegmentsDialog(std::vector<float> existing_segment_timestamps, QWidget *parent = nullptr);
  ~CustomSegmentsDialog() override;
  std::vector<float> get_custom_segments();

 //private slots:
  //void on_buttonBox_accepted();

  //void on_buttonBox_rejected();

  //void on_pB_add_channel_clicked();

  //void on_pB_add_function_generated_clicked(QObject *);

private:
  Ui::CustomSegmentsDialog *ui;
  std::vector<float> segment_timestamps;

 private slots:
  void on_add_custom_segments_finished_clicked();
  void on_pB_add_segment_clicked();
  void on_pB_delete_segment_clicked();

};

#endif // CUSTOMSEGMENTSDIALOG_H
