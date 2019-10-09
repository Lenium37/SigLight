//
// Created by Jan Honsbrok on 03.09.19.
//
#include <version.h>

#ifndef GIT_BRANCH_NAME
#define GIT_BRANCH_NAME "0000000" // 0000000 means uninitialized
#endif

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "0000000" // 0000000 means uninitialized
#endif

std::string getBranchName() {
  return std::string(GIT_BRANCH_NAME);
}
std::string getCommitHash() {
  return std::string(GIT_COMMIT_HASH);
}

