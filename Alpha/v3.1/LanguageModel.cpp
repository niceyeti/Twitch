//#include "LanguageModel.hpp"
#include "Controller.hpp"


/*
  FYI: These models appears to eat up about 100MB

*/
LanguageModel::LanguageModel()
{
  /*
    TODO: build word n-gram models from COCA or other n-gram source data
  */
}

LanguageModel::~LanguageModel()
{
  unigramModel.clear();
  bigramModel.clear();
  trigramModel.clear();
  quadgramModel.clear();
  pentagramModel.clear();
}

//This may belong in the constructor, I just got tired of it building the models every run, when the LanguageModel wasn't underS test.
void LanguageModel::BuildModels(void)
{
  //charMap.resize(256);  //size if same as 26-char alphabet, but 256 to prevent any overflow
  cout << "Building character-gram language models..." << endl;
  //build the n-gram models. multiple classes use these, so they need to be placed in some higher class.
  cout << " > building unigram language models..." << endl;
  string s = "../unigrams.txt";
  BuildCharacterNgramModel(s);
  cout << " > building bigram language models..." << endl;
  s = "../bigrams.txt";
  BuildCharacterNgramModel(s);
  cout << " > building trigram language models..." << endl;
  s = "../trigrams.txt";
  BuildCharacterNgramModel(s);
  cout << " > building quadgram language models..." << endl;
  s = "../quadgrams.txt";
  BuildCharacterNgramModel(s);
  cout << " > building pentagram language models..." << endl;
  s = "../pentagrams.txt";
  BuildCharacterNgramModel(s);
  cout << "...complete." << endl;

}


/*
  Re-condition the outputs using character n-gram data, where each n-gram model is assigned some optimized weight.

  Uses a linear interpolation aross n-gram models: lambda1 * unigramProb(d) + lambda2 * bigramProb(d|c) + lambda3 * trigram...
  See Jurafsky SLP for details on linear interpolation in ngram models.

  TODO: determine optimal n-gram lambdas.
*/
void LanguageModel::ReconditionByCharGrams(LatticePaths& edits)
{
  int i;
  double ngram_val;

  for(LatticePathsIt it = edits.begin(); it != edits.end(); ++it){
    ngram_val = 0.0;
    //unigram conditioning
    for(i = 0; i < it->first.size(); i++){
      //cout << "here1" << endl;
      ngram_val += (CHAR_UNIGRAM_LAMBDA * GetUnigramProbability(it->first[i]));
    }
    //bigram conditioning
    for(i = 1; i < it->first.size(); i++){
      //cout << "here2" << endl;
      //cout << "adding " << (CHAR_BIGRAM_LAMBDA * GetBigramProbability(it->first[i],it->first[i+1])) << endl;
      ngram_val += (CHAR_BIGRAM_LAMBDA * GetBigramProbability(it->first[i-1],it->first[i]));
    }
    //trigram conditioning
    for(i = 2; i < it->first.size(); i++){
      //cout << "here3" << endl;
      ngram_val += (CHAR_TRIGRAM_LAMBDA * GetTrigramProbability(it->first[i-2],it->first[i-1],it->first[i]));
    }
    //quadgram conditioning
    for(i = 3; i < it->first.size(); i++){
      //cout << "here4" << endl;
      ngram_val += (CHAR_QUADGRAM_LAMBDA * GetQuadgramProbability(it->first[i-3],it->first[i-2],it->first[i-1],it->first[i]));
    }
    //pentagram conditioning
    for(i = 4; i < it->first.size(); i++){
      //cout << "here5" << endl;
      ngram_val += (CHAR_PENTAGRAM_LAMBDA * GetPentagramProbability(it->first[i-4],it->first[i-3],it->first[i-2],it->first[i-1],it->first[i]));
    }
    it->second += (ngram_val * CHAR_NGRAM_MODEL_WEIGHT);
    //cout << "here6..." << endl;
  }
}

