/**
 * @file
 * @date 2017/7/8
 */
#include <gtest/gtest.h>

#include <Moe.Core/Optional.hpp>

using namespace std;
using namespace moe;

enum State
{
    sDefaultConstructed,
    sValueCopyConstructed,
    sValueMoveConstructed,
    sCopyConstructed,
    sMoveConstructed,
    sMoveAssigned,
    sCopyAssigned,
    sValueCopyAssigned,
    sValueMoveAssigned,
    sMovedFrom,
    sValueConstructed
};

struct OracleVal
{
    State s;
    int i;
    OracleVal(int i = 0) : s(sValueConstructed), i(i) {}
};

struct Oracle
{
    State s;
    OracleVal val;

    Oracle() : s(sDefaultConstructed) {}
    Oracle(const OracleVal& v) : s(sValueCopyConstructed), val(v) {}
    Oracle(OracleVal&& v) : s(sValueMoveConstructed), val(std::move(v)) {v.s = sMovedFrom;}
    Oracle(const Oracle& o) : s(sCopyConstructed), val(o.val) {}
    Oracle(Oracle&& o) : s(sMoveConstructed), val(std::move(o.val)) {o.s = sMovedFrom;}

    Oracle& operator=(const OracleVal& v) { s = sValueCopyConstructed; val = v; return *this; }
    Oracle& operator=(OracleVal&& v) { s = sValueMoveConstructed; val = std::move(v); v.s = sMovedFrom; return *this; }
    Oracle& operator=(const Oracle& o) { s = sCopyConstructed; val = o.val; return *this; }
    Oracle& operator=(Oracle&& o) { s = sMoveConstructed; val = std::move(o.val); o.s = sMovedFrom; return *this; }
};

struct Guard
{
    std::string val;
    Guard() : val{} {}
    explicit Guard(std::string s, int = 0) : val(s) {}
    Guard(const Guard&) = delete;
    Guard(Guard&&) = delete;
    void operator=(const Guard&) = delete;
    void operator=(Guard&&) = delete;
};

struct ExplicitStr
{
    std::string s;
    explicit ExplicitStr(const char* chp) : s(chp) {};
};

bool operator==( Oracle const& a, Oracle const& b ) { return a.val.i == b.val.i; }
bool operator!=( Oracle const& a, Oracle const& b ) { return a.val.i != b.val.i; }

TEST(Optional, DisengagedCtor)
{
    Optional<int> o1;
    EXPECT_TRUE(!o1);

    Optional<int> o2 = o1;
    EXPECT_TRUE(!o2);

    EXPECT_TRUE(o1 == Optional<int>{});
    EXPECT_TRUE(!o1);
    EXPECT_TRUE(bool(o1) == false);

    EXPECT_TRUE(o2 == Optional<int>{});
    EXPECT_TRUE(!o2);
    EXPECT_TRUE(bool(o2) == false);

    EXPECT_TRUE(o1 == o2);
    EXPECT_TRUE(o2 == o1);
};

