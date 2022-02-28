#ifndef SRCFILE_H
#define SRCFILE_H
#include <vector>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

struct SrcFile {
  fs::path name;
  std::string contents;
  std::vector<const char*> line_offsets;

  SrcFile(fs::path name);
};


#endif
