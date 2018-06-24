#!/bin/python
# -*- coding: utf-8 -*-
# 生成UNICODE DATA
import os
import re
import sys
import logging
import urllib.request
import unicodedata
import collections
from functools import cmp_to_key


def get_element_size(data: list) -> int:
    m = max(data)
    assert m >= 0
    if m < 256:
        return 1
    elif m < 65536:
        return 2
    else:
        return 4


def find(arr: list, v: list) -> int:
    for i in range(0, len(arr) - len(v)):
        for j in range(0, len(v)):
            if arr[i + j] != v[j]:
                return -1
        return i
    return -1


def split_array(t: list):
    n = len(t) - 1
    max_shift = 0
    best = [], [], 0
    if n > 0:
        while n >> 1:
            n >>= 1
            max_shift += 1
    best_size = sys.maxsize
    t = tuple(t)
    for shift in range(max_shift + 1):
        t1 = []
        t2 = []
        size = 2 ** shift
        cache = {}
        for i in range(0, len(t), size):
            s = t[i:i + size]
            index = cache.get(s)
            if index is None:
                index = len(t2)
                cache[s] = index
                t2.extend(s)
            t1.append(index >> shift)
        b = len(t1) * get_element_size(t1) + len(t2) * get_element_size(t2)
        if b < best_size:
            best = t1, t2, shift
            best_size = b
    return best


class Array:
    def __init__(self, name, data, as_char32=False):
        self._name = name
        self._data = data
        self._as_char32 = as_char32

    def dump(self, file):
        size = get_element_size(self._data)
        file.write("static const ")
        if self._as_char32:
            file.write("char32_t")
        else:
            if size == 1:
                file.write("uint8_t")
            elif size == 2:
                file.write("uint16_t")
            else:
                assert size == 4
                file.write("uint32_t")
        file.write(" " + self._name + "[] = {\n")
        if self._data:
            s = "    "
            for item in self._data:
                i = str(item) + ", "
                if len(s) + len(i) > 78:
                    file.write(s.rstrip() + "\n")
                    s = "    " + i
                else:
                    s = s + i
            if s.strip():
                file.write(s.rstrip() + "\n")
        file.write("};\n")


# Bits 0~3
IDNA_STATUS_VALID = 0
IDNA_STATUS_VALID_NV8 = 1
IDNA_STATUS_VALID_XV8 = 2
IDNA_STATUS_DISALLOWED = 3
IDNA_STATUS_IGNORED = 4
IDNA_STATUS_MAPPED = 5
IDNA_STATUS_DEVIATION = 6
IDNA_STATUS_DISALLOWED_STD3_VALID = 7
IDNA_STATUS_DISALLOWED_STD3_MAPPED = 8

# Bits 4~5
IDNA_PVALID = 1 << 4
IDNA_CONTEXT_J = 2 << 4
IDNA_CONTEXT_O = 3 << 4

# Bits 7
IDNA_EXTRA_MAPPING = 1 << 7

# Exceptions are manually assigned in Section 2.6 of RFC 5892.
IDNA2008_EXCEPTIONS = {
    0x00DF: "PVALID",      # LATIN SMALL LETTER SHARP S
    0x03C2: "PVALID",      # GREEK SMALL LETTER FINAL SIGMA
    0x06FD: "PVALID",      # ARABIC SIGN SINDHI AMPERSAND
    0x06FE: "PVALID",      # ARABIC SIGN SINDHI POSTPOSITION MEN
    0x0F0B: "PVALID",      # TIBETAN MARK INTERSYLLABIC TSHEG
    0x3007: "PVALID",      # IDEOGRAPHIC NUMBER ZERO
    0x00B7: "CONTEXTO",    # MIDDLE DOT
    0x0375: "CONTEXTO",    # GREEK LOWER NUMERAL SIGN (KERAIA)
    0x05F3: "CONTEXTO",    # HEBREW PUNCTUATION GERESH
    0x05F4: "CONTEXTO",    # HEBREW PUNCTUATION GERSHAYIM
    0x30FB: "CONTEXTO",    # KATAKANA MIDDLE DOT
    0x0660: "CONTEXTO",    # ARABIC-INDIC DIGIT ZERO
    0x0661: "CONTEXTO",    # ARABIC-INDIC DIGIT ONE
    0x0662: "CONTEXTO",    # ARABIC-INDIC DIGIT TWO
    0x0663: "CONTEXTO",    # ARABIC-INDIC DIGIT THREE
    0x0664: "CONTEXTO",    # ARABIC-INDIC DIGIT FOUR
    0x0665: "CONTEXTO",    # ARABIC-INDIC DIGIT FIVE
    0x0666: "CONTEXTO",    # ARABIC-INDIC DIGIT SIX
    0x0667: "CONTEXTO",    # ARABIC-INDIC DIGIT SEVEN
    0x0668: "CONTEXTO",    # ARABIC-INDIC DIGIT EIGHT
    0x0669: "CONTEXTO",    # ARABIC-INDIC DIGIT NINE
    0x06F0: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT ZERO
    0x06F1: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT ONE
    0x06F2: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT TWO
    0x06F3: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT THREE
    0x06F4: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT FOUR
    0x06F5: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT FIVE
    0x06F6: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT SIX
    0x06F7: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT SEVEN
    0x06F8: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT EIGHT
    0x06F9: "CONTEXTO",    # EXTENDED ARABIC-INDIC DIGIT NINE
    0x0640: "DISALLOWED",  # ARABIC TATWEEL
    0x07FA: "DISALLOWED",  # NKO LAJANYALAN
    0x302E: "DISALLOWED",  # HANGUL SINGLE DOT TONE MARK
    0x302F: "DISALLOWED",  # HANGUL DOUBLE DOT TONE MARK
    0x3031: "DISALLOWED",  # VERTICAL KANA REPEAT MARK
    0x3032: "DISALLOWED",  # VERTICAL KANA REPEAT WITH VOICED SOUND MARK
    0x3033: "DISALLOWED",  # VERTICAL KANA REPEAT MARK UPPER HALF
    0x3034: "DISALLOWED",  # VERTICAL KANA REPEAT WITH VOICED SOUND MARK UPPER HA
    0x3035: "DISALLOWED",  # VERTICAL KANA REPEAT MARK LOWER HALF
    0x303B: "DISALLOWED",  # VERTICAL IDEOGRAPHIC ITERATION MARK
}

