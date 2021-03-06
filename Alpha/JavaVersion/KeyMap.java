import java.io.*;
import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;

//TODO: this is a temporary abstraction. Use the Team Force data structure, 
public class KeyMap
{
  Map<Character,Key> _keyMap;
  double _minInterKeyRadius; // defined as one-half the distance between the two nearest alpha keys
  double _leftExtreme, _rightExtreme, _upperExtreme, _lowerExtreme; //defines the ui boundaries for alpha chars

  //should not use the default ctor; only use the one that inits the map based on a file of key coordinates
  public KeyMap(){
    _keyMap = new HashMap<Character,Key>();
    _minInterKeyRadius = 0;
    _leftExtreme = 0;
    _rightExtreme = 0;
    _lowerExtreme = 0;
    _upperExtreme = 0;
  }

  public KeyMap(String keyCoordinateFileName){
    _keyMap = new HashMap<Character,Key>();
    initKeyMap(keyCoordinateFileName);
  }

  public double GetMinInterKeyRadius(){
    return _minInterKeyRadius;
  }

  //util, lookup a Point (value) given char c (a map key)
  //TODO: error checking on null returns, if c is not in map keys      
  public Point GetPoint(char key){
    Point ret = null;

    if(_keyMap.containsKey(key)){
      ret = _keyMap.get(key).GetPoint();
      if(ret == null){
        System.out.println("ERROR key >"+key+"< found in map, with null Point reference");
        ret = _keyMap.get('G').GetPoint();  //return something we know is in the map
      }
    }
    else{
      System.out.println("ERROR key >"+key+"< not found in map!");
    }
    return ret;
  }

  /*
    Iterate over the map looking for key nearest to some point p.
    Note this returns the nearest ALPHA key, not the nearest key among all the keys.

    TODO: This could be a lot faster than linear search. It will be called for every mean from
    the SingularityBuilder, so eliminating it might speed things a bit.  Could at least return 
    as soon as we find a min-distance that is less than the key width (or width/2), such that
    no other key could be closer than this value. Something like that...
  */
  public char FindNearestAlphaKey(Point pt)
  {
    char c = 'G';
    double dist, min;

    //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
    min = 99999.0;
    for(Key key : _keyMap.values()){
      if(Utilities.IsAlpha(key.GetId())){
        dist = Point.DoubleDistance(key.GetPoint(),pt);
        if(min > dist){
          min = dist;
          if(min < _minInterKeyRadius){  //optimization. return when dist is below some threshold, such that no other key could be nearer
            return key.GetId();
          }
          c = key.GetId();
        }
      }
    }

    return c;
  }

  //TODO: all of the key map initialization and update (eg, if the user resizes the screen or moves the client window) needs to be done according to the Team Gleason project
  public void initKeyMap(String keyCoordinateFileName)
  {
    //the calling order of these inits matters, since the first inits all of the entries, second fills the location data of the entries, etc
    initKeyNeighbors();
    initKeyCoordinates(keyCoordinateFileName);
    initDerivedGeometryVals(); //initializes various geometric properties (_minInterKeyRadius, etc) used in some of the inference logic
  }

