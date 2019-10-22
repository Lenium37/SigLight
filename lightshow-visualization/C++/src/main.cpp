//
// Created by Johannes on 22.10.2019.
//

#include <iostream>
#include <QtWidgets/qapplication.h>
#include <QtCore/qfile.h>
#include <QtCore/QIODevice>
#include <QtCore/QXmlStreamReader>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include "lightshow_window.h"
#include <tinyxml2.h>
#include <unistd.h>
#include <QtMultimedia/qmediaplayer.h>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  QFile stylesheet(":styles.qss");
  stylesheet.open(QFile::ReadOnly);
  QString StyleSheet = QLatin1String(stylesheet.readAll());
  a.setStyleSheet(StyleSheet);
  stylesheet.close();


  auto* lightshow_window = new LightshowWindow();
  lightshow_window->init();



  return QApplication::exec();
}