IDNA2008_BACKWARDS_COMPATIBLE = {}

# Scripts affected by IDNA contextual rules
IDNA_SCRIPT_WHITELIST = ["Greek", "Han", "Hebrew", "Hiragana", "Katakana"]


class UniChar:
    def __init__(self, ch):
        self.ch = ch
        self.status = IDNA_STATUS_VALID
        self.mapping = []
        self.prop_list = []
        self.blocks = ""
        self.hst = ""
        self.as_join_type = ""
        self.scripts = ""

    def to_nfkc_cf(self):
        return unicodedata.normalize("NFKC", unicodedata.normalize("NFKC", chr(self.ch)).casefold())

    def is_ldh(self):
        if self.ch == 0x002d or self.ch in range(0x0030, 0x0039 + 1) or self.ch in range(0x0061, 0x007a + 1):
            return True
        return False

    def is_join_control(self):
        return "Join_Control" in self.prop_list

    def is_unstable(self):
        return chr(self.ch) != self.to_nfkc_cf()

    def in_ignorable_properties(self):
        if "Default_Ignorable_Code_Point" in self.prop_list:
            return True
        elif "White_Space" in self.prop_list:
            return True
        elif "Noncharacter_Code_Point" in self.prop_list:
            return True
        return False

    def in_ignorable_blocks(self):
        return self.blocks in ("Combining Diacritical Marks for Symbols", "Musical Symbols",
                               "Ancient Greek Musical Notation")

    def is_oldhanguljamo(self):
        return self.hst in ('L', 'V', 'T')

    def in_lettersdigits(self):
        return unicodedata.category(chr(self.ch)) in ('Ll', 'Lu', 'Lo', 'Nd', 'Lm', 'Mn', 'Mc')

    def get_idna2008_status(self):
        if self.ch in IDNA2008_EXCEPTIONS:
            return IDNA2008_EXCEPTIONS[self.ch]
        elif self.ch in IDNA2008_BACKWARDS_COMPATIBLE:
            return IDNA2008_BACKWARDS_COMPATIBLE[self.ch]
        elif self.is_ldh():
            return "PVALID"
        elif self.is_join_control():
            return "CONTEXTJ"
        elif self.is_unstable():
            return "DISALLOWED"
        elif self.in_ignorable_properties():
            return "DISALLOWED"
        elif self.in_ignorable_blocks():
            return "DISALLOWED"
        elif self.is_oldhanguljamo():
            return "DISALLOWED"
        elif self.in_lettersdigits():
            return "PVALID"
        else:
            return "DISALLOWED"


