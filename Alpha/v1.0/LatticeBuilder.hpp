#include "Header.hpp"

#ifndef LATTICEBUILDER_HPP
#define LATTICEBUILDER_HPP


/*
  SingularityBuilder takes a raw signal and converts it to an ordered list of
  physical points. Conditioner's primary responsibility is taking that vector
  of points and converting it into a stochastic structure, a directed graph where
  each arc has an associated probability (a letter lattice).

  NOTES: I'm going to simplify the graph and assume a directed lattice, instead of accounting
  for skip-arcs (an arc skipping over sets of nodes, etc) or other complex relationships. It's
  better to define this as "error" to be minimized, for now.

  Builds and conditions the lattice. The only conditioning data for the arcs, as yet, is bigram
  data. Its not clear what other parameters might be used as input to arc weights. State holds the internal
  probability of a state, given its physically-determined probability.

  Revision: Its still possible to condition the lattice on construction, but I determined its better not to.
  The only possible conditioning data as yet is n-gram data; however, only unigram and bigram data may be
  applied directly to arcs. Trigram and other higher-order n-gram data cannot condition arcs directly, since they
  break the dynamic programming nature of the lattice, and require a path of greater than two states (iow, there
  are multiple trigram sources for a given arc: draw this out if you don't understand why). So higher-order conditioning
  data can only be applied when searching the graph, but even this incurs incredibly high cost.
  Therefore, since the overall responsibility of this class is only to output some reprerentation of most likely letter permutations
  based on physical input, its far better not to condition, either when building the graph, or when searching it to generate
  most likely permuations.  Enumerating all graph paths is an exponential task; so delaying conditioning until after we
  generate the letter permutations vastly reduces computational expenses, and only applies that processing to the set of
  items most favorable candidate strings (based solely on physical probability). Through math and experimentation, it became
  obvious that the best route is to generate candidate strings based on physical input data, which nearly always puts the correct
  word in the top-20 or so strings (path). Then, apply n-gram analysis or other analyses to these strings to boost the correct strings;
  this is both more accurate, and also means that n-gram analysis is being applied to a subset of a few dozen items, instead of millions,
  billions, even trillions for somewhat long paths through the lattice. 
  The design principle uncovered is that, while stuffing everything into the graph is possible, and while clever search methods
  can be written to help reduce computation over the exponential search space, it is best to leave the graph as sparse as possible.
  Whatever can information or logic can be removed from the graph, or from the graph-search functionality, should be, because
  of the enormous performance gains. For now, I'll keep the graph/lattice structure the same, supportive of arc-weights and so on,
  in case they are useful in the future. Some Viterbi-like implementations even maintain lists of backpointers (instead of just one)
  as a method for generating n-best paths, which may be useful. Otherwise, simple/sparse is better for the graph.


*/

class LatticeBuilder{

public:

  KeyMap CLASS_keyMap; //put this into the outer class container for all these classes

  LatticeBuilder();
  ~LatticeBuilder();

  //these don't seem to belong in this class
  void BuildKeyMap(void);
  void BuildMapCoordinates(void);
  //char FindNearestKey(const Point& p);
  void SearchForNeighborKeys(const Point& p, vector<State>& neighbors);  //might be obsolete

  void ClearLattice(Lattice& lattice);
  void BuildStaticLattice(vector<PointMu>& inData, Lattice& lattice);
  double ReflexiveLikelihood(U16 ticks);
  void AppendCluster(PointMu& mu, Lattice& lattice);
  void BuildTransitionModel(Lattice& lattice);
  void TestBuildLattice(Lattice& lattice);
  void InitArcSizes(Lattice& lattice);
};



#endif