/*
  Given the lattice paths have been conditioned and sorted (and possibly pruned),
  this finds the top most likely edits in the first k results. This is a novel method.
  Say you have the top ten result strings. This method looks at column 1 and finds
  the most likely char, which it sets as the first character in the output, and so on,
  until you have a word composed of the majority letter selected from each column for some
  k results.  For the same k, a list of results is obtained by backing off from the majority
  character, to taking the next-most-likely character from the column with the highest std-dev. 


  TODO: can k be factored out? Eg, can it be learned instead of static?

  In short: given a sorted list of pairs in the form (word,probability)<string,double>, and a parameter
  topN, look column by column across the topN words. For each column, get the majority char. Then,
  back off to the column with the highest st-dev (the highest variance, or intuitively, the messiest
  letter distribution, and thus the highest likelihood of a possible edit). Take the next-most-likely
  char from that column as a possible edit of the first string acquired. Repeat until we have
  k such outputs.
  The each edit in the output probably doesn't need an associated probability, just sort by the order
  in which the strings are built (which is inherently a decreasing probability).

  TODO: k is a confusing parameter. There must be a more formal way of solving this, maybe using LCS or matrices.
  Right now, setting k = 2, the simplest case. For k > 3, things get confusing. For k = 2, you just select the second
  most likely item from the highest variance column. Is there a sorting method to do this??? Is this just doing a fold()?
  This damn thing reeks of a problem that shouldn't be solved procedurally, but by modifying the data model 
  or by using an information theoretic approach.


  TODO: Could shove the count into the probability of the pair, but only if probabilities are obsolete after this function, which they
  seem to be.
  
  
  This could be much more advanced, such as incorporating the sub-scores of each voter, etc, other correlations.
  It also probably needs to be modified to skip words that it can recognize in a vocabulary; you don't want
  to edit real words into things that aren't!

  For a clever voting method, see http://www.geeksforgeeks.org/find-the-maximum-repeating-number-in-ok-time/ 
      
  Precondition: VERY IMPORTANT PRECONDITION. This assumes all input words are the same length; otherwise overflow is possible.

 
      // Returns maximum repeating element in arr[0..n-1].
      // The array elements are in range from 0 to k-1
      int maxRepeating(int* arr, int n, int k)
      {
          // Iterate though input array, for every element
          // arr[i], increment arr[arr[i]%k] by k
          for (int i = 0; i< n; i++)
              arr[arr[i]%k] += k;
       
          // Find index of the maximum repeating element
          int max = arr[0], result = 0;
          for (int i = 1; i < n; i++)
          {
              if (arr[i] > max)
              {
                  max = arr[i];
                  result = i;
              }
          }
       
          // Uncomment this code to get the original array back
          //   for (int i = 0; i< n; i++)
          //      arr[i] = arr[i]%k;
       
          // Return index of the maximum element
          return result;
      }
*/
void LanguageModel::MajorityVoteFilter(LatticePaths& paths, int topN, int k, vector<string>& output)
{
  int col, i, j, max, latticeWidth = paths.begin()->first.length();
  LatticePathsIt inner;
  char maxChar;
  string s;

  //TODO: get this to run faster. Its a bit of a puzzle.
  //im just using index-mapping for now, which cost an extra O(256) in space for the charmap

  //this is O(n^2), or specifically, O(k*n) where n is the length of the lattice (words)
  //iterate columns
  for(col = 0; col < latticeWidth; col++){
    for(i = 0; i < SMALL_BUFSIZE; i++){  //clears first 256 entries in char map on each column iteration. Thats 256*8 for an 8 char word!
      //((U32*)charMap)[i] = 0;
      charMap[i] = 0;
    }

    //for this column, summarize the counts to get the max element
    max = -1;
    for(j = 0, inner = paths.begin(); inner != paths.end() && j < topN; ++inner, j++){
      ++charMap[ (int)inner->first[col] ];
      cout << "val=" << (U32)inner->first[col] << endl;
      if(charMap[ (int)inner->first[col] ] > max){
        max = charMap[ (int)inner->first[col] ];
        maxChar = inner->first[col];
      }
    }
    cout << "max is: " << maxChar << endl;
    s += maxChar;
  }

  cout << "majority string=" << s << " of the top " << k << " results" << endl;
  output.push_back(s);

  //TODO: top-k, using std-dev
}


