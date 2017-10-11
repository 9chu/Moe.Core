/**
 * @file
 * @date 2017/10/9
 */
#pragma once
#include <vector>
#include <unordered_map>
#include "ArrayView.hpp"
#include "RefPtr.hpp"

namespace moe
{
    class XmlNode :
        public RefBase<XmlNode>
    {
    public:
        XmlNode() = default;
        virtual ~XmlNode() = default;

    public:
        /**
         * @brief 检查是否是元素节点
         */
        virtual bool IsElement()const noexcept = 0;

        /**
         * @brief 检查是否是文本节点
         */
        virtual bool IsText()const noexcept = 0;

        /**
         * @brief 写到字符串
         * @param str 字符串
         * @return 即str的引用
         */
        virtual std::string& Stringify(std::string& str)const = 0;
    };

    using XmlNodePtr = RefPtr<XmlNode>;

    class XmlElement;
    using XmlElementList = std::vector<RefPtr<XmlElement>>;

    class XmlElement :
        public XmlNode
    {
    public:
        XmlElement(const std::string& name);
        XmlElement(std::string&& name);
        ~XmlElement();

        /**
         * @brief 索引器
         * @exception OutOfRangeException 索引越界时产生异常
         * @param index 下标
         * @return 节点指针
         */
        XmlNodePtr operator[](size_t index);
        const XmlNodePtr operator[](size_t index)const;

    public:
        /**
         * @brief 获取名称
         */
        const std::string& GetName()const noexcept { return m_stName; }

        /**
         * @brief 获取节点个数
         */
        size_t GetNodeCount()const noexcept { return m_stNodes.size(); }

        /**
         * @brief 追加一个节点
         * @param node 节点
         */
        void AppendNode(XmlNodePtr node);

        /**
         * @brief 移除一个节点
         * @param node 节点
         * @return 节点是否存在
         */
        bool RemoveNode(XmlNodePtr node);

        /**
         * @brief 插入一个节点
         * @param index 插入位置
         * @param node 节点
         */
        void InsertNode(size_t index, XmlNodePtr node);

        /**
         * @brief 使用名称查找所有元素
         * @param name 节点名称
         * @return 元素列表
         */
        const XmlElementList& FindElementByName(const char* name)const;
        const XmlElementList& FindElementByName(const std::string& name)const
        {
            return FindElementByName(name.c_str());
        }

    public:  // implement for XmlNode
        bool IsElement()const noexcept override;
        bool IsText()const noexcept override;
        std::string& Stringify(std::string& str)const override;

    private:
        std::string m_stName;  // 节点名字是不可变的
        std::vector<XmlNodePtr> m_stNodes;

        std::unordered_map<std::string, XmlElementList> m_stCache;  // 查找缓存
    };

    class XmlText :
        public XmlNode
    {
    public:
        XmlText();
        XmlText(const std::string& content);
        XmlText(std::string&& content);
        ~XmlText();

    public:
        const std::string& GetContent()const noexcept { return m_stContent; }
        void SetContent(const std::string& content) { m_stContent = content; }
        void SetContent(std::string&& content)noexcept { m_stContent = std::move(content); }

    public:  // implement for XmlNode
        bool IsElement()const noexcept override;
        bool IsText()const noexcept override;
        std::string& Stringify(std::string& str)const override;

    private:
        std::string m_stContent;
    };

    /**
     * @brief Xml解析回调
     *
     * 举例：
     *   <a>abc<b>def</b>123</a>
     *
     * 执行过程为：
     *   XmlElementBegin(a)
     *   XmlContent(abc)
     *   XmlElementBegin(b)
     *   XmlContent(def)
     *   XmlElementEnd(b)
     *   XmlContent(123)
     *   XmlElementEnd(a)
     *
     * 被调用方需要自行检查标签是否匹配。
     */
    class XmlSaxHandler
    {
    public:
        virtual void OnXmlElementBegin(const std::string& name) = 0;
        virtual void OnXmlElementEnd(const std::string& name) = 0;
        virtual void OnXmlAttribute(const std::string& key, const std::string& val) = 0;
        virtual void OnXmlContent(const std::string& content) = 0;
    };

    class Xml
    {
    public:
    };
}
