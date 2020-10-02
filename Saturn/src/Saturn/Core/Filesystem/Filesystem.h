#pragma once

#include <filesystem>
#include "Saturn/Core.h"
#include "Saturn/Log.h"

namespace Saturn::FileSystem {

    using CopyOptions = 
        std::filesystem::copy_options
    ;

    using Path =
        std::filesystem::path
    ;

    using FileErrorcode =
        std::error_code
    ;

    /*FROM STD::FILESYSTEM::COPY_FILE*/


    bool Copy(const Path& From, const Path& To, const CopyOptions Options, FileErrorcode& FileErrorCode) noexcept{
        return std::filesystem::copy_file(From, To, Options, FileErrorCode);
    }

    bool Copy(const Path& From, const Path& To, FileErrorcode& FileErrorCode) noexcept {
        return std::filesystem::copy_file(From, To, CopyOptions::none, FileErrorCode);
    }

    bool Copy(const Path& From, const Path& To) {
        return std::filesystem::copy_file(From, To, CopyOptions::none);
    }

    bool Copy(const Path& From, const Path& To, const CopyOptions Options) {
        return copy_file(From, To, Options);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    uintmax_t FileSize(const Path& Dir /*Dir = Path*/) {
      return std::filesystem::file_size(Dir);
    }

    /*
    *   Example: DisplayAllFilesInPath(C:\\MyFolder)
    *   Will display the files in that dir aka C:\\MyFolder
    */
   std::vector<Path> GetAllFilePaths(std::string path) {

       std::vector<Path> paths;
       Path displayedFiles;

       for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
       {
           paths.push_back(entry.path());
           SAT_CORE_INFO("Files {0}", displayedFiles);
       }

       return paths;
    
   }

   /*
    *   Example: DisplayAllFilesInPath(C:\\MyFolder)
    *   Will display the files in that dir aka C:\\MyFolder
    */
   std::vector<Path> GetAllFilesInPath(std::string path) {

       std::vector<Path> paths;
  
       for (const auto& entry : std::filesystem::recursive_directory_iterator(path))
       {
           paths.push_back(entry.path());
       }

       return paths;

   }

}