TEST(Optional, ValueCtor)
{
    OracleVal v;
    Optional<Oracle> oo1(v);
    EXPECT_TRUE(oo1 != Optional<Oracle>{});
    EXPECT_TRUE(oo1 == Optional<Oracle>{v});
    EXPECT_TRUE(!!oo1);
    EXPECT_TRUE(bool(oo1));
    // NA: EXPECT_TRUE(oo1->s == sValueCopyConstructed);
    EXPECT_TRUE(oo1->s == sMoveConstructed);
    EXPECT_TRUE(v.s == sValueConstructed);

    Optional<Oracle> oo2(std::move(v));
    EXPECT_TRUE(oo2 != Optional<Oracle>{});
    EXPECT_TRUE(oo2 == oo1);
    EXPECT_TRUE(!!oo2);
    EXPECT_TRUE(bool(oo2));
    // NA: EXPECT_TRUE(oo2->s == sValueMoveConstructed);
    EXPECT_TRUE(oo2->s == sMoveConstructed);
    EXPECT_TRUE(v.s == sMovedFrom);

    {
        OracleVal v;
        Optional<Oracle> oo1{InPlaceInit, v};
        EXPECT_TRUE(oo1 != Optional<Oracle>{});
        EXPECT_TRUE(oo1 == Optional<Oracle>{v});
        EXPECT_TRUE(!!oo1);
        EXPECT_TRUE(bool(oo1));
        EXPECT_TRUE(oo1->s == sValueCopyConstructed);
        EXPECT_TRUE(v.s == sValueConstructed);

        Optional<Oracle> oo2{InPlaceInit, std::move(v)};
        EXPECT_TRUE(oo2 != Optional<Oracle>{});
        EXPECT_TRUE(oo2 == oo1);
        EXPECT_TRUE(!!oo2);
        EXPECT_TRUE(bool(oo2));
        EXPECT_TRUE(oo2->s == sValueMoveConstructed);
        EXPECT_TRUE(v.s == sMovedFrom);
    }
};

TEST(Optional, Assignment)
{
    Optional<int> oi;
    oi = Optional<int>{1};
    EXPECT_TRUE(*oi == 1);
    
    oi = 2;
    EXPECT_TRUE(*oi == 2);

    oi = {};
    EXPECT_TRUE(!oi);
};

template <class T>
struct MoveAware
{
    T val;
    bool moved;
    MoveAware(T val) : val(val), moved(false) {}
    MoveAware(MoveAware const&) = delete;
    MoveAware(MoveAware&& rhs) : val(rhs.val), moved(rhs.moved) { rhs.moved = true; }
    MoveAware& operator=(MoveAware const&) = delete;
    MoveAware& operator=(MoveAware&& rhs)
    {
        val = (rhs.val);
        moved = (rhs.moved);
        rhs.moved = true;
        return *this;
    }
};

TEST(Optional, MovedFromState)
{
    // first, test mock:
    MoveAware<int> i{1}, j{2};
    EXPECT_TRUE(i.val == 1);
    EXPECT_TRUE(!i.moved);
    EXPECT_TRUE(j.val == 2);
    EXPECT_TRUE(!j.moved);

    MoveAware<int> k = std::move(i);
    EXPECT_TRUE(k.val == 1);
    EXPECT_TRUE(!k.moved);
    EXPECT_TRUE(i.val == 1);
    EXPECT_TRUE(i.moved);

    k = std::move(j);
    EXPECT_TRUE(k.val == 2);
    EXPECT_TRUE(!k.moved);
    EXPECT_TRUE(j.val == 2);
    EXPECT_TRUE(j.moved);

    // now, test Optional
    Optional<MoveAware<int>> oi{1}, oj{2};
    EXPECT_TRUE(oi);
    EXPECT_TRUE(!oi->moved);
    EXPECT_TRUE(oj);
    EXPECT_TRUE(!oj->moved);

    Optional<MoveAware<int>> ok = std::move(oi);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(!ok->moved);
    EXPECT_TRUE(oi);
    EXPECT_TRUE(oi->moved);

    ok = std::move(oj);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(!ok->moved);
    EXPECT_TRUE(oj);
    EXPECT_TRUE(oj->moved);
};

TEST(Optional, CopyMoveCtorOptionalInt)
{
    Optional<int> oi;
    Optional<int> oj = oi;

    EXPECT_TRUE(!oj);
    EXPECT_TRUE(oj == oi);
    EXPECT_TRUE(!bool(oj));

    oi = 1;
    Optional<int> ok = oi;
    EXPECT_TRUE(!!ok);
    EXPECT_TRUE(bool(ok));
    EXPECT_TRUE(ok == oi);
    EXPECT_TRUE(ok != oj);
    EXPECT_TRUE(*ok == 1);

    Optional<int> ol = std::move(oi);
    EXPECT_TRUE(!!ol);
    EXPECT_TRUE(bool(ol));
    EXPECT_TRUE(ol == oi);
    EXPECT_TRUE(ol != oj);
    EXPECT_TRUE(*ol == 1);
};

