//
// Created by Johannes on 23.01.2020.
//

#include "change_fixtures.h"
#include "ui_change_fixtures.h"

ChangeFixtures::ChangeFixtures(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ChangeFixtures) {
    ui->setupUi(this);

}

ChangeFixtures::~ChangeFixtures() {
  delete ui;
}
