/**
 * @file
 * @date 2017/10/12
 */
#include <Moe.Core/Utils.hpp>
#include <Moe.Core/Exception.hpp>

#include <fstream>

using namespace std;
using namespace moe;

std::string& moe::ReadWholeFileInPlace(std::string& out, const char* path)
{
    out.clear();

    ifstream t(path, ios::binary);
    if (!t.good())
        MOE_THROW(IOException, "Open file \"{0}\" error", path);

    t.seekg(0, std::ios::end);
    if (!t.good())
        MOE_THROW(IOException, "Seek to end on file \"{0}\" error", path);

    auto size = t.tellg();
    if (size < 0)
        MOE_THROW(IOException, "Tellg on file \"{0}\" error", path);
    out.reserve(size);

    t.seekg(0, std::ios::beg);
    if (!t.good())
        MOE_THROW(IOException, "Seek to begin on file \"{0}\" error", path);

    out.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    if (out.length() != (size_t)size)
        MOE_THROW(IOException, "Read file \"{0}\" error", path);
    return out;
}