TEST(Optional, OptionalOptional)
{
    Optional<Optional<int>> oi1;
    EXPECT_TRUE(!oi1);

    {
        Optional<Optional<int>> oi2 {InPlaceInit};
        EXPECT_TRUE(bool(oi2));
        EXPECT_TRUE(!(*oi2));
        //std::cout << typeid(**oi2).name() << std::endl;
    }

    {
        Optional<Optional<int>> oi2 {Optional<int>{}};
        EXPECT_TRUE(bool(oi2));
        EXPECT_TRUE(!*oi2);
    }

    Optional<int> oi;
    auto ooi = MakeOptional(oi);
    static_assert( std::is_same<Optional<Optional<int>>, decltype(ooi)>::value, "");
};

TEST(Optional, ExampleGuard)
{
    //FAILS: Optional<Guard> ogx(Guard("res1"));
    //FAILS: Optional<Guard> ogx = "res1";
    //FAILS: Optional<Guard> ogx("res1");
    Optional<Guard> oga;                     // Guard is non-copyable (and non-moveable)
    Optional<Guard> ogb(InPlaceInit, "res1");   // initialzes the contained value with "res1"
    EXPECT_TRUE(bool(ogb));
    EXPECT_TRUE(ogb->val == "res1");

    Optional<Guard> ogc(InPlaceInit);        // default-constructs the contained value
    EXPECT_TRUE(bool(ogc));
    EXPECT_TRUE(ogc->val == "");

    oga.Emplace("res1");                     // initialzes the contained value with "res1"
    EXPECT_TRUE(bool(oga));
    EXPECT_TRUE(oga->val == "res1");

    oga.Emplace();                           // destroys the contained value and
    // default-constructs the new one
    EXPECT_TRUE(bool(oga));
    EXPECT_TRUE(oga->val == "");
};

TEST(Optional, BadComparison)
{
    Optional<int> oi, oj;
    int i;
    bool b = (oi == oj);
    b = (oi >= i);
    b = (oi == i);
    static_cast<void>(b);
};

TEST(Optional, Clear)
{
    Optional<int> oi {1};
    oi.Clear();
    EXPECT_TRUE(!oi);

    int i = 1;
    Optional<const int&> oir {i};
    oir.Clear();
    EXPECT_TRUE(!oir);
};

