//
// Created by Johannes on 23.01.2020.
//

#ifndef SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_
#define SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_

#include <QtWidgets/QDialog>

namespace Ui {
class ChangeFixtures;
}

class ChangeFixtures : public QDialog {
 Q_OBJECT

 public:
  explicit ChangeFixtures(QWidget *parent = nullptr);
  ~ChangeFixtures() override;


 private:
  Ui::ChangeFixtures *ui;

};

#endif //SIGLIGHT_SRC_UI_CHANGE_FIXTURES_H_