void LanguageModel::Process(LatticePaths& edits)
{
  cout << "lm.process()..." << endl;
  //TruncateResults(edits, 100);
  ReconditionByCharGrams(edits);

  //best candidates should fall in the first 20-30 edits, maybe even the top ten, if previous class's do their job well.
  //So eliminate candidates in the [100:end] range to reduce search complexity
  //TruncateResults(edits, 100);

  //TODO
  //SearchForEdits(edits, 2); //edit distance processing. second parameter is max edit-distances to search for.
  //ReconditionByWordGrams(list<string> usersPreviousInputs, edits);  
  cout << "sorting lm results..." << endl;
  edits.sort(ByLogProb);
}

/*
  Eliminates list items not in the top-k set of ranked items.

  Precondition: input list is sorted.

  TODO: make this more efficient.
*/
void LanguageModel::TruncateResults(LatticePaths& edits, int depth)
{
  int i;
  LatticePathsIt it;
  LatticePaths output;

  for(i = 0, it = edits.begin(); i < depth && it != edits.end(); ++it, i++){
    output.push_back(*it);
  }
  
  edits.clear();

  for(it = output.begin(); it != output.end(); ++it){
    edits.push_back(*it);
  }
}

/*
  Converts a key string for a char-based n-gram model
  to a key. This function is important to preserve mapping of keys,
  depending on different machine architecture.
  str="ABC" -> key= (A << 16) | (B << 8) | (C << 0)

  KeyStr must be null-terminated.  
*/
U32 LanguageModel::GetCharGramKey(char keyStr[])
{
  U32 key = 0;

/*
  for(int i = 0; keyStr[i] != '\0'; i++){
    key |= (keyStr[i] << (8 * i));
  }
*/

  switch(strnlen(keyStr,6)){
    case 1:
        key = (U32)keyStr[0];
      break;
    case 2:
        key = GetBigramKey(keyStr[0],keyStr[1]);
      break;
    case 3:
        key = GetTrigramKey(keyStr[0],keyStr[1],keyStr[2]);
      break;
    case 4:
        key = GetQuadgramKey(keyStr[0],keyStr[1],keyStr[2],keyStr[3]);
      break;
    case 5:
        key = GetPentagramKey(keyStr[0],keyStr[1],keyStr[2],keyStr[3],keyStr[4]);
      break;
    default:
      cout << "ERRROR no key found for keystr >" << keyStr << endl;
  }

  return key;
}



