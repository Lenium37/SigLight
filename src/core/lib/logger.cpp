//
// Created by Jan on 26.05.2019.
//
#include <logger.h>
void Logger::set_level(LevelEnum level) {
  if (level == LevelEnum::trace) {
    spdlog::set_level(spdlog::level::trace);
  } else if (level == LevelEnum::debug) {
    spdlog::set_level(spdlog::level::debug);
  } else if (level == LevelEnum::info) {
    spdlog::set_level(spdlog::level::info);
  } else if (level == LevelEnum::warn) {
    spdlog::set_level(spdlog::level::warn);
  } else if (level == LevelEnum::err) {
    spdlog::set_level(spdlog::level::err);
  } else if (level == LevelEnum::critical) {
    spdlog::set_level(spdlog::level::critical);
  } else if (level == LevelEnum::off) {
    spdlog::set_level(spdlog::level::off);
  }
}