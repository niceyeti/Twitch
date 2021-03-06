#include "conditioner.hpp"

LatticeBuilder::LatticeBuilder()
{

  BuildBigramModel();
  //buildUnigramModel();
  //BuildTrigramMOdel();


  /*
  Initial state requirements: allocate lattice to sufficient size to be able to handle max possible cluster size and
  max likely clusters.  Of these, initialize all members to 0, but the first item in each cluster, init it to 65535 or
  other signal value.

  As we append new clusters to the lattice, the next column will by default have all zero entries except for the first
  item identified as 65535 (or other signal) and all transition probabilities set to 1.0. Or some similar scheme...
  */

  //TODO: resolve the memory reservation strategy once data model settles

  //build the lattice columns
  lattice.reserve(MAX_COLS);
  lattice.resize(MAX_COLS);
  for(int i = 0; i < lattice.size(); i++){
    lattice.alphas.resize(MAX_CLUSTER_ALPHAS);  //resize each column to accomodate the max number of letters in a cluster
    for(int j = 0 ; j < MAX_CLUSTER_ALPHAS; j++){
      lattice.alphas[j].arcs.reserve(MAX_CLUSTER_ALPHAS);
    }
  }

  //init the lattice to all zero entries
  for(int i = 0; i < lattice.size(); i++){
    lattice.pReflexive = 0.0;  
    for(int j = 0; j < lattice.alphas.size(); j++){  //init all the alphas to zero
      lattice.alphas[j].pState = 0.0;
      lattice.alphas[j].symbol = (char)0;
    }
  }



}

LatticeBuilder::~LatticeBuilder()
{
  //this is done by the language specification. stl data structure destructors are called recursively
}

/*
  Lookup a bigram probability.

  Assume caller is responsible for validating bigram model (exists/not-empty, all possible a and b in model, etc).
*/
LatticeBuilder::GetBigramProbability(char a, char b)
{
  return BigramModel[(U32)a << 8 | (U32)b];  //returns zero if these elements are new to the model, which they never should be
}

