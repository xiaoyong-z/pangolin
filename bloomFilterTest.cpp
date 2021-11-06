// Copyright (c) 2012 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#include "bloomFilter.cpp"
#include "gtest/gtest.h"

static const int kVerbose = 1;

inline void EncodeFixed32(char* dst, uint32_t value) {
  uint8_t* const buffer = reinterpret_cast<uint8_t*>(dst);

  // Recent clang and gcc optimize this to a single mov / str instruction.
  buffer[0] = static_cast<uint8_t>(value);
  buffer[1] = static_cast<uint8_t>(value >> 8);
  buffer[2] = static_cast<uint8_t>(value >> 16);
  buffer[3] = static_cast<uint8_t>(value >> 24);
}
class BloomTest : public testing::Test {
   public:
    BloomTest(){}

    ~BloomTest() {}

    void Reset() {
        keys_.clear();
        filter_.clear();
    }

    void Add(const std::string& s) { 
        keys_.push_back(BloomFilter::Hash(s.c_str())); 
    }

    void Build() {
        filter_.clear();
        int bits_per_key = BloomFilter::calBitsPerKey(keys_.size(), 0.007);
        BloomFilter::createFilter(keys_, filter_, bits_per_key);
        keys_.clear();
        if (kVerbose >= 2) 
            DumpFilter();
    }

    size_t FilterSize() const { return filter_.size(); }

    void DumpFilter() {
        std::fprintf(stderr, "F(");
        for (size_t i = 0; i + 1 < filter_.size(); i++) {
            const unsigned int c = static_cast<unsigned int>(filter_[i]);
            for (int j = 0; j < 8; j++) {
                std::fprintf(stderr, "%c", (c & (1 << j)) ? '1' : '.');
            }
        }
        std::fprintf(stderr, ")\n");
    }

    bool Matches(const std::string& s) {
        if (!keys_.empty()) {
            Build();
        }
        return BloomFilter::Contains(s.c_str(), filter_);
    }

    double FalsePositiveRate() {
        char buffer[sizeof(int)];
        int result = 0;
        for (int i = 0; i < 10000; i++) {
            EncodeFixed32(buffer, i + 1000000000);
            if (Matches(std::string(buffer))) {
                result++;
            }
        }
        return result / 10000.0;
    }

   private:
    std::vector<char> filter_;
    std::vector<uint32_t> keys_;
};

TEST_F(BloomTest, EmptyFilter) {
    ASSERT_TRUE(!Matches("hello"));
    ASSERT_TRUE(!Matches("world"));
}

TEST_F(BloomTest, Small) {
    Add("hello");
    Add("world");
    ASSERT_TRUE(Matches("hello"));
    ASSERT_TRUE(Matches("world"));
    ASSERT_TRUE(!Matches("x"));
    ASSERT_TRUE(!Matches("foo"));
}

static int NextLength(int length) {
    if (length < 10) {
        length += 1;
    } else if (length < 100) {
        length += 10;
    } else if (length < 1000) {
        length += 100;
    } else {
        length += 1000;
    }
    return length;
}

TEST_F(BloomTest, VaryingLengths) {
    char buffer[sizeof(int)];

    // Count number of filters that significantly exceed the false positive rate
    int mediocre_filters = 0;
    int good_filters = 0;

    for (int length = 1; length <= 10000; length = NextLength(length)) {
        Reset();
        for (int i = 0; i < length; i++) {
            EncodeFixed32(buffer, i);
            Add(std::string(buffer));
        }
        Build();

        ASSERT_LE(FilterSize(), static_cast<size_t>((length * 10 / 7) + 40)) << length;

        // All added keys must match
        for (int i = 0; i < length; i++) {
            EncodeFixed32(buffer, i);
            ASSERT_TRUE(Matches(std::string(buffer)))
                << "Length " << length << "; key " << i;
        }

        // Check false positive rate
        double rate = FalsePositiveRate();
        if (kVerbose >= 1) {
            std::fprintf(
                stderr,
                "False positives: %5.2f%% @ length = %6d ; bytes = %6d\n",
                rate * 100.0, length, static_cast<int>(FilterSize()));
        }
        ASSERT_LE(rate, 0.025);  // Must not be over 25%
        if (rate > 0.0125)
            mediocre_filters++;  // Allowed, but not too often
        else
            good_filters++;
    }
    if (kVerbose >= 1) {
        std::fprintf(stderr, "Filters: %d good, %d mediocre\n", good_filters,
                     mediocre_filters);
    }
    ASSERT_LE(mediocre_filters, good_filters / 3);
}

// Different bits-per-byte

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
