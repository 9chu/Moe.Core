/**
 * @file
 * @date 2017/10/9
 */
#include <Moe.Core/Xml.hpp>
#include <Moe.Core/Parser.hpp>

using namespace std;
using namespace moe;

//////////////////////////////////////////////////////////////////////////////// Xml

namespace
{
    class XmlParser :
        public Parser
    {
    public:
        XmlParser(XmlSaxHandler* handler)
            : m_pHandler(handler) {}

    public:
        void Run(TextReader& reader)override
        {
            Parser::Run(reader);

            // 初始化内部状态
            m_stStringBuffer.clear();

            // 开始解析

        }

    private:


    private:
        XmlSaxHandler* m_pHandler = nullptr;
        std::string m_stStringBuffer;
    };
}