LatticeBuilder::BuildBigramModel(const string& bigramFile)
{
  int ntoks;
  U32 key;
  char buf[BUFSIZE];
  char* tokens[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
  //TODO: hardcode the bigram file path for now
  fstream infile(bigramFile.c_str(), ios::in);
  string delims = "\t";

  if(!infile){
    cout << "ERROR could not open file: " << trainingFile << endl;
    return;
  }

  while(infile.getline(buf,BUFSIZE)){  // same as: while (getline( myfile, line ).good())
    ntoks = tokenize(tokens,buf,delims);

    if(ntoks == 2){
      if(strnlen(tokens[0],16) == 2){
        key = (U32)buf[0] << 8 | (U32)buf[1];  //first two letters OR'ed together into a U32 key form the key
        cout << "entering tokens[0] >" << tokens[0] << "< with probability=" << tokens[1] << endl;
        bigramModel[key] = atof(tokens[1]);
      }
      else{
        cout << "WARNING incorrect strlen in tokens found in n-gram file. Sample gram: " << tokens[0] << " value=" << tokens[1] << endl;
      }
    }
    else{
      cout << "WARNING incorrect number of tokens found in n-gram file: " << bigramFile << endl;
    }

  }

}


/*
  This needs to be manually defined. It would be nice to at least have it read values
  from a file, instead of needing to be recompiled. Nicer still, to define it with ratios
  instead of units... hm... different screens may yield different coordinates as well, screwing
  up the whole mapping...
  Screens, resolutions, and various keyboard layouts are the parameters. I'm basing this off a typical
  DELL qwerty keyboard right now.

  TODO: Put this at outermost class level. Doesn't belong here.
  
  TODO: Its important to remember that the set of neighbor's defines the possible letters in the output; so if
  a letter is omitted to eagerly, it may not be possible to correct except with edit-distance logic. Clients
  of the data structure probably want tight-clusters, though, so this error may be tolerable since later
  stages should catch it.

  Ignore punctuation for now.

  */
void LatticeBuilder::BuildKeyMap(void)
{
  pair<Point,vector<char> > p;

  if(!CLASS_keyMap.empty()){
    CLASS_keyMap.clear();
  }

  BuildMapCoordinates();

  //q: keyboard neighbors of the letter q
  CLASS_keyMap['q'].second.push_back('w');
  CLASS_keyMap['q'].second.push_back('a');
  p.second.clear();
  //w
  CLASS_keyMap['w'].second.push_back('q');
  CLASS_keyMap['w'].second.push_back('a');
  CLASS_keyMap['w'].second.push_back('s');
  CLASS_keyMap['w'].second.push_back('e');
  p.second.clear();
  //e
  CLASS_keyMap['e'].second.push_back('r');
  CLASS_keyMap['e'].second.push_back('d');
  CLASS_keyMap['e'].second.push_back('s');
  CLASS_keyMap['e'].second.push_back('w');
  p.second.clear();
  //r
  CLASS_keyMap['r'].second.push_back('t');
  CLASS_keyMap['r'].second.push_back('f');
  CLASS_keyMap['r'].second.push_back('d');
  CLASS_keyMap['r'].second.push_back('e');
  p.second.clear();

  //t
  CLASS_keyMap['t'].second.push_back('y');
  CLASS_keyMap['t'].second.push_back('g');
  CLASS_keyMap['t'].second.push_back('f');
  CLASS_keyMap['t'].second.push_back('r');
  p.second.clear();

  //y
  CLASS_keyMap['y'].second.push_back('u');
  CLASS_keyMap['y'].second.push_back('h');
  CLASS_keyMap['y'].second.push_back('g');
  CLASS_keyMap['y'].second.push_back('t');
  p.second.clear();

    //u
  CLASS_keyMap['u'].second.push_back('i');
  CLASS_keyMap['u'].second.push_back('j');
  CLASS_keyMap['u'].second.push_back('h');
  CLASS_keyMap['u'].second.push_back('y');
  p.second.clear();

    //i
  CLASS_keyMap['i'].second.push_back('o');
  CLASS_keyMap['i'].second.push_back('k');
  CLASS_keyMap['i'].second.push_back('j');
  CLASS_keyMap['i'].second.push_back('u');
  p.second.clear();

    //o
  CLASS_keyMap['o'].second.push_back('p');
  CLASS_keyMap['o'].second.push_back('l');
  CLASS_keyMap['o'].second.push_back('k');
  CLASS_keyMap['o'].second.push_back('i');
  p.second.clear();

    //p
  CLASS_keyMap['p'].second.push_back('l');
  CLASS_keyMap['p'].second.push_back('o');
  p.second.clear();

    //a
  CLASS_keyMap['a'].second.push_back('q');
  CLASS_keyMap['a'].second.push_back('w');
  CLASS_keyMap['a'].second.push_back('s');
  CLASS_keyMap['a'].second.push_back('z');
  p.second.clear();

    //s
  CLASS_keyMap['s'].second.push_back('a');
  CLASS_keyMap['s'].second.push_back('w');
  CLASS_keyMap['s'].second.push_back('e');
  CLASS_keyMap['s'].second.push_back('d');
  CLASS_keyMap['s'].second.push_back('x');
  CLASS_keyMap['s'].second.push_back('z');
  p.second.clear();

    //d
  CLASS_keyMap['d'].second.push_back('s');
  CLASS_keyMap['d'].second.push_back('e');
  CLASS_keyMap['d'].second.push_back('r');
  CLASS_keyMap['d'].second.push_back('f');
  CLASS_keyMap['d'].second.push_back('c');
  CLASS_keyMap['d'].second.push_back('x');
  p.second.clear();

    //f
  CLASS_keyMap['f'].second.push_back('d');
  CLASS_keyMap['f'].second.push_back('r');
  CLASS_keyMap['f'].second.push_back('t');
  CLASS_keyMap['f'].second.push_back('g');
  CLASS_keyMap['f'].second.push_back('v');
  CLASS_keyMap['f'].second.push_back('c');
  p.second.clear();

    //g
  CLASS_keyMap['g'].second.push_back('f');
  CLASS_keyMap['g'].second.push_back('t');
  CLASS_keyMap['g'].second.push_back('y');
  CLASS_keyMap['g'].second.push_back('h');
  CLASS_keyMap['g'].second.push_back('b');
  CLASS_keyMap['g'].second.push_back('v');
  p.second.clear();


    //h
  CLASS_keyMap['h'].second.push_back('g');
  CLASS_keyMap['h'].second.push_back('y');
  CLASS_keyMap['h'].second.push_back('u');
  CLASS_keyMap['h'].second.push_back('j');
  CLASS_keyMap['h'].second.push_back('n');
  CLASS_keyMap['h'].second.push_back('b');
  p.second.clear();


    //j
  CLASS_keyMap['j'].second.push_back('h');
  CLASS_keyMap['j'].second.push_back('u');
  CLASS_keyMap['j'].second.push_back('i');
  CLASS_keyMap['j'].second.push_back('k');
  CLASS_keyMap['j'].second.push_back('m');
  CLASS_keyMap['j'].second.push_back('n');
  p.second.clear();


    //k
  CLASS_keyMap['k'].second.push_back('j');
  CLASS_keyMap['k'].second.push_back('i');
  CLASS_keyMap['k'].second.push_back('o');
  CLASS_keyMap['k'].second.push_back('l');
  CLASS_keyMap['k'].second.push_back(',');
  CLASS_keyMap['k'].second.push_back('m');
  p.second.clear();


    //l
  CLASS_keyMap['l'].second.push_back('k');
  CLASS_keyMap['l'].second.push_back('o');
  CLASS_keyMap['l'].second.push_back('p');
  CLASS_keyMap['l'].second.push_back(',');
  CLASS_keyMap['l'].second.push_back('.');
  p.second.clear();


    //z
  CLASS_keyMap['z'].second.push_back('a');
  CLASS_keyMap['z'].second.push_back('s');
  CLASS_keyMap['z'].second.push_back('x');
  p.second.clear();


    //x
  CLASS_keyMap['x'].second.push_back('z');
  CLASS_keyMap['x'].second.push_back('s');
  CLASS_keyMap['x'].second.push_back('d');
  CLASS_keyMap['x'].second.push_back('c');
  p.second.clear();


    //c
  CLASS_keyMap['c'].second.push_back('x');
  CLASS_keyMap['c'].second.push_back('d');
  CLASS_keyMap['c'].second.push_back('f');
  CLASS_keyMap['c'].second.push_back('v');
  p.second.clear();


    //v
  CLASS_keyMap['v'].second.push_back('c');
  CLASS_keyMap['v'].second.push_back('f');
  CLASS_keyMap['v'].second.push_back('g');
  CLASS_keyMap['v'].second.push_back('b');
  p.second.clear();


    //b
  CLASS_keyMap['b'].second.push_back('v');
  CLASS_keyMap['b'].second.push_back('g');
  CLASS_keyMap['b'].second.push_back('h');
  CLASS_keyMap['b'].second.push_back('n');
  p.second.clear();

    //n
  CLASS_keyMap['n'].second.push_back('b');
  CLASS_keyMap['n'].second.push_back('h');
  CLASS_keyMap['n'].second.push_back('j');
  CLASS_keyMap['n'].second.push_back('m');
  p.second.clear();

    //m
  CLASS_keyMap['m'].second.push_back('n');
  CLASS_keyMap['m'].second.push_back('j');
  CLASS_keyMap['m'].second.push_back('k');
  CLASS_keyMap['m'].second.push_back(',');
  p.second.clear();

    //,
  CLASS_keyMap[','].second.push_back('m');
  CLASS_keyMap[','].second.push_back('k');
  CLASS_keyMap[','].second.push_back('l');
  CLASS_keyMap[','].second.push_back('.');
  p.second.clear();

    //.
  CLASS_keyMap['.'].second.push_back(',');
  CLASS_keyMap['.'].second.push_back('l');
  p.second.clear();
}

/*
  TODO: This doesn't belong here, belongs in metaclass.

  Consumes some static data (file, etc) containing key coordinate
  info, and fill map with that data
*/
void LatticeBuilder::BuildMapCoordinates(void)
{
  cout << "TODO: BuildMapCoordinates" << endl;
}
/*
  Iterate over the map looking for key nearest to some point p.
  
  TODO: This could be a lot faster than linear search. It will be called for every mean from
  the SingularityBuilder, so eliminating it might speed things a bit.
*/
char LatticeBuilder::FindNearestKey(const Point& p)
{
  char c = '\0';
  double min = 99999;

  //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
  for(KeyMapIt it = CLASS_keyMap.begin(); it != CLASS_keyMap.end(); ++it){
    if(min > Point.DoubleDistance(it->second.first, p)){
      c = it->first;
    }
  }

  return c;
}


/*
  Given some euclidean point, map it to a collection of neighbor keyboard keys w/in
  some search radius, such that any given point yields up to seven keys.

*/
void LatticeBuilder::SearchForNeighborKeys(const Point& p, vector<State>& neighbors)
{
  double logProbability;
  double sumDist = 0.0;  //normalization constant for converting distances to a radial probability
  char c = FindNearestKey(p);

  for(int i = 0; i < CLASS_keyMap[c].second.size(); i++){
    State s;
    s.symbol = CLASS_keyMap[c].second[i];
    s.pState = Point.DoubleDistance(p,CLASS_keyMap[c].first);
    sumDist += s.pState;
    neighbors.push_back(s);
  }

  for(int i = 0; i < neighbors.size(); i++){
    neighbors[i].pState /= sumDist;
    neighbors[i].pState = -1.0 * log2l(neighbors[i].pState);  //convert probability to -log2-space
  }
}

/*
  The primary responsibility of this class. Build the lattice, and assign
  conditional probabilities based on physical distances.

  This is the static, non-streaming version of lattice-building.

  Secondary responsibility is re-conditioning the probabilities based on character n-gram probabilities.
*/
void LatticeBuilder::BuildStaticLattice(vector<PointMu>& inData, Lattice& lattice)
{
  cout << "in BuildStaticLattice(), inDate.size()=" << inData.size() << endl;

  //for each point in inData, map it to a collection of immediate neighbors (keys), each with a probability with respect to its distance from the point
  for(int i = 0; i < inData.size(); i++){
   
    AppendCluster(inData[i]);
    //these types are wrong. this should be some graph/lattice type
    SearchForNeighborKeys(inData[i],neighbors);  //gets all the neighbors and assigns physical probabilities to each one
    lattice.push_back(neighbors);
  }
}

/*
  Given some raw 'tick' value of the number of ticks spent in a given cluster,
  map this to a likelihood for a reflexive transition in that state. For instance,
  if the user focuses on 's' for longer, we would like to know if that indicates 'ss' as input,
  (or possibly some close neighbors of 's', whose transition can't be detected with current system resolution)

  Returns: a real number indicating the likelihood of a reflexive transition. When modelling this numerically, think of 
  the values with which this one is competing: the forward edges of the lattice. This parameter must be able to compete
  with these values (in terms of magnitude), if it is to have any use.


  TODO: define this more function's behavior once testing begins. It is a precision parameter.

*/
double ReflexiveLikelihood(U16 ticks)
{
  //for now, this is a simple threshold function: exceed threshold, and trigger. This is a discrete strategy,
  //but one that is likely to work with little calibration.
  if(ticks >= REFLEXIVE_TICK_THRESHOLD){
    return 0.5;
  }
  
  return 0.0;
}


/*
  Given an input (mu, a mean point), append a new cluster.
  Could search for nearest neighbor points in real-space. Currently,
  I think it is sufficient to identify the nearest key, and grab its neighbors based
  on keyboard layout.  

  Input: a point mean, representing a spatial mean point along with the estimate time spent their (which gives the likelihood
  a reflexive arc occurred for this cluster).

*/
void LatticeBuilder::AppendCluster(PointMu& mu)
{
  int i, j, k, prevCol;

  //get nearest neighbor keys of point, via pre-defined static key-clusters
  //TODO: evaluate if static clusters should be used, or continuous value ones. For instance, should 's' alwasy return {aqwedxz} regardless of the focus on 's' key?
  char index = getNearestKey(mu.first);
  //double pReflexiveArc = ReflexiveLikelihood(mu.ticks);
  Cluster newCluster(CLASS_keyMap[index], ReflexiveLikelihood(mu.ticks));

  //Each time a cluster is appended, resize the arc vectors in the previous cluster each to the number of states in this cluster.
  //But only do so if there is a previous cluster (eg, this is not the start state).
  if(lattice.size() > 0){
    prevCol = lattice.size() - 1; //get the previous column in the lattice, which is still currently just the last one
    for(i = 0; i < lattice[prevCol].alphas.size(); i++){
      for(j = 0; j < newCluster.alphas.size(); j++){
        Arc newArc = {&newCluster.alphas[j], ZERO_LOG_PROB};
        lattice[prevCol].alphas[i].arcs.push_back(newArc);
      }
      //lattice[prevCol].alphas[i].arcs.resize(newCluster.alphas.size()); //resize each prev arc vector to num alphas in new cluster
    }
  }
  else{
    //for the first added column, init the back pointers to null, the maxProb value to a negative flag val
    for(i = 0; i < newCluster.alphas.size(); i++){
      newCluster.alphas[i].maxPrev = NULL;
      newCluster.alphas[i].maxPrev = INIT_STATE_LOG_PROB;
    }
  }

  lattice.push_back(newCluster); //i kinda prefer push_back (dynamic addition), as opposed to static=
}

/* 
  Once---or possibly in streaming fashion, once a new cluster has been appended--

  Try to write this to encompass both states of the lattice: streaming and static lattice models.

  Obviously each arc is composed of a source and a dest, both of which are alphas (letters/keys). Each alpha
  has a unique index, so each src/dest is an alpha/index pair of type char/int
  

*/
void LatticeBuilder::BuildTransitionModel(Lattice& lattice)
{
  int i, j, k , l;

  /*
    Starting at the 0th index in lattice, the arcs denote the transition from current alpha to some alpha in next column of matrix
    The last column in the matrix is the end of the word, so there is no next column to which to assign arcs. The <stop> state is indicated
    when 
  */

  //iterate up to n-1 columns of the lattice, assigning arcs for every alpha to every alpha in next column. Therefore if columns are n, there will be n^2 arcs.
  for(i = 0 ; i < lattice.size - 1; i++){
  
    //foreach alpha in current column, assign each of its arcs an edge weight given by an n-gram probability (beyond bigrams, this will require a lot of probability modeling)
    for(j = 0; j < lattice[i].alphas.size(); j++){
      for(k = 0; k < lattice[i].alphas[j].arcs.size(); k++){
        for(l = 0; l < lattice[i + 1].alphas.size()){
          lattice[i].alphas[j].arcs[k].dest = &lattice[i + 1].alphas[l];
          if(USE_NGRAM_DATA){
            lattice[i].alphas[j].arcs[k].pArc = GetBigramProbability(lattice[i].alphas[j].symbol, lattice[i + 1].alphas[l].symbol); //each arc's probability is given by p(B|A) for path from state A to state B
          }
          else{
            lattice[i].alphas[j].arcs[k].pArc = 1.0; //effectively arc weight becomes a dont-care if set to 1.0
          }
        }
      }
    }
  }
  
}




/*

  Re-condition arcs based on n-gram probabilities (likely bi-gram data will be sufficient)

void LatticeBuilder::ConditionArcs(Lattice& lattice)
{
  //let p(arc_physical) be the physically-assigned probability based solely on location in relation to neighboring keys

  //for arc in lattice:
  // p(arc) = p(arc_physical|alpha,beta)
  // giving:
  // p(arc) = p(arc_physical)*p(alpha|beta)
}
*/




