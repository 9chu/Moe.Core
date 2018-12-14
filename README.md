# Moe.Core

MOE游戏开发计划：跨平台基础库。

`Moe.Core`旨在实现一个基于C++11的跨平台、无依赖及开箱即用的C++基础库（相比而言效率不是首要目标）。这一基础库实现了许多常见的功能模块，亦或整合自其他开源项目。

**该基础库依旧在开发过程中。**

## 编译

### 依赖

- CMake >= 3.1
- g++ >= 4.8、VS2017及更高，或者其他支持C++11的编译器（对，我是说clang）

## 功能模块

- Any/Optional: Any/Optional的C++11支持
- ArrayView: 使用<T\*, length>二元组描述的任意数组
- Cipher: 加密方法
- CmdParser: 命令行解析器
- ConsistentHash: 基于KETAMA的一致性哈希算法
- Convert: 字符串<->整数/浮点类型转换库
- Encoding: 编码转换库（目前仅实现Unicode）
- Exception: 基本异常定义
- Hasher: 哈希算法
- Http/Url: HTTP/URL解析器
- Idna/Unicode: IDNA/Unicode支持
- TextReader/Parser/Json/Xml: 解析器相关与实现
- Logging: 全局日志接口
- Math: 数学库
- Mdr: 二进制数据交换协议**（TODO）**
- ObjectPool: 对象池
- Pal: 平台无关抽象
- StringUtils/PathUtils: 字符串与路径处理辅助库
- RefPtr: 侵入式引用计数指针
- StaticContainer: 预分配大小的容器
- Stream: 流式接口定义
- Threading: 多线程相关
- Time: 时间相关辅助库
- Utils: 杂项

