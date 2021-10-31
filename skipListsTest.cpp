// Copyright (c) 2011 The LevelDB Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.


#include <atomic>
#include <set>

#include "gtest/gtest.h"
#include <string>
#include "skipLists.cpp"

typedef uint64_t Key;

struct Comparator {
  int operator()(const Key& a, const Key& b) const {
    if (a < b) {
      return -1;
    } else if (a > b) {
      return +1;
    } else {
      return 0;
    }
  }
};

std::string RandString(int len){
    char* bytes = new char[len];
    for (int i = 0; i < len; i++) {
        char b = rand()%256;
        bytes[i] = b;
    }
	return std::string(bytes);
}

class TestMove{
public:
    TestMove(double score) {
        std::cout << "default constructor" << std::endl;
        score_ = score;
    }

    TestMove(TestMove&& another) {
        std::cout << "move constrcutor" << std::endl;
    }
private:
    TestMove(const TestMove& another) {
        std::cout << "copy constructor" << std::endl;
    }

    double score_;
};

TEST(SkipTest, BasicCRUD) {
    TestMove a(1.2);
    TestMove b(2.3);
    std::cout << "generate" << std::endl;
    Entry<TestMove, TestMove>* entry1 = new Entry<TestMove, TestMove>(std::move(a), std::move(b));
    
    SkipList<TestMove, TestMove> skipList;

    skipList.insert(std::move(*entry1));

//   Arena arena;
//   Comparator cmp;
//   SkipList<Key, Comparator> list(cmp, &arena);
//   ASSERT_TRUE(!list.Contains(10));

//   SkipList<Key, Comparator>::Iterator iter(&list);
//   ASSERT_TRUE(!iter.Valid());
//   iter.SeekToFirst();
//   ASSERT_TRUE(!iter.Valid());
//   iter.Seek(100);
//   ASSERT_TRUE(!iter.Valid());
//   iter.SeekToLast();
//   ASSERT_TRUE(!iter.Valid());
}



int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