/*
  Builds with -log2-space probabilities, as will all other functions.

  Expect input file has already been converted to -log2-space values.

  Note this doesn't need an n-gram parameter; the function instead determines this by the length
  of the grams in the file, so you can pass it any filename and it will build the appropriate model.
  
  Its important to understand the key format. Since its U32, it could theoretically only support up to a
  four-gram model, which isn't necessary. Also its building U32 keys from signed char values, which could
  become problematic if not handled right.

  When using conditining data, defined this application specifically: applying trigram data (or any n-gram data for n > 2)
  breaks the dynamic programming model of the lattice. See Jurafsky SLP Ch 10.


  ****WARNING: The n-gram files must be built by python first. Since probabilities are conditional, you re supposed to
  store the prefix of n-1 chars (the key) and map this to a tuple of (nextletter,conditional probability of nextLetter|prefix).
  I am instead preparing the files in advance with all conditional values predefined. Just know its an assumption; don't do processing
  on the n-gram files. Just assume the file values are all already given, and in -log2-space.

*/
void LanguageModel::BuildCharacterNgramModel(const string& ngramFile)
{
  int ntoks, n;
  U32 key;
  char buf[BUFSIZE];
  char* tokens[8] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
  //TODO: hardcode the bigram file path for now
  fstream infile(ngramFile.c_str(), ios::in);
  string delims = "\t";

  if(!infile){
    cout << "ERROR could not open file: " << ngramFile << endl;
    return;
  }

  while(infile.getline(buf,BUFSIZE)){  // same as: while (getline( myfile, line ).good())
    StrToUpper(buf);
    ntoks = Tokenize(tokens,buf,delims);
    if(ntoks == 2){

      //map the ngram model by the length of the tokens
      n = strnlen(tokens[0],16);

      //build the key
      if(n >= 1 && n <= 5){
        key = GetCharGramKey(tokens[0]);
      }
      else{
        cout << "ERROR string of length " << n << " not recognized in languagemodel.buildcharacterngrammodel()" << endl;
      }

      switch(n){

        //build unigram model: first letter as U32 forms the key
        case 1: 
            //key = (U32)tokens[0][0];
            //cout << "entering tokens[0] >" << tokens[0] << "< with log-probability=" << tokens[1] << "  (verify correctness)" << endl;
            unigramModel[key] = atof(tokens[1]);     
          break;
 
        //build bigram model: first two letters OR'ed together into a U32 key form the key
        case 2:
            //key = ((U32)tokens[0][1] << 8) | (U32)tokens[0][0];
            //cout << "entering tokens[0] >" << tokens[0] << "< with log-probability=" << tokens[1] << "  (verify correctness)" << endl;
            bigramModel[key] = atof(tokens[1]);
            //cout << "key/val: " << key << "|" << bigramModel[key] << endl;
          break;

        //build trigram model: first three letters OR'ed together into a U32 key form the key
        case 3: 
            //key = ((U32)tokens[0][2] << 16) | ((U32)tokens[0][1] << 8) | (U32)tokens[0][0];
            //cout << "entering tokens[0] >" << tokens[0] << "< with log-probability=" << tokens[1] << "  (verify correctness)" << endl;
            trigramModel[key] = atof(tokens[1]);
          break;

        //build quadgram model: first four letters OR'ed together into a U32 key form the absolute key
        case 4: 
            //key = ((U32)tokens[0][3] << 24) | ((U32)tokens[0][2] << 16) | ((U32)tokens[0][1] << 8) | (U32)tokens[0][0];
            //cout << "entering tokens[0] >" << tokens[0] << "< with log-probability=" << tokens[1] << "  (verify correctness)" << endl;
            quadgramModel[key] = atof(tokens[1]);
          break;

        //build pentagram model: first five letters OR'ed together into a U32 key form a COMPRESSED key
        case 5: 
            //key = GetPentagramKey(tokens[0][4], tokens[0][3], tokens[0][2], tokens[0][1], tokens[0][0]);
            //cout << "entering tokens[0] >" << tokens[0] << "< with log-probability=" << tokens[1] << "  (verify correctness)" << endl;
            pentagramModel[key] = atof(tokens[1]);
          break;

        //error catchall
        default:
            cout << "WARNING incorrect strlen in tokens found in n-gram file. Sample gram: " << tokens[0] << " value=" << tokens[1] << endl;
          break;
      }
    }
    else{
      cout << "WARNING incorrect number of tokens found in n-gram file: " << ngramFile << endl;
    }
  }

  infile.close();
}
//return probability of a
double LanguageModel::GetUnigramProbability(char a)
{
  CharGramIt it = unigramModel.find((U32)a);

  if(it != unigramModel.end()){
    return it->second;
  }
  else{
    //cout << "WARNING alpha >" << a << "< not found in unigram model (upper/loower case issue?)" << endl;
    return DEFAULT_LOG_PROB;
  }
  //return unigramModel[ (U32)a ];  //returns zero if these elements are new to the model, which they never should be
}
//  Lookup a bigram probability: probability of b, given a.
//  Assume caller is responsible for validating bigram model (exists/not-empty, all possible a and b in model, etc).
double LanguageModel::GetBigramProbability(char a, char b)
{
  CharGramIt it = bigramModel.find( GetBigramKey(a,b) );

  if(it != bigramModel.end()){
    return it->second;
  }
  else{
    //cout << "WARNING alphas >" << a << b << "< not found in bigram model (upper/lower case issue?)" << endl;
    return DEFAULT_LOG_PROB;
  }
//  return bigramModel[((U32)b << 8) | (U32)a];  //returns zero if these elements are new to the model, which they never should be
}
//Returns probability of c, given sequence ab.
double LanguageModel::GetTrigramProbability(char a, char b, char c)
{
  CharGramIt it = trigramModel.find( GetTrigramKey(a,b,c) );

  if(it != trigramModel.end()){
    return it->second;
  }
  else{
    //cout << "WARNING alphas >" << a  << b << c << "< not found in trigram model (upper/lower case issue?)" << endl;
    return DEFAULT_LOG_PROB;
  }
}

