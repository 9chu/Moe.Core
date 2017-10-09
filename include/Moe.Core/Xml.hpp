/**
 * @file
 * @date 2017/10/9
 */
#pragma once
#include "ArrayView.hpp"

namespace moe
{
    class XmlSaxHandler
    {
    public:
        virtual void OnXmlDeclarationBegin(const std::string& name) = 0;
        virtual void OnXmlDeclarationEnd() = 0;
        virtual void OnXmlElementBegin(const std::string& name) = 0;
        virtual void OnXmlElementEnd(const std::string& name) = 0;
        virtual void OnXmlAttribute(const std::string& key, const std::string& val) = 0;
        virtual void OnXmlComment(const std::string& comment) = 0;
        virtual void OnXmlContent(const std::string& content) = 0;
    };

    class Xml
    {
    public:
    };
}
