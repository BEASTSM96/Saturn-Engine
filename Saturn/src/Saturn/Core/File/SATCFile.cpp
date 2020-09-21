#include "sppch.h"
#include "SATCFile.h"

#include <ostream>
#include <fstream>

namespace Saturn {

    SATC::_SATCFIETYPEVOID()(std::string name, std::string path)
    {
        NewFile(name , path);
    }

    std::ofstream SATC::NewFile(std::string name, std::string path)
    {
        std::ofstream o = std::ofstream(name + path);

        return o;
    }

}