#!/bin/python
# -*- coding: utf-8 -*-
# 生成UNICODE DATA#!/bin/python
import os
import sys
import urllib.request
from functools import cmp_to_key


IDNA_VERSION = "10.0.0"

IDNA_STATUS_VALID = 0
IDNA_STATUS_IGNORED = 1
IDNA_STATUS_MAPPED = 2
IDNA_STATUS_DEVIATION = 3
IDNA_STATUS_DISALLOWED = 4
IDNA_STATUS_DISALLOWED_STD3_VALID = 5
IDNA_STATUS_DISALLOWED_STD3_MAPPED = 6

IDNA_STATUS_ENUMS = {
    "valid": IDNA_STATUS_VALID,
    "ignored": IDNA_STATUS_IGNORED,
    "mapped": IDNA_STATUS_MAPPED,
    "deviation": IDNA_STATUS_DEVIATION,
    "disallowed": IDNA_STATUS_DISALLOWED,
    "disallowed_STD3_valid": IDNA_STATUS_DISALLOWED_STD3_VALID,
    "disallowed_STD3_mapped": IDNA_STATUS_DISALLOWED_STD3_MAPPED,
}

IDNA_2008_STATUS_NONE = 0
IDNA_2008_STATUS_NV8 = 1 << 4
IDNA_2008_STATUS_XV8 = 2 << 4

IDNA_2008_STATUS_ENUMS = {
    "": IDNA_2008_STATUS_NONE,
    "NV8": IDNA_2008_STATUS_NV8,
    "XV8": IDNA_2008_STATUS_XV8,
}

IDNA_EXTRA_MAPPING = 1 << 7


def getsize(data):
    maxdata = max(data)
    if maxdata < 256:
        return 1
    elif maxdata < 65536:
        return 2
    else:
        return 4


def find_in_seq(seq, what):
    for i in range(0, len(seq) - len(what)):
        for j in range(0, len(what)):
            if seq[i + j] != what[j]:
                return -1
        return i
    return -1


def splitbins(t):
    n = len(t)-1    # last valid index
    maxshift = 0    # the most we can shift n and still have something left
    if n > 0:
        while n >> 1:
            n >>= 1
            maxshift += 1
    del n
    bytes = sys.maxsize  # smallest total size so far
    t = tuple(t)    # so slices can be dict keys
    for shift in range(maxshift + 1):
        t1 = []
        t2 = []
        size = 2**shift
        bincache = {}
        for i in range(0, len(t), size):
            bin = t[i:i+size]
            index = bincache.get(bin)
            if index is None:
                index = len(t2)
                bincache[bin] = index
                t2.extend(bin)
            t1.append(index >> shift)
        # determine memory size
        b = len(t1)*getsize(t1) + len(t2)*getsize(t2)
        if b < bytes:
            best = t1, t2, shift
            bytes = b
    t1, t2, shift = best
    return best


class Array:
    def __init__(self, name, data, as_char32=False):
        self.name = name
        self.data = data
        self.as_char32 = as_char32

    def dump(self, file):
        size = getsize(self.data)
        file.write("static const ")
        if self.as_char32:
            file.write("char32_t")
        else:
            if size == 1:
                file.write("uint8_t")
            elif size == 2:
                file.write("uint16_t")
            else:
                file.write("uint32_t")
        file.write(" " + self.name + "[] = {\n")
        if self.data:
            s = "    "
            for item in self.data:
                i = str(item) + ", "
                if len(s) + len(i) > 78:
                    file.write(s.rstrip() + "\n")
                    s = "    " + i
                else:
                    s = s + i
            if s.strip():
                file.write(s.rstrip() + "\n")
        file.write("};\n\n")


def mapping_cmp(a, b):
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


def open_data(template, version):
    local = os.path.join("tmp/", template % ('-' + version, ))
    if not os.path.exists(local):
        if not os.path.exists("tmp/"):
            os.mkdir("tmp/")
        url = ('http://www.unicode.org/Public/idna/%s/' + template) % (version, '')
        urllib.request.urlretrieve(url, filename=local)
    return open(local, encoding='utf-8')


