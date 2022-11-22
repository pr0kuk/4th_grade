#include <iostream>
#include <vector>
#include <array>

constexpr double a = 1000;
constexpr double step = 0.1;
constexpr size_t size = 20.0 / step - 2;

double f(double data) {
  return a * (data - data * data * data);
}
double rightSizeIn (double lhs , double mid , double rhs)
{
  return mid + (lhs - 2 * mid + rhs) / 12;
}
double rightSize (double lhs , double mid , double rhs)
{
  return rightSizeIn(f(lhs), f(mid), f(rhs));
}

using vector = std::array<double , size>;
using matrix = std::array<std::array<double , size> , size>;

matrix initA ()
{
  constexpr double mid = -2.0 / (step * step);
  constexpr double side = 1.0 / (step * step);
  matrix A;
  A[0][0] = mid;
  A[0][1] = size;
  A[size - 1][size - 1] = mid;
  A[size - 1][size - 2] = side;
  for (int i = 1; i < size - 1; ++i) {
    A[i][i - 1] = side;
    A[i][i] = mid;
    A[i][i + 1] = side;
  }
  return A;
}
vector initY ()
{
  vector Y;
  for (auto& y : Y)
    y = 1.41;
  return Y;
}
vector calcF (const vector& vec) {
  vector F;
  F[0] = rightSize(1.41, vec[0], vec[1]);
  F[size - 1] = rightSize (vec[size - 2] , vec[size - 1], 1.41);
  for (int i = 1; i < size - 1; ++i) {
    F[i] = rightSize(vec[i - 1], vec[i], vec[i + 1]);
  }
  return F;
}

vector operator - (const vector& lhs, const vector& rhs) {
  vector out;
  for (int i = 0; i < size; ++i)
    out[i] = lhs[i] - rhs[i];
  return out;
}

vector operator * (const matrix& lhs , const vector& rhs)
{
  vector out;
  for (int i = 0; i < size; ++i) {
    out[i] = 0;
    for (int j = 0; j < size; ++j)
      out[i] += lhs[i][j] * rhs[j];
  }
  return out;
}




int main() {
  auto A = initA();
  auto Y = initY();
  auto F = calcF(Y);
  auto dif = F - A * Y;
  
}