//return probability of d, given abc
double LanguageModel::GetQuadgramProbability(char a, char b, char c, char d)
{
  CharGramIt it = quadgramModel.find( GetQuadgramKey(a,b,c,d) );  //returns zero if these elements are new to the model, which they never should be

  if(it != quadgramModel.end()){
    return it->second;
  }
  else{
    //cout << "WARNING alphas >" << a << b << c << d << "< not found in quadgram model (upper/lower case issue?)" << endl;
    return DEFAULT_LOG_PROB * 1.5;
  }
}

//return probability of e, given abcd. Pentagram model shoves five characters into a 32-bit key,
//which requires a compression scheme of converting a char to 6-bit encoding. This is handled by CompressedChar().
double LanguageModel::GetPentagramProbability(char a, char b, char c, char d, char e)
{
  CharGramIt it = pentagramModel.find( GetPentagramKey(a,b,c,d,e) );

  if(it != pentagramModel.end()){
    return it->second;
  }
  else{
    //cout << "WARNING alphas >" << a << b << c << d << e << "< not found in pentagram model (upper/lower case issue?)" << endl;
    return DEFAULT_LOG_PROB * 1.5;
  }
}

U32 LanguageModel::GetBigramKey(char a, char b)
{
  return ((U32)a << 8) | (U32)b;
}
U32 LanguageModel::GetTrigramKey(char a, char b, char c)
{
  return ((U32)a << 16) | ((U32)b << 8) | (U32)c;
}
U32 LanguageModel::GetQuadgramKey(char a, char b, char c, char d)
{
  return ((U32)a << 24) | ((U32)b << 16) | ((U32)c << 8) | (U32)d;
}
U32 LanguageModel::GetPentagramKey(char a, char b, char c, char d, char e)
{
  return ((CompressedChar(a) << 24) | (CompressedChar(b) << 18) | (CompressedChar(c) << 12) | (CompressedChar(d) << 6) | CompressedChar(e));
}

/*
  returns only the least 6 bits of char. Ascii alpha range is 65 - 122. Subtracting 65 to baseline 'A' means we need enough bits to cover
  57 integers, which can be squeezed into a 6-bit number, since 2^6 = 64.

  TODO: Improve this function using some other compression scheme, or just assume go 64 bit if 32 bit is extremely unlikely.
  But what we do depends on the semantic constraints we're willing to apply to "fastmode", such as constricting its range
  to only alphas characters, or even only 26 lowercase chars.
  Look at it this way. We have a range of chars we need to encode. The ASCII chars range from 32-122, modestly. That 122 - 32 == 90
  integers encodings for these 90 unique items. For a five char key, then, we need a bit length sufficient to hold the number 90^5,
  or the number 5904900000 (5 billion), which is over the size of 2^32 (4.2 billion).
  On the other hand, if we go with only lowercase alphas, the range is 26^5, or only about 12 million, which fits easily in U32.

*/
U32 LanguageModel::CompressedChar(char c)
{
  return (U32)c - (U32)'A';
}