TEST(Optional, MixedOrder)
{
    Optional<int> oN {};
    Optional<int> o0 {0};
    Optional<int> o1 {1};

    EXPECT_TRUE( (oN <   0));
    EXPECT_TRUE( (oN <   1));
    EXPECT_TRUE(!(o0 <   0));
    EXPECT_TRUE( (o0 <   1));
    EXPECT_TRUE(!(o1 <   0));
    EXPECT_TRUE(!(o1 <   1));

    EXPECT_TRUE(!(oN >=  0));
    EXPECT_TRUE(!(oN >=  1));
    EXPECT_TRUE( (o0 >=  0));
    EXPECT_TRUE(!(o0 >=  1));
    EXPECT_TRUE( (o1 >=  0));
    EXPECT_TRUE( (o1 >=  1));

    EXPECT_TRUE(!(oN >   0));
    EXPECT_TRUE(!(oN >   1));
    EXPECT_TRUE(!(o0 >   0));
    EXPECT_TRUE(!(o0 >   1));
    EXPECT_TRUE( (o1 >   0));
    EXPECT_TRUE(!(o1 >   1));

    EXPECT_TRUE( (oN <=  0));
    EXPECT_TRUE( (oN <=  1));
    EXPECT_TRUE( (o0 <=  0));
    EXPECT_TRUE( (o0 <=  1));
    EXPECT_TRUE(!(o1 <=  0));
    EXPECT_TRUE( (o1 <=  1));

    EXPECT_TRUE( (0 >  oN));
    EXPECT_TRUE( (1 >  oN));
    EXPECT_TRUE(!(0 >  o0));
    EXPECT_TRUE( (1 >  o0));
    EXPECT_TRUE(!(0 >  o1));
    EXPECT_TRUE(!(1 >  o1));

    EXPECT_TRUE(!(0 <= oN));
    EXPECT_TRUE(!(1 <= oN));
    EXPECT_TRUE( (0 <= o0));
    EXPECT_TRUE(!(1 <= o0));
    EXPECT_TRUE( (0 <= o1));
    EXPECT_TRUE( (1 <= o1));

    EXPECT_TRUE(!(0 <  oN));
    EXPECT_TRUE(!(1 <  oN));
    EXPECT_TRUE(!(0 <  o0));
    EXPECT_TRUE(!(1 <  o0));
    EXPECT_TRUE( (0 <  o1));
    EXPECT_TRUE(!(1 <  o1));

    EXPECT_TRUE( (0 >= oN));
    EXPECT_TRUE( (1 >= oN));
    EXPECT_TRUE( (0 >= o0));
    EXPECT_TRUE( (1 >= o0));
    EXPECT_TRUE(!(0 >= o1));
    EXPECT_TRUE( (1 >= o1));
};

struct BadRelops
{
    int i;
};

constexpr bool operator<(BadRelops a, BadRelops b) { return a.i < b.i; }
constexpr bool operator>(BadRelops a, BadRelops b) { return a.i < b.i; } // intentional error!

TEST(Optional, BadRelops)
{
    BadRelops a{1}, b{2};
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(a > b);

    Optional<BadRelops> oa = a, ob = b;
    EXPECT_TRUE(oa < ob);
    EXPECT_TRUE(!(oa > ob));

    EXPECT_TRUE(oa < b);
    EXPECT_TRUE(oa > b);

    Optional<BadRelops&> ra = a, rb = b;
    EXPECT_TRUE(ra < rb);
    EXPECT_TRUE(!(ra > rb));

    EXPECT_TRUE(ra < b);
    EXPECT_TRUE(ra > b);
};

TEST(Optional, MixedEquality)
{
    EXPECT_TRUE(MakeOptional(0) == 0);
    EXPECT_TRUE(MakeOptional(1) == 1);
    EXPECT_TRUE(MakeOptional(0) != 1);
    EXPECT_TRUE(MakeOptional(1) != 0);

    Optional<int> oN {};
    Optional<int> o0 {0};
    Optional<int> o1 {1};

    EXPECT_TRUE(o0 ==  0);
    EXPECT_TRUE( 0 == o0);
    EXPECT_TRUE(o1 ==  1);
    EXPECT_TRUE( 1 == o1);
    EXPECT_TRUE(o1 !=  0);
    EXPECT_TRUE( 0 != o1);
    EXPECT_TRUE(o0 !=  1);
    EXPECT_TRUE( 1 != o0);

    EXPECT_TRUE( 1 != oN);
    EXPECT_TRUE( 0 != oN);
    EXPECT_TRUE(oN !=  1);
    EXPECT_TRUE(oN !=  0);
    EXPECT_TRUE(!( 1 == oN));
    EXPECT_TRUE(!( 0 == oN));
    EXPECT_TRUE(!(oN ==  1));
    EXPECT_TRUE(!(oN ==  0));

    std::string cat{"cat"}, dog{"dog"};
    Optional<std::string> oNil{}, oDog{"dog"}, oCat{"cat"};

    EXPECT_TRUE(oCat ==  cat);
    EXPECT_TRUE( cat == oCat);
    EXPECT_TRUE(oDog ==  dog);
    EXPECT_TRUE( dog == oDog);
    EXPECT_TRUE(oDog !=  cat);
    EXPECT_TRUE( cat != oDog);
    EXPECT_TRUE(oCat !=  dog);
    EXPECT_TRUE( dog != oCat);

    EXPECT_TRUE( dog != oNil);
    EXPECT_TRUE( cat != oNil);
    EXPECT_TRUE(oNil !=  dog);
    EXPECT_TRUE(oNil !=  cat);
    EXPECT_TRUE(!( dog == oNil));
    EXPECT_TRUE(!( cat == oNil));
    EXPECT_TRUE(!(oNil ==  dog));
    EXPECT_TRUE(!(oNil ==  cat));
};

