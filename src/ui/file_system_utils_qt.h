//
// Created by Jan on 01.06.2019.
//

#ifndef RASPITOLIGHT_SRC_UI_FILE_SYSTEM_UTILS_QT_H_
#define RASPITOLIGHT_SRC_UI_FILE_SYSTEM_UTILS_QT_H_

#include <string>
#include <QtCore/QUrl>
#include <QtCore/QFileInfo>
#include <file_system_utils.h>

/**
 * FileSystemUtils specific to Qt
 */
class FileSystemUtilsQt {
 public:
  static std::string get_file_name(const QUrl &url) {
    const std::string &string = QFileInfo(url.toLocalFile()).absoluteFilePath().toStdString();
    return FileSystemUtils::get_file_name(string);
  }
};

#endif //RASPITOLIGHT_SRC_UI_FILE_SYSTEM_UTILS_QT_H_
