//#include "Header.hpp"
#include "SingularityBuilder.hpp"
#include "LatticeBuilder.hpp"

/*
  Testing for singularity builder, the class whose reponsibility is generating point-mus (clusters)
  from some stream of data.
*/
int main(void)
{
  long double res, scalar = 1000000000.0;
  struct timespec begin, end;
  vector<string> output;
  vector<Point> inData;
  vector<PointMu> outData;
  Lattice testLattice;
  LatticePaths decoded;

  //class components
  SingularityBuilder sb(0,900,0,310);
  LatticeBuilder lb;

  sb.UnitTests();

  return 0;
}