def process():
    map = [None] * 0x110000
    print("Reading data...")
    with open_data("IdnaMappingTable%s.txt", IDNA_VERSION) as f:
        while True:
            line = f.readline()
            if not line:
                break
            line = line.split('#')[0].strip()
            if len(line) == 0:
                continue
            data = line.split(';')
            cp_range = data[0].strip()
            status = data[1].strip()
            mapping = []
            if len(data) > 2:
                for x in data[2].strip().split(' '):
                    ch = x.strip()
                    if len(ch) != 0:
                        mapping.append(int(ch, 16))
            idna2008_status = data[3].strip() if len(data) > 3 else ""
            if ".." in cp_range:
                cp_range = cp_range.split("..")
                begin = int(cp_range[0], 16)
                end = int(cp_range[1], 16)
                assert begin < end
            else:
                begin = end = int(cp_range, 16)
            for ch in range(begin, end + 1):
                map[ch] = (IDNA_STATUS_ENUMS[status], tuple(mapping), IDNA_2008_STATUS_ENUMS[idna2008_status])
    print("Processing mapping data...")
    all_mapping_pairs = []
    for i in range(0, len(map)):
        mapping = map[i][1]
        if len(mapping) > 1 and mapping not in all_mapping_pairs:
            all_mapping_pairs.append(mapping)
    all_mapping_pairs.sort(key=cmp_to_key(mapping_cmp))
    mapping_pair_data = []
    mapping_pair_cache = {}
    for i in range(0, len(all_mapping_pairs)):
        pair = all_mapping_pairs[i]
        if pair in mapping_pair_cache:
            break
        index = find_in_seq(mapping_pair_data, pair)
        if index == -1:
            index = len(mapping_pair_data)
            for j in range(0, len(pair)):
                mapping_pair_data.append(pair[j])
        mapping_pair_cache[pair] = index
    print("Assign mapping data...")
    for i in range(0, len(map)):
        mapping = map[i][1]
        flags = map[i][0] | map[i][2]
        if len(mapping) == 0:
            mapping_index = 0
        elif len(mapping) == 1:
            mapping_index = map[i][1][0]
        else:
            mapping_index = mapping_pair_cache[map[i][1]]
            mapping_index = mapping_index | (len(map[i][1]) << 16)
            flags |= IDNA_EXTRA_MAPPING
        map[i] = (flags, mapping_index)
    print("Collect records...")
    record_cache = {(0, 0): 0}
    record_data = [(0, 0)]
    idna_index = []
    for i in range(0, len(map)):
        if map[i] not in record_cache:
            index = len(record_data)
            record_data.append(map[i])
            record_cache[map[i]] = index
        else:
            index = record_cache[map[i]]
        idna_index.append(index)
    print("Write out data...")
    print("kIdnaMappingData = %d" % len(mapping_pair_data))
    print("kIdnaRecords = %d" % len(record_data))
    with open("../src/IdnaData.inl", "w", encoding="utf-8") as f:
        print("static const unsigned kIdnaRecordsCount = %d;" % len(map), file=f)
        print("", file=f)
        Array("kIdnaMappingData", mapping_pair_data, True).dump(f)
        print("static const IdnaRecord kIdnaRecords[] = {", file=f)
        for i in range(0, len(record_data)):
            print("    {%d, %d}," % (record_data[i][0], record_data[i][1]), file=f)
        print("};", file=f)

        index1, index2, shift = splitbins(idna_index)
        print('', file=f)
        print("static const unsigned kIdnaRecordsIndexShift = %d;" % shift, file=f)
        Array("kIdnaRecordsIndex1", index1).dump(f)
        Array("kIdnaRecordsIndex2", index2).dump(f)


if __name__ == "__main__":
    process()