TEST(Optional, ConstPropagation)
{
    Optional<int> mmi{0};
    static_assert(std::is_same<decltype(*mmi), int&>::value, "WTF");

    const Optional<int> cmi{0};
    static_assert(std::is_same<decltype(*cmi), const int&>::value, "WTF");

    Optional<const int> mci{0};
    static_assert(std::is_same<decltype(*mci), const int&>::value, "WTF");

    Optional<const int> cci{0};
    static_assert(std::is_same<decltype(*cci), const int&>::value, "WTF");
};

TEST(Optional, OptionalRef)
{
    // FAILS: Optional<int&&> orr;
    // FAILS: Optional<nullopt_t&> on;
    int i = 8;
    Optional<int&> ori;
    EXPECT_TRUE(!ori);
    ori.Emplace(i);
    EXPECT_TRUE(bool(ori));
    EXPECT_TRUE(*ori == 8);
    EXPECT_TRUE(&*ori == &i);
    *ori = 9;
    EXPECT_TRUE(i == 9);

    int j = 22;
    auto&& oj = MakeOptional(std::ref(j));
    *oj = 23;
    EXPECT_TRUE(&*oj == &j);
    EXPECT_TRUE(j == 23);
};

TEST(Optional, OptionalRefConstPropagation)
{
    int i = 9;
    const Optional<int&> mi = i;
    int& r = *mi;
    Optional<const int&> ci = i;
    static_assert(std::is_same<decltype(*mi), int&>::value, "WTF");
    static_assert(std::is_same<decltype(*ci), const int&>::value, "WTF");

    static_cast<void>(r);
};

TEST(Optional, OptionalRefAssign)
{
    int i = 9;
    Optional<int&> ori = i;

    int j = 1;
    ori = Optional<int&>{j};
    ori = {j};
    // FAILS: ori = j;

    Optional<int&> orx = ori;
    ori = orx;

    Optional<int&> orj = j;

    EXPECT_TRUE(ori);
    EXPECT_TRUE(*ori == 1);
    EXPECT_TRUE(ori == orj);
    EXPECT_TRUE(i == 9);

    *ori = 2;
    EXPECT_TRUE(*ori == 2);
    EXPECT_TRUE(ori == 2);
    EXPECT_TRUE(2 == ori);
    EXPECT_TRUE(ori != 3);

    EXPECT_TRUE(ori == orj);
    EXPECT_TRUE(j == 2);
    EXPECT_TRUE(i == 9);

    ori = {};
    EXPECT_TRUE(!ori);
    EXPECT_TRUE(ori != orj);
    EXPECT_TRUE(j == 2);
    EXPECT_TRUE(i == 9);
};

TEST(Optional, OptionalRefSwap)
{
    int i = 0;
    int j = 1;
    Optional<int&> oi = i;
    Optional<int&> oj = j;

    EXPECT_TRUE(&*oi == &i);
    EXPECT_TRUE(&*oj == &j);

    oi.Swap(oj);
    EXPECT_TRUE(&*oi == &j);
    EXPECT_TRUE(&*oj == &i);
};

TEST(Optional, OptionalInitialization)
{
    using std::string;
    string s = "STR";

    Optional<string> os{s};
    Optional<string> ot = s;
    Optional<string> ou{"STR"};
    Optional<string> ov = string{"STR"};
};
