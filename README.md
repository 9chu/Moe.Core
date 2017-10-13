# Moe.Core

Moe游戏框架底层模块。

## 编译

### 依赖

- CMake 2.8
- g++4.8（或其他支持C++11的编译器）

### 目录树

- 3rd：第三方代码
- doc：（预留）文档目录
- build：（预留）编译目录
- include：头文件目录
    - Moe.Core
- src：源文件目录
- test：测试用例目录

### 功能模块

- [x] Any/Optional: Any/Optional的C++11支持
- [x] ArrayView: 使用<T*, length>二元组描述的任意数组
- [x] Buffer/FixedBufferPool: 一些定长/变长缓冲区
- [x] Cipher: 加密算法库
    - [x] RC4
- [x] CircularQueue: 循环队列
- [x] Convert: 转换库
    - [x] ParseInt
    - [x] ParseUInt
    - [x] ParseFloat
    - [x] ParseDouble
    - [x] \(Byte/Short/Int/Int64/Double\)ToString
- [x] Encoding: 编码转换库
    - [x] UTF8
    - [x] UTF16
- [x] Exception: 基本异常定义
- [x] Hasher: 哈希算法
    - [x] MPQHash
    - [x] MurmurHash
- [x] TextReader/Parser/Json/Xml: 解析器相关与实现
- [x] Logging: 全局日志接口
- [x] Math: 数学库
    - [x] 基本数学函数与常量
    - [x] Vector2
    - [x] Vector3
    - [x] Quaternion
    - [ ] Matrix
- [ ] Mbp: 二进制数据交换协议
- [x] StringUtils/PathUtils: 字符串与路径处理辅助库
- [x] RefPtr: 侵入式引用计数指针
- [x] Stream: 流式接口定义
- [x] Threading: 多线程相关
- [x] Time: 时间相关辅助库
- [x] Utils: 杂项