  /*
    FastMode needs a few geometric values such as the minimum distance between keys, and the layout boundaries.
    The layout boundaries are used to determine if a sensor value (an X/Y pair) is omitted, and min-key distance
    can be used in various ways for logic questions and for generating/smoothing probability distributions. Like maybe
    you have a raw trigger point (x,y), and a list of its nearest neighbor keys, each with its own (x,y) center. A probability
    for the "intended key" can be based in various ways upon whether or not the trigger point is within the _minInterKeyRadius distance.
    Or say you look up the neighbors of the nearest key, and want to use the minInterKey distance to omit neighbor keys that are greater
    than some scaled _minInterKeyRadius, such that somewhat far neighbor keys aren't evaluated or are assigned a rapidly diminishing probability.
    A few basic geometry values can be used in this way to scale probabilities in an intuitive way, and to assign precise values when answering
    questions like, given some trigger point and its nearest neighbors, determine the most likely "intended" key press. And other such
    "regional" queries for the interface.

    NOTE: the values are only based in alpha characters, since only these drive the fastmode inference process.
  */
  public void initDerivedGeometryVals()
  {
    int curX, curY;
    String alphaStr = new String("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

     if(_keyMap.isEmpty()){
       System.out.println("ERROR key map not yet created");
     }

     //order of these calls matters
     initMinInterKeyRadius();
     initKeyBoundaries();
  }

  //sets the minimum interkey distance, defined as one half the center-to-center distance between the nearest two keys
  private void initMinInterKeyRadius()
  {
    double dist;

    _minInterKeyRadius = 999999;
    //find the minimum interkey distance, the dist between the centers of the two nearest keys.
    for(Key outerKey : _keyMap.values()){
      for(Key innerKey : _keyMap.values()){
        if(Utilities.IsAlpha(outerKey.GetId()) && Utilities.IsAlpha(innerKey.GetId())){
          if(outerKey.GetId() != innerKey.GetId()){ //dont compare keys with themselves!
            dist = Point.DoubleDistance(outerKey.GetPoint(),innerKey.GetPoint()) / 2.0;
            if(dist < _minInterKeyRadius){
              _minInterKeyRadius = dist;
            }
          }
        }
      }
    }  //post-loop: _minInterKeyRadius holds one half the distance between the two nearst alpha keys
  }

  //Utility for converting a word (sequence of chars) to a point sequence.
  public ArrayList<Point> WordToPointSequence(String word)
  {
    ArrayList<Point> wordPoints = new ArrayList<Point>();
    for(int i = 0; i < word.length(); i++){
      Point pt = this.GetPoint(word.charAt(i));
      wordPoints.add(pt);
    }

    return wordPoints;
  }

  //calculates the left, right, upper, and lower-most boundaries based on the alpha characters in the layout
  private void initKeyBoundaries()
  {
    int curX, curY;

    //to add as much confusion as possible, the upper left window corner is defined as (0,0), so "_upperExtreme" will be less than _lowerExtreme
    _leftExtreme = 99999; _rightExtreme = 0; _upperExtreme = 99999; _lowerExtreme = 0;
    for(Key kbKey : _keyMap.values()){
      if(Utilities.IsAlpha(kbKey.GetId())){ //verify its an alpha
        curX = kbKey.GetPoint().GetX();
        curY = kbKey.GetPoint().GetY();

        if(_leftExtreme > curX){
          _leftExtreme = curX;
        }
        if(_rightExtreme < curX){
          _rightExtreme = curX;
        }
        if(_upperExtreme > curY){
          _upperExtreme = curY;
        }
        if(_lowerExtreme < curY){
          _lowerExtreme = curY;
        }
      }
    }

    //We just found the most extreme left/right/lower/upper values among the centers of the keys,
    //but its the boundaries we're after. So add the _minInterKeyRadius to each to expand the boundaries a little.
    _lowerExtreme += _minInterKeyRadius;
    _upperExtreme -= _minInterKeyRadius;
    _rightExtreme += _minInterKeyRadius;
    _leftExtreme -= _minInterKeyRadius;

    //check if we've underflowed the boundaries of the client window, and adjust
    if(_upperExtreme < 0){
      _upperExtreme = 0;
    }
    if(_leftExtreme < 0){
      _leftExtreme = 0;
    }
    //should check the right and lower, but there is no equivalent way to see if they extend too far.
    //TODO: get client window size and make sure right and _lowerExtreme are within it as well
    System.out.println("KeyMap _minInterKeyRadius: "+Double.toString(_minInterKeyRadius));
    System.out.println("KeyMap bounds (x:y): "+Double.toString(_leftExtreme)+","+Double.toString(_rightExtreme)+":"+Double.toString(_upperExtreme)+","+Double.toString(_lowerExtreme));
  }

  //Init the coordinates of keys, based on an input file containing pixel/int x/y coordinates for keys.
  // Precondition: keyMap entries must have already been created for all keys. This function just fills in the coordinate data for existing entries.
  public void initKeyCoordinates(String keyCoordinateFileName)
  {
    try{
      BufferedReader br = new BufferedReader(new FileReader(keyCoordinateFileName));
      String line;
      String[] tokens;

      //error-check: keymap must be populated with key-val entries in order to set coordinates for the existing entries
      if(_keyMap.isEmpty()){
        System.out.println("ERROR key map not yet loaded with key-value entries in initKeyCoordinates()");
        return;
      }

      while((line = br.readLine()) != null){
        // process the line
        tokens = line.split("\t");
        if(tokens.length != 3){
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< incorrectly formatted with "+Integer.toString(tokens.length)+" tokens per line: "+line);
        }
        else if(!Utilities.IsIntString(tokens[1]) || !Utilities.IsIntString(tokens[2])){ //verify integer inputs
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< coordinates incorrectly formatted: "+tokens[1]+"/"+tokens[2]);
        }
        else{
           try{
             int x = Integer.parseInt(tokens[1]);
             int y = Integer.parseInt(tokens[2]);
             char c = tokens[0].toUpperCase().charAt(0);
             
             //enter these coordinates for this char
             if(_keyMap.containsKey(c)){
               _keyMap.get(c).GetPoint().SetXY(x,y);
             }
             else{
               System.out.println("ERROR char >"+Character.toString(c)+"< not found in keyMap. Key coordinate file entries misaligned with key initialization in initKeyNeighbors()");
             }
           }
           catch(NumberFormatException e){
             System.out.println("Number format exception >"+e.getCause().getMessage()+"< caught for key map coordinate file >"+keyCoordinateFileName+"<  Coordinates incorrectly formatted");
           }
        }
      }
      br.close();
    }
    catch(java.io.FileNotFoundException ex){
      System.out.println("ERROR FileNotFoundException thrown in KeyMap.initKeyCoordinates for file: "+keyCoordinateFileName);
    }
    catch(java.io.IOException ex){
      System.out.println("ERROR IOException thrown in KeyMap.initKeyCoordinates reading file: "+keyCoordinateFileName);
      System.out.println("Cause: "+ex.getCause());
    }
  }

  //initializes the keymap based on the on-screen keyboard layout. If the layout changes, this must also change
  //This function only initilializes the key entries in the map and the key layout relative to neighbor keys. Establishing
  //each key's coordinates is done in initKeyCoordinates()
  //TODO: punctuation is not defined! OR worse, defined here but not in the key coordinate file. Also missing spacebar here
  public void initKeyNeighbors()
  {
    _keyMap.put('Q', new Key('Q', new String("WA")));
    _keyMap.put('W', new Key('W', new String("QASE")));
    _keyMap.put('E', new Key('E', new String("RDSW")));
    _keyMap.put('R', new Key('R', new String("TFDE")));
    _keyMap.put('T', new Key('T', new String("YGFR")));
    _keyMap.put('Y', new Key('Y', new String("UHGT")));
    _keyMap.put('U', new Key('U', new String("IJHY")));
    _keyMap.put('I', new Key('I', new String("OKJU")));
    _keyMap.put('O', new Key('O', new String("PLKI")));
    _keyMap.put('P', new Key('P', new String("LO;")));
    _keyMap.put('A', new Key('A', new String("QWSZ")));
    _keyMap.put('S', new Key('S', new String("AWEDXZ")));
    _keyMap.put('D', new Key('D', new String("SERFCX")));
    _keyMap.put('F', new Key('F', new String("DRTGVC")));
    _keyMap.put('G', new Key('G', new String("FTYHBV")));
    _keyMap.put('H', new Key('H', new String("GYUJNB")));
    _keyMap.put('J', new Key('J', new String("HUIKMN")));
    _keyMap.put('K', new Key('K', new String("JIOL,M")));
    _keyMap.put('L', new Key('L', new String("KOP;.,")));
    _keyMap.put(';', new Key(';', new String("PL.?\"")));
    _keyMap.put('\"', new Key('\"', new String(";?")));
    _keyMap.put('Z', new Key('Z', new String("ASX")));
    _keyMap.put('X', new Key('X', new String("ZSDC")));
    _keyMap.put('C', new Key('C', new String("XDFV")));
    _keyMap.put('V', new Key('V', new String("CFGB")));
    _keyMap.put('B', new Key('B', new String("VGHN")));
    _keyMap.put('N', new Key('N', new String("BHJM")));
    _keyMap.put('M', new Key('M', new String("NJK,")));
    _keyMap.put(',', new Key(',', new String("MKL.")));
    _keyMap.put('.', new Key('.', new String(",;?L")));
    _keyMap.put('?', new Key('?', new String(".;\"")));
    _keyMap.put(' ', new Key(' ', new String("!")));  //TODO: Space character is an exception, having no neighbors. Is this safe elsewhere, wherever neighbors are expected?
  }

  //Util, returns true if a point is within the active region of the ALPHA characters on the keyboard layout
  public boolean InBounds(Point pt){
    if((pt.GetX() < _rightExtreme) && (pt.GetX() > _leftExtreme)){
      if((pt.GetY() < _lowerExtreme) && (pt.GetY() > _upperExtreme)){
        return true;
      }
    }
    return false;
  }
} //end keyMap class

