#include "mainwindow.h"
#include <QApplication>
#include <stacktrace_registration.h>
#include <version.h>
#include "mp_3_to_wav_converter.h"

int main(int argc, char *argv[]) {
  Logger::info("RaspiToLight, commit hash {} on branch \"{}\"", getCommitHash(), getBranchName());
  const bool lame_avaible = Mp3ToWavConverter::is_avaible();
  if(lame_avaible){
    Logger::info("Lame is avaible");
  }
  else{
    Logger::warning("Lame is not avaible, Support for MP3 files will be disabled!");
  }
  Logger::set_level(Logger::LevelEnum::off);

  register_signal_handlers();
  Logger::info(signal_handler_registration_report());

  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  return QApplication::exec();
}
