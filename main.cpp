//
// Created by staf02 on 21/08/22.
//
#include <iostream>
#include "socow-vector.h"

int main() {
  socow_vector<size_t, 3> a;
  a.reserve(10);
  size_t n = a.capacity();
  for (size_t i = 0; i != n; ++i)
    a.push_back(i);
  return 0;
}