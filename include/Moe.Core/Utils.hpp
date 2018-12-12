/**
 * @file
 * @author chu
 * @date 2017/4/30
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <type_traits>
#include <string>
#include <memory>

//////////////////////////////////////// <editor-fold desc="辅助宏">

/**
 * @brief 不可执行分支宏
 */
#define MOE_UNREACHABLE() \
    do { \
        assert(false); \
        ::abort(); \
    } while (false)

/**
 * @brief 标记未使用
 */
#define MOE_UNUSED(x) \
    static_cast<void>(x)

/**
 * @brief 单行断言
 *
 * 检查CHECK，若成立返回EXPR。
 * 用于解决C++11在constexpr中不允许多行代码的问题。
 */
#ifdef NDEBUG
#define MOE_ASSERT_EXPR(CHECK, EXPR) (EXPR)
#else
#define MOE_ASSERT_EXPR(CHECK, EXPR) ((CHECK) ? (EXPR) : ([]{assert(!#CHECK);}(), (EXPR)))
#endif

#define MOE_PP_ARG_OP_1(op, sep, arg, ...) \
    op(arg)
#define MOE_PP_ARG_OP_2(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_1(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_3(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_2(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_4(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_3(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_5(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_4(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_6(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_5(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_7(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_6(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_8(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_7(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_9(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_8(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_10(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_9(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_11(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_10(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_12(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_11(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_13(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_12(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_14(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_13(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_15(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_14(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_16(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_15(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_17(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_16(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_18(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_17(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_19(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_18(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_20(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_19(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_21(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_20(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_22(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_21(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_23(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_22(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_24(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_23(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_25(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_24(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_26(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_25(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_27(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_26(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_28(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_27(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_29(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_28(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_30(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_29(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_31(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_30(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_32(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_31(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_33(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_32(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_34(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_33(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_35(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_34(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_36(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_35(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_37(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_36(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_38(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_37(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_39(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_38(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_40(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_39(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_41(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_40(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_42(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_41(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_43(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_42(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_44(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_43(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_45(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_44(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_46(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_45(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_47(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_46(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_48(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_47(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_49(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_48(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_50(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_49(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_51(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_50(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_52(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_51(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_53(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_52(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_54(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_53(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_55(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_54(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_56(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_55(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_57(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_56(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_58(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_57(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_59(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_58(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_60(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_59(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_61(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_60(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_62(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_61(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_63(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_62(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_64(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_63(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_65(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_64(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_66(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_65(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_67(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_66(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_68(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_67(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_69(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_68(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_70(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_69(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_71(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_70(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_72(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_71(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_73(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_72(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_74(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_73(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_75(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_74(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_76(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_75(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_77(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_76(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_78(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_77(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_79(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_78(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_80(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_79(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_81(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_80(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_82(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_81(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_83(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_82(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_84(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_83(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_85(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_84(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_86(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_85(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_87(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_86(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_88(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_87(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_89(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_88(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_90(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_89(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_91(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_90(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_92(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_91(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_93(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_92(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_94(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_93(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_95(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_94(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_96(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_95(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_97(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_96(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_98(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_97(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))
#define MOE_PP_ARG_OP_99(op, sep, arg, ...) \
    op(arg) sep MOE_PP_EXPAND_(MOE_PP_ARG_OP_98(op, MOE_PP_EXPAND_(sep), ##__VA_ARGS__))

#define MOE_PP_ARG_SEQ_() \
    99,98,97,96,95,94,93,92,91,90, 89,88,87,86,85,84,83,82,81,80, \
    79,78,77,76,75,74,73,72,71,70, 69,68,67,66,65,64,63,62,61,60, \
    59,58,57,56,55,54,53,52,51,50, 49,48,47,46,45,44,43,42,41,40, \
    39,38,37,36,35,34,33,32,31,30, 29,28,27,26,25,24,23,22,21,20, \
    19,18,17,16,15,14,13,12,11,10,  9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define MOE_PP_ARG_N_(_1, _2, _3, _4, _5, _6, _7, _8, _9,_10, _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
    _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
    _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
    _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
    _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, _91,_92,_93,_94,_95,_96,_97,_98,_99, N, ...) N

#define MOE_PP_EXPAND_(...) __VA_ARGS__
#define MOE_PP_ARG_OP(N) MOE_PP_CONCAT(MOE_PP_ARG_OP_, N)
#define MOE_PP_COMMA ,

/**
 * @brief CONCAT宏
 */
#define MOE_PP_CONCAT(A, B) MOE_PP_CONCAT_(A, B)
#define MOE_PP_CONCAT_(A, B) A##B

/**
 * @brief ARG_COUNT宏
 *
 * 计算参数个数。
 */
#define MOE_PP_ARG_COUNT(...) MOE_PP_ARG_COUNT_(__VA_ARGS__, MOE_PP_ARG_SEQ_())
#define MOE_PP_ARG_COUNT_(...) MOE_PP_EXPAND_(MOE_PP_ARG_N_(__VA_ARGS__))

/**
 * @brief FOR_EACH宏
 *
 * MOE_PP_FOR_EACH(FOO, MOE_PP_COMMA, a, b) => FOO(a), FOO(b)
 */
#define MOE_PP_FOR_EACH(op, sep, arg, ...) MOE_PP_ARG_OP(arg, ##__VA_ARGS__)(op, sep, arg, ##__VA_ARGS__)

/**
 * @brief FOR_EACH_COMMA宏
 *
 * MOE_PP_FOR_EACH_COMMA(FOO, a, b) => FOO(a), FOO(b)
 */
#define MOE_PP_FOR_EACH_COMMA(op, arg, ...) MOE_PP_ARG_OP(arg, ##__VA_ARGS__)(op, MOE_PP_COMMA, arg, ##__VA_ARGS__)

//////////////////////////////////////// </editor-fold>

namespace moe
{
    /**
     * @brief 获取一个空对象的引用
     * @tparam T 类型
     * @return 引用
     */
    template <typename T>
    const T& EmptyRefOf()noexcept
    {
        static const T kEmptyInstance {};
        return kEmptyInstance;
    }

    /**
     * @brief 计算数组大小
     * @note 该方法用于计算一维数组的大小
     * @tparam T 数组类型
     * @return 结果，元素个数
     */
    template <typename T, size_t S>
    constexpr size_t CountOf(T(&)[S])noexcept
    {
        return S;
    }

    /**
     * @brief 不可拷贝基类
     */
    class NonCopyable
    {
    protected:
        constexpr NonCopyable()noexcept = default;
        ~NonCopyable()noexcept = default;

        NonCopyable(const NonCopyable&)noexcept = delete;
        NonCopyable& operator=(const NonCopyable&)noexcept = delete;
    };

    /**
     * @brief 按位强制转换
     * @tparam T 目标类型
     * @tparam P 输入类型
     * @param source 待转换对象
     * @return 转换后结果
     *
     * C++的aliasing rule允许指向不同类型的指针不会相互关联。这导致下述代码不会工作：
     *
     * float f = foo();
     * int fbits = *(int*)(&f);
     *
     * 编译器认为int*类型的fbits不会关联到f上（由于类型不同），所以优化后可能将f置于寄存器，从而使fbits中留下任意随机的结果。
     * 下述方法基于char、unsigned char类型的特殊规则，从而保证不会出现上述情况。
     */
    template <typename T, typename P>
    constexpr T BitCast(const P& source)noexcept
    {
        static_assert(sizeof(T) == sizeof(P), "Type size mismatched");
        static_assert(std::is_same<uint8_t, unsigned char>::value, "Bad compiler");

        return *reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(&source));
    }

    /**
     * @brief 单件
     * @tparam T 类型
     */
    template <typename T>
    class Singleton :
        public NonCopyable
    {
    public:
        static T& Instance()
        {
            static T s_stInstance;  // C++11保证对该static的初始化操作是原子的
            return s_stInstance;
        }
    };

    /**
     * @brief ScopeGuard
     * @tparam T 函数类型
     */
    template <typename T>
    class ScopeExit :
        public NonCopyable
    {
    public:
        explicit ScopeExit(const T& func)
            : m_stFunc(func) {}
        explicit ScopeExit(T&& func)
            : m_stFunc(std::move(func)) {}

        ScopeExit(ScopeExit&& rhs)
            : m_stFunc(std::move(rhs.m_stFunc)), m_bDismiss(rhs.m_bDismiss) {}

        ~ScopeExit()noexcept
        {
            if (!m_bDismiss)
            {
                if (m_stFunc)
                    m_stFunc();
            }
        }

    public:
        void Dismiss()noexcept { m_bDismiss = true; }

    private:
        T m_stFunc;
        bool m_bDismiss = false;
    };


    /**
     * @brief 类型索引
     *
     * TypeIndex without RTTI
     */
    class TypeIndex
    {
    private:
        template <typename T>
        static uintptr_t MakeTypeId()noexcept
        {
            static int s_iStub;
            return reinterpret_cast<uintptr_t>(&s_iStub);
        }

    public:
        template <typename T>
        TypeIndex()noexcept
        {
            using RemoveCVType = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
            m_uId = MakeTypeId<RemoveCVType>();
        }

        TypeIndex(const TypeIndex &rhs)noexcept
            : m_uId(rhs.m_uId)
        {}

    private:
        TypeIndex(uintptr_t id)noexcept
            : m_uId(id)
        {}

    public:
        bool operator==(const TypeIndex &t)const noexcept { return m_uId == t.m_uId; }
        bool operator!=(const TypeIndex &t)const noexcept { return m_uId != t.m_uId; }
        bool operator<(const TypeIndex &t)const noexcept { return std::less<uintptr_t>()(m_uId, t.m_uId); }
        bool operator<=(const TypeIndex &t)const noexcept { return !(t > *this); }
        bool operator>(const TypeIndex &t)const noexcept { return t < *this; }
        bool operator>=(const TypeIndex &t)const noexcept { return !(*this < t); }

    public:
        unsigned GetHashCode()const noexcept { return std::hash<uintptr_t>()(m_uId); }

    private:
        uintptr_t m_uId = 0;
    };

    struct FileCloser
    {
        void operator()(FILE* p)const noexcept
        {
            if (p)
                ::fclose(p);
        }
    };

    /**
     * @brief 文件句柄
     */
    using UniqueFileHandle = std::unique_ptr<FILE, FileCloser>;
    using SharedFileHandle = std::shared_ptr<FILE>;

    /**
     * @brief 读取整个文件（二进制的）
     * @exception IOException 打开文件失败抛出
     * @param[out] out 目标缓冲区
     * @param path 路径
     * @return 数据，即out
     */
    std::string& ReadWholeFile(std::string& out, const char* path);

    inline std::string ReadWholeFile(const char* path)
    {
        std::string ret;
        ReadWholeFile(ret, path);
        return ret;
    }

    inline std::string ReadWholeFile(const std::string& path)
    {
        std::string ret;
        ReadWholeFile(ret, path.c_str());
        return ret;
    }
}
