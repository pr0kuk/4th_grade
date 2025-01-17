#include "common.hpp"

int main() {
  
  // To show how algorithm works

  // -----------------------------
  auto A_1 = initA_1();
  auto A = initA();
  auto Y = initY();
  double mer = 10.0;
  int curIter = 1;
  
  while (mer > 0.001) {
    auto F = Seq::calcF (Y);
    auto dif = Seq::Minus(F , Seq::Mult (A , Y));
    Y = Seq::Plus (Y , Seq::Div (Seq::Mult (A_1 , dif) , 10));
    mer = dumpMer (dif , std::to_string (curIter));
    curIter++;
  }
  // -----------------------------

  // To measure sequential work

  // -----------------------------
  A_1 = initA_1 ();
  A = initA ();
  Y = initY ();
  mer = 10.0;
  curIter = 0;
  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now ();
  while (mer > 0.001)
  {
    auto F = Seq::calcF (Y);
    auto dif = Seq::Minus (F , Seq::Mult (A , Y));
    Y = Seq::Plus (Y , Seq::Div (Seq::Mult (A_1 , dif) , 10));
    mer = Seq::dumpMer (dif);
    ++curIter;
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now ();
  std::cout << "Sequential time : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count () << "[µs]" << std::endl;
  std::cout << "Used " << curIter << " iterations\n";
  // -----------------------------

  // To measure parallel work

  // -----------------------------
  A_1 = initA_1 ();
  A = initA ();
  Y = initY ();
  mer = 10.0;
  curIter = 0;
  begin = std::chrono::steady_clock::now ();
  while (mer > 0.001)
  {
    auto F = Par::calcF (Y);
    auto dif = Par::Minus (F , Par::Mult (A , Y));
    Y = Par::Plus (Y , Par::Div (Par::Mult (A_1 , dif) , 10));
    mer = Par::dumpMer (dif);
    ++curIter;
  }
  end = std::chrono::steady_clock::now ();
  std::cout << "Parallel time : " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count () << "[µs]" << std::endl;
  std::cout << "Used " << curIter << " iterations\n";
  // -----------------------------
}