class IDNAProcessor:
    @staticmethod
    def _open_idna_data(template, version):
        local = os.path.join("tmp/", template % ('-' + version, ))
        if not os.path.exists(local):
            if not os.path.exists("tmp/"):
                os.mkdir("tmp/")
            url = ("http://www.unicode.org/Public/idna/%s/" + template) % (version, "")
            urllib.request.urlretrieve(url, filename=local)
        return open(local, encoding="utf-8")

    @staticmethod
    def _open_ucd_data(template, version):
        local = os.path.join("tmp/", template % ('-'+version,))
        if not os.path.exists(local):
            if not os.path.exists("tmp/"):
                os.mkdir("tmp/")
            url = ("http://www.unicode.org/Public/%s/ucd/" + template) % (version, "")
            urllib.request.urlretrieve(url, filename=local)
        return open(local, encoding="utf-8")

    @staticmethod
    def _split_ucd_line(f):
        while True:
            line = f.readline()
            if not line:
                return None
            line = line.split('#')[0].strip()
            if len(line) == 0:
                continue
            return [t.strip() for t in line.split(';')]

    def __init__(self, version="10.0.0"):
        self._chars = [None] * 0x110000  # type: list[UniChar]
        self._version = version

    def _read_idna_mapping_table(self):
        logging.info("Reading IdnaMappingTable%s.txt" % self._version)
        with IDNAProcessor._open_idna_data("IdnaMappingTable%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                status = data[1]
                mapping = []
                if len(data) > 2:
                    for ch in data[2].split(' '):
                        ch = ch.strip()
                        if len(ch) != 0:
                            mapping.append(int(ch, 16))
                idna2008_status = data[3] if len(data) > 3 else None
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    ch = UniChar(ch)
                    if status == "valid":
                        ch.status = IDNA_STATUS_VALID
                        if idna2008_status == "NV8":
                            ch.status = IDNA_STATUS_VALID_NV8
                        elif idna2008_status == "XV8":
                            ch.status = IDNA_STATUS_VALID_XV8
                        else:
                            assert idna2008_status is None
                    elif status == "ignored":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_IGNORED
                    elif status == "mapped":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_MAPPED
                    elif status == "deviation":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_DEVIATION
                    elif status == "disallowed":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_DISALLOWED
                    elif status == "disallowed_STD3_valid":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_DISALLOWED_STD3_VALID
                    elif status == "disallowed_STD3_mapped":
                        assert idna2008_status is None
                        ch.status = IDNA_STATUS_DISALLOWED_STD3_MAPPED
                    else:
                        assert False
                    ch.mapping = tuple(mapping)
                    self._chars[ch.ch] = ch

    def _read_ucd_prop_list(self):
        with IDNAProcessor._open_ucd_data("PropList%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    self._chars[ch].prop_list.append(data[1])

    def _read_ucd_derived_core_props(self):
        with IDNAProcessor._open_ucd_data("DerivedCoreProperties%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    self._chars[ch].prop_list.append(data[1])

    def _read_ucd_blocks(self):
        with IDNAProcessor._open_ucd_data("Blocks%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    assert self._chars[ch].blocks == ""
                    self._chars[ch].blocks = data[1]

    def _read_ucd_hangu_list(self):
        with IDNAProcessor._open_ucd_data("HangulSyllableType%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    assert self._chars[ch].hst == ""
                    self._chars[ch].hst = data[1]

    def _read_ucd_arabic_shaping(self):
        with IDNAProcessor._open_ucd_data("ArabicShaping%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end + 1):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    assert self._chars[ch].as_join_type == ""
                    self._chars[ch].as_join_type = data[2]

    def _read_ucd_scripts(self):
        with IDNAProcessor._open_ucd_data("Scripts%s.txt", self._version) as f:
            while True:
                data = IDNAProcessor._split_ucd_line(f)
                if not data:
                    break
                ranges = data[0]
                if ".." in ranges:
                    ranges = ranges.split("..")
                    begin = int(ranges[0], 16)
                    end = int(ranges[1], 16)
                    assert begin < end
                else:
                    begin = end = int(ranges, 16)
                for ch in range(begin, end):
                    assert ch < len(self._chars)
                    if self._chars[ch] is None:
                        self._chars[ch] = UniChar(ch)
                    assert self._chars[ch].scripts == ""
                    self._chars[ch].scripts = data[1]

    def _write_idna_mapping_table(self, f):
        def cmp(a, b):
            if len(a) < len(b):
                return 1
            elif len(a) > len(b):
                return -1
            for i in range(0, len(a)):
                if a[i] < b[i]:
                    return 1
                elif a[i] > b[i]:
                    return -1
            return 0

        logging.info("Expanding mapping data")
        mapping_pairs = []
        for ch in self._chars:
            if ch is not None:
                mapping = ch.mapping
                if len(mapping) > 1 and mapping not in mapping_pairs:
                    mapping_pairs.append(mapping)
        mapping_pairs.sort(key=cmp_to_key(cmp))
        mapping_pairs_data = []
        mapping_pairs_cache = {}
        for pair in mapping_pairs:
            if pair in mapping_pairs_cache:
                break
            i = find(mapping_pairs, pair)
            if i == -1:
                i = len(mapping_pairs)
                for j in range(0, len(pair)):
                    mapping_pairs_data.append(pair[j])
            mapping_pairs_cache[pair] = i

        logging.info("Assigning records")
        records = {}
        for ch in self._chars:
            if ch is not None:
                flags = ch.status
                assert flags <= 15
                if len(ch.mapping) == 0:
                    mapping_index = 0
                elif len(ch.mapping) == 1:
                    mapping_index = ch.mapping[0]
                else:
                    mapping_index = mapping_pairs_cache[ch.mapping]
                    assert mapping_index <= 65535
                    mapping_index = mapping_index | (len(ch.mapping) << 16)
                    assert len(ch.mapping) <= 65535
                    flags |= IDNA_EXTRA_MAPPING
                idna2008_status = ch.get_idna2008_status()
                if idna2008_status == "PVALID":
                    flags |= IDNA_PVALID
                elif idna2008_status == "CONTEXTJ":
                    flags |= IDNA_CONTEXT_J
                elif idna2008_status == "CONTEXTO":
                    flags |= IDNA_CONTEXT_O
                records[ch.ch] = (flags, mapping_index)

        logging.info("Compressing records")
        records_data = [(IDNA_STATUS_VALID, 0)]
        records_cache = {(IDNA_STATUS_VALID, 0): 0}  # 默认VALID
        records_index = []
        for ch in self._chars:
            if ch is not None:
                if records[ch.ch] not in records_cache:
                    index = len(records_data)
                    records_data.append(records[ch.ch])
                    records_cache[records[ch.ch]] = index
                else:
                    index = records_cache[records[ch.ch]]
                records_index.append(index)
            else:
                records_index.append(0)

        logging.info("Writing IDNA mapping data")
        logging.info("len(kIdnaMappingData) = %d" % len(mapping_pairs_data))
        logging.info("len(kIdnaRecords) = %d" % len(records_data))
        print("static const unsigned kIdnaRecordsCount = %d;" % len(records_index), file=f)
        print("", file=f)
        Array("kIdnaMappingData", mapping_pairs_data, True).dump(f)
        print("", file=f)
        print("static const IdnaRecord kIdnaRecords[] = {", file=f)
        for i in range(0, len(records_data)):
            print("    {%d, %d}," % (records_data[i][0], records_data[i][1]), file=f)
        print("};", file=f)

        print("", file=f)
        index1, index2, shift = split_array(records_index)
        print("static const unsigned kIdnaRecordsIndexShift = %d;" % shift, file=f)
        print("", file=f)
        Array("kIdnaRecordsIndex1", index1).dump(f)
        print("", file=f)
        Array("kIdnaRecordsIndex2", index2).dump(f)

    def _write_join_type(self, f):
        print("", file=f)
        print("static char GetJoiningType(char32_t ch)noexcept", file=f)
        print("{", file=f)
        print("    switch (ch)", file=f)
        print("    {", file=f)
        for ch in self._chars:
            if ch.as_join_type != "":
                assert len(ch.as_join_type) == 1
                print("        case %d:" % ch.ch, file=f)
                print("            return '%s';" % ch.as_join_type, file=f)
        print("        default:", file=f)
        print("            return 0;", file=f)
        print("    }", file=f)
        print("}", file=f)

    def _write_scripts(self, f):
        for script in IDNA_SCRIPT_WHITELIST:
            print("", file=f)
            print("static bool IsIn%sScript(char32_t ch)noexcept" % script, file=f)
            print("{", file=f)
            bools = [False] * len(self._chars)
            for ch in self._chars:
                if ch.scripts == script:
                    bools[ch.ch] = True
            first = True
            begin = end = 0
            for ch in range(0, len(bools) + 1):
                if ch < len(bools) and bools[ch]:
                    if begin == 0:
                        begin = ch
                    end = ch
                else:
                    if begin != 0:
                        assert begin <= end
                        cond = "if" if first else "else if"
                        if begin == end:
                            print("    %s (%d == ch)" % (cond, begin), file=f)
                            print("        return true;", file=f)
                        else:
                            print("    %s (%d <= ch && ch <= %d)" % (cond, begin, end), file=f)
                            print("        return true;", file=f)
                        first = False
                    begin = end = 0
            print("    return false;", file=f)
            print("}", file=f)

    def process(self, filename="../src/IdnaData.inl"):
        self._read_idna_mapping_table()
        self._read_ucd_prop_list()
        self._read_ucd_derived_core_props()
        self._read_ucd_blocks()
        self._read_ucd_hangu_list()
        self._read_ucd_arabic_shaping()
        self._read_ucd_scripts()
        with open(filename, "w", encoding="utf-8") as f:
            self._write_idna_mapping_table(f)
            self._write_join_type(f)
            self._write_scripts(f)


if __name__ == "__main__":
    processor = IDNAProcessor()
    processor.process()
