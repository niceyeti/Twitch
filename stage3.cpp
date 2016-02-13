#include "header.h"

/*
  The is the beginning of the second-stage filtering: the Viterbi algorithm;
  Specifically stage 3 in my design diagrams.

  Pre-condition: Signal filtering in stages 1 and 2 is complete. The output of the
  previous stages is a set of vectors describing likely state paths (a set of edges)
  each of which has an associated probability.

  Corollary: For now, assume static state sets, meaning the previous stages have successfully
  created a set of edges like this:
     (a + b)(c + d)(e + f)
  Which, with probabilities, will look like this:
  (<a,0.8> + <b,0.2>)(<c + 0.5> + <d,0.5>) ...

  But assume no missing or extra letters, for now. The edit-distance module after this
  one will help remedy extra/missing letters. Assume the simplest case of a static regular
  expression with no "*" operators.

*/

//class Viterbi : PipeFilter ??
class Viterbi{
  Viterbi();
  ~Viterbi();
  Filter(TransitionModel& model);
  TransitionMatrix transitionModel;






};





int main(void)
{
  Stage3 Viterbi;


}





















