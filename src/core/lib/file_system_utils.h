//
// Created by Jan on 01.06.2019.
//

#ifndef RASPITOLIGHT_SRC_CORE_LIB_FILE_SYSTEM_UTILS_H_
#define RASPITOLIGHT_SRC_CORE_LIB_FILE_SYSTEM_UTILS_H_

#include <string>
#include <ghc/filesystem.hpp>

class FileSystemUtils {
 public:
  /*
   * "M:\workspace\19ss-raspitolight1\assets\Crazy-Frog-Axel-F.wav" => "Crazy-Frog-Axel-F.wav"
   * "M:\workspace\19ss-raspitolight1\assets\Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav" => "Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav"
   * "M:/workspace/19ss-raspitolight1/assets/Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav" => "Avicii-feat.-Aloe-Blacc-Wake-Me-Up.wav"
   */
  static std::string get_file_name(const std::string &t){
    return ghc::filesystem::path(t).filename();
  }

  static std::string get_file_name(const char* c){
    return ghc::filesystem::path(c).filename();
  }
};


#endif //RASPITOLIGHT_SRC_CORE_LIB_FILE_SYSTEM_UTILS_H_
