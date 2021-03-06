

//math?

/*
  TODO: file path escape for different operating systems (can Java resolve these internally???)
  TODO: all the public/private qualifiers and class-file break down
  TODO: coding standards, make em purty. Change the capital case methods to camel case, like the java libs, if desired...
  TODO: check and evaluate enforcement of uppercase requirement; use uppercase char encodings throughout, but perhaps catch potential lowercase leaks at their root
  TODO: keymap update stuff. keyMap is stateful, since the user may change the location and size of keys, so these will need to be update then
  TODO: check operator precedence usage compared with c++, especially in compound conditionals, etc

*/


public class FastMode{

  public class Point
  {
    int m_X;
    int m_Y;
    public Point(){
      m_X = 0;
      m_Y = 0;
    }
    public Point(int x, int y){
      SetXY(x,y);
    }
    public void InitPoint(Point p){
      m_X = p.GetX();
      m_Y = p.GetY();
    }
    public int GetX(){
      return m_X;
    }
    public int GetY(){
      return m_Y;
    }
    public void SetXY(int X, int Y){
      m_X = X;
      m_Y = Y;
    }
    public void SetY(int Y){
      m_Y = Y;
    }
    public void SetX(int X){
      m_X = X;
    }
    @Override
    public String toString(){
      return "("+Integer.toString(m_X)+","+Integer.toString(m_Y)+")";
    }
    public static double DoubleDistance(Point p1, Point p2){
      return Math.sqrt(Math.pow((double)(p1.GetX() - p2.GetX()),2.0) + Math.pow((double)(p1.GetY() - p2.GetY()),2.0));
    }
  } //end Point class

  // a PointMu (mu like mean) represents a trigger point or a cluster center for a sequence of x/y inputs.
  // This may be either something discrete like a key press, or something stochastic, like "I think there was a click here" and my_ticks determines the likelihood of a click
  public class PointMu
  {
    char m_alpha;
    Point m_pt;
    int m_ticks;

    public PointMu(){
      m_pt.SetXY(0,0);
      m_ticks = 0;
      m_alpha = 'A';
    }
    public PointMu(Point point, int ticks){
      m_pt = new Point(point.GetX(), point.GetY());
      m_ticks = ticks;
      m_alpha = 'A';
    }
    public PointMu(Point point, int ticks, char c){
      m_pt = new Point(point.GetX(), point.GetY());
      m_ticks = ticks;
      m_alpha = c;
    }
    public void SetAlpha(char c){
      m_alpha = c;
    }
    public void SetPoint(Point pt){
      m_pt.SetXY(pt.GetX(),pt.GetY());
    }
    public void SetTicks(int t){
      m_ticks = t;
    }
    public char GetAlpha(){
      return m_alpha;
    }
    public int GetTicks(){
      return m_ticks;
    }
    public Point GetPoint(){
      return m_pt;
    }
  } //end PointMu class

  public class SearchResult implements Comparable<SearchResult>
  {
    double m_score;
    String m_word;  //note this will be used only as a reference to a string in the wordModel, not an actual String, per Java rules
    public SearchResult(){
      m_score = 0.0;
      m_word = "";
    }
    public SearchResult(double score, String word){
      m_score = score;
      m_word = word;
    }
    public double GetScore(){
      return m_score;
    }
    @Override
    public String toString(){
      return m_word+":"+Double.toString(m_score);
    }

    @Override
    public int compareTo(SearchResult res) {
        //return Comparators.SCORE.compare(this, o);
      int ret;

      ret = (this.GetScore() < res.GetScore()) ? 1 : -1;

      return ret;
    }

    /*
    //TODO: evaluate this pattern, I yanked it from the net. Its a nice pattern, since you can create new comparators if needed.
    public static class Comparators{

      /*
      //TODO: delete this original
      public static class SearchResultDistanceComparator implements Comparator<SearchResult>{
        public int compare(SearchResult result1, SearchResult result2) {
          int ret;
          ret = (result1.GetScore() < result1.GetScore()) ? 1 : -1;
          return ret;
        }
      }
      

        public static Comparator<SearchResult> SCORE = new Comparator<SearchResult>() {
            @Override
            public int compare(SearchResult result1, SearchResult result2) {
              int ret;
              ret = (result1.GetScore() < result1.GetScore()) ? 1 : -1;
              return ret;
            }
        };
      }
      */
  }

  //Every key has a char id, a location (Point), and a list of its neighbors (as a string)
  public class Key
  {
    char m_id;
    Point m_center;
    String m_neighbors; //raw neighbors are just stored as a string
 
    public Key(){
      m_id = '\0';
      m_neighbors = "";
    }
    public Key(char c, String neighborStr){
      m_id = c;
      m_neighbors = neighborStr;
    }
    public String GetNeighbors(){
      return m_neighbors;
    }
    public char GetId(){
      return m_id;
    }
    public Point GetPoint(){
      return m_center;
    }
    public void SetNeighbors(String neighbors){
      m_neighbors = neighbors;
    }
    public void SetPoint(Point p){
      m_center.SetXY(p.GetX(), p.GetY());
    }
    public void SetPoint(int x, int y){
      m_center.SetXY(x,y);
    }
  } //end Key class

  //TODO: this is a temporary abstraction. Use the Team Force data structure, 
  public class KeyMap
  {
    Map<Character,Key> m_keyMap;
    double minInterKeyRadius; // defined as one-half the distance between the two nearest alpha keys
    double leftExtreme, rightExtreme, upperExtreme, lowerExtreme; //defines the ui boundaries for alpha chars

    public KeyMap(){
      minInterKeyRadius = 0;
      leftExtreme = 0;
      rightExtreme = 0;
      lowerExtreme = 0;
      upperExtreme = 0;
    }

    public KeyMap(String keyCoordinateFileName){
      initKeyMap(keyCoordinateFileName);
    }

    public double GetMinInterKeyRadius(){
      return minInterKeyRadius;
    }

    //util, lookup a Point (value) given char c (a map key)
    //TODO: error checking on null returns, if c is not in map keys      
    public Point GetPoint(char key){
      Point ret = null;

      if(m_keyMap.containsKey(key)){
        ret = m_keyMap.get(key).GetPoint();
        if(ret == null){
          System.out.println("ERROR key >"+key+"< found in map, with null Point reference");
          ret = m_keyMap.get('G').GetPoint();  //return something we know is in the map
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
      char c;
      double dist, min;

      //rather inefficiently searches over the entire keyboard... this could be improved with another lookup datastructure
      min = 99999.0;
      for(Key key : m_keyMap.values()){
        if(IsAlpha(key.GetId())){
          dist = Point.DoubleDistance(key.GetPoint(),pt);
          if(min > dist){
            min = dist;
            if(min < minInterKeyRadius){  //optimization. return when dist is below some threshold, such that no other key could be nearer
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
      initDerivedGeometryVals(); //initializes various geometric properties (minInterKeyRadius, etc) used in some of the inference logic
    }

    /*
      FastMode needs a few geometric values such as the minimum distance between keys, and the layout boundaries.
      The layout boundaries are used to determine if a sensor value (an X/Y pair) is omitted, and min-key distance
      can be used in various ways for logic questions and for generating/smoothing probability distributions. Like maybe
      you have a raw trigger point (x,y), and a list of its nearest neighbor keys, each with its own (x,y) center. A probability
      for the "intended key" can be based in various ways upon whether or not the trigger point is within the minInterKeyRadius distance.
      Or say you look up the neighbors of the nearest key, and want to use the minInterKey distance to omit neighbor keys that are greater
      than some scaled minInterKeyRadius, such that somewhat far neighbor keys aren't evaluated or are assigned a rapidly diminishing probability.
      A few basic geometry values can be used in this way to scale probabilities in an intuitive way, and to assign precise values when answering
      questions like, given some trigger point and its nearest neighbors, determine the most likely "intended" key press. And other such
      "regional" queries for the interface.

      NOTE: the values are only based in alpha characters, since only these drive the fastmode inference process.
    */
    public void initDerivedGeometryVals()
    {
      int curX, curY;
      String alphaStr = new String("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

       if(m_keyMap.isEmpty()){
         System.out.println("ERROR key map not yet created");
       }

       //order of these calls matters, as initMinInterKeyRadius() uses value established in initKeyBoundaries()
       initKeyBoundaries();
       initMinInterKeyRadius();
    }

    //sets the minimum interkey distance, defined as one half the center-to-center distance between the nearest two keys
    private void initMinInterKeyRadius()
    {
      double dist;

      minInterKeyRadius = 999999;
      //find the minimum interkey distance, the dist between the centers of the two nearest keys.
      for(Key outerKey : m_keyMap.values()){
        for(Key innerKey : m_keyMap.values()){
          if(IsAlpha(outerKey.GetId()) && IsAlpha(innerKey.GetId())){
            if(outerKey.GetId() != innerKey.GetId()){ //dont compare keys with themselves!
              dist = Point.DoubleDistance(outerKey.GetPoint(),innerKey.GetPoint()) / 2.0;
              if(dist < minInterKeyRadius){
                minInterKeyRadius = dist;
              }
            }
          }
        }
      }  //post-loop: minInterKeyRadius holds one half the distance between the two nearst alpha keys
    }

    //calculates the left, right, upper, and lower-most boundaries based on the alpha characters in the layout
    private void initKeyBoundaries()
    {
      int curX, curY;

       //to add as much confusion as possible, the upper left window corner is defined as (0,0), so "upperExtreme" will be less than lowerExtreme
       leftExtreme = 99999; rightExtreme = 0; upperExtreme = 99999; lowerExtreme = 0;
       for(Key kbKey : m_keyMap.values()){
         if(IsAlpha(kbKey.GetId())){ //verify its an alpha
           curX = kbKey.GetPoint().GetX();
           curY = kbKey.GetPoint().GetY();

           if(leftExtreme > curX){
             leftExtreme = curX;
           }
           if(rightExtreme < curX){
             rightExtreme = curX;
           }
           if(upperExtreme > curY){
             upperExtreme = curY;
           }
           if(lowerExtreme < curY){
             lowerExtreme = curY;
           }
         }
       }

       //We just found the most extreme left/right/lower/upper values among the centers of the keys,
       //but its the boundaries we're after. So add the minInterKeyRadius to each to expand the boundaries a little.
       lowerExtreme += minInterKeyRadius;
       upperExtreme -= minInterKeyRadius;
       rightExtreme += minInterKeyRadius;
       leftExtreme -= minInterKeyRadius;

       //check if we've underflowed the boundaries of the client window, and adjust
       if(upperExtreme < 0){
         upperExtreme = 0;
       }
       if(leftExtreme < 0){
         leftExtreme = 0;
       }
       //should check the right and lower, but there is no equivalent way to see if they extend too far.
       //TODO: get client window size and make sure right and lowerExtreme are within it as well
    }

    //Init the coordinates of keys, based on an input file containing pixel/int x/y coordinates for keys.
    // Precondition: keyMap entries must have already been created for all keys. This function just fills in the coordinate data for existing entries.
    public void initKeyCoordinates(String keyCoordinateFileName)
    {
      BufferedReader br = new BufferedReader(new FileReader(keyCoordinateFileName));
      String line;
      String[] tokens;

      //error-check: keymap must be populated with key-val entries in order to set coordinates for the existing entries
      if(m_keyMap.isEmpty()){
        System.out.println("ERROR key map not yet loaded with key-value entries in initKeyCoordinates()");
        return;
      }

      while((line = br.readLine()) != null){
        // process the line
        tokens = line.trim().split("\t");
        if(tokens.length != 3){
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< incorrectly formatted with "+Integer.toString(tokens.length)+" tokens per line");
        }
        else if(isIntString(tokens[1]) && isIntString(tokens[2])){ //verify 
          System.out.println("ERROR key map coordinate file >"+keyCoordinateFileName+"< coordinates incorrectly formatted");
        }
        else{
           try{
             int x = Integer.parseInt(tokens[1]);
             int y = Integer.parseInt(tokens[2]);
             char c = tokens[0].toUpperCase().charAt(0);
             
             //enter these coordinates for this char
             if(m_keyMap.containsKey(c)){
               m_keyMap.get(c).GetPoint().SetXY(x,y);
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

    //initializes the keymap based on the on-screen keyboard layout. If the layout changes, this must also change
    //This function only initilializes the key entries in the map and the key layout relative to neighbor keys. Establishing
    //each key's coordinates is done in initKeyCoordinates()
    //TODO: punctuation is not defined! OR worse, defined here but not in the key coordinate file. Also missing spacebar here
    public void initKeyNeighbors()
    {
      m_keyMap.put('Q', new Key('Q', new String("WA")));
      m_keyMap.put('W', new Key('W', new String("QASE")));
      m_keyMap.put('E', new Key('E', new String("RDSW")));
      m_keyMap.put('R', new Key('R', new String("TFDE")));
      m_keyMap.put('T', new Key('T', new String("YGFR")));
      m_keyMap.put('Y', new Key('Y', new String("UHGT")));
      m_keyMap.put('W', new Key('W', new String("IJHY")));
      m_keyMap.put('I', new Key('I', new String("OKJU")));
      m_keyMap.put('O', new Key('O', new String("PLKI")));
      m_keyMap.put('P', new Key('P', new String("LO;")));
      m_keyMap.put('A', new Key('A', new String("QWSZ")));
      m_keyMap.put('S', new Key('S', new String("AWEDXZ")));
      m_keyMap.put('D', new Key('D', new String("SERFCX")));
      m_keyMap.put('F', new Key('F', new String("DRTGVC")));
      m_keyMap.put('G', new Key('G', new String("FTYHBV")));
      m_keyMap.put('H', new Key('H', new String("GYUJNB")));
      m_keyMap.put('J', new Key('J', new String("HUIKMN")));
      m_keyMap.put('W', new Key('W', new String("JIOL,M")));
      m_keyMap.put('L', new Key('L', new String("KOP;.,")));
      m_keyMap.put(';', new Key(';', new String("PL.?\"")));
      m_keyMap.put('\"', new Key('\"', new String(";?")));
      m_keyMap.put('Z', new Key('Z', new String("ASX")));
      m_keyMap.put('X', new Key('X', new String("ZSDC")));
      m_keyMap.put('C', new Key('C', new String("XDFV")));
      m_keyMap.put('V', new Key('V', new String("CFGB")));
      m_keyMap.put('B', new Key('B', new String("VGHN")));
      m_keyMap.put('N', new Key('N', new String("BHJM")));
      m_keyMap.put('M', new Key('M', new String("NJK,")));
      m_keyMap.put(',', new Key(',', new String("MKL.")));
      m_keyMap.put('.', new Key('.', new String(",;?L")));
      m_keyMap.put('?', new Key('?', new String(".;\"")));
    }

    //Util, returns true if a point is within the active region of the ALPHA characters on the keyboard layout
    public boolean InBounds(Point pt){
      if(pt.GetX() < rightExtreme && pt.GetX() > leftExtreme){
        if(pt.GetY() < lowerExtreme && pt.GetY() > upperExtreme){
          return true;
        }
      }
      return false;
    }
  } //end keyMap class

  /*
    The core responsibility of this class is to consume a stream of raw (x,y) coordinates from the eye tracking hardware,
    and the extract the likely key-events within that stream. For instance, it may be a characteristic that when the user
    hits a key, the standard deviation of the last k x/y tuples falls below some threshold, signalling high density. In that
    very simple pattern recognition scheme, this class would then seek to extract from the stream all the events that occurred
    when the standard deviation falls below this threshold, signalling a likely key-press. That's just one model, and there are
    many kinematic parameters you can throw at the input stream to do pattern recogntion: dx/dy, standard deviation, 
    acceleration/velocity vectors, theta, delta-theta, anything you can think of that will help identify when a key press occurs, 
    but without overestimating them. Note also a side effect of event-based pattern recognition is that two consecutive events may
    be triggered for the same key due to noise, and other unusual events. So it is a subsidiary responsibility of this class
    to do things like consolidating repeated events, and so on, to clean the input and output as much as possible before sending
    it along to the inference class.

    In engineering terms I guess you might call this a transformer class, or a "filter" in a pipe-and-filter pattern: the class
    has no data members, it just takes an input stream, transforms it somehow, and passes along its output to the next class.
    It needs a reference to a KeyMap, and is currently the only FastMode sub-class accessing the key map. But its conceivable other
    classes might need to do so also in the future, so I put the keyMap at the outer class level, and pass it as a ref to this subclass.
  */
  public class SignalProcessor
  {
    KeyMap m_keyMapRef;
    int m_triggerThreshold, m_dxThreshold, m_innerDxThreshold;

    public SignalProcessor(){
      System.out.println("ERROR SignalProcessor default constructor called, with no keyMap parameter. Use ctor with KeyMap parameter only.");
    }
    public SignalProcessor(KeyMap kmap, int triggerThresh, int dxThresh, int innerDxThresh){
      m_keyMapRef = kmap; //shallow copy of the KeyMap
      SetEventParameters(triggerThresh,dxThresh,innerDxThresh);
    }
    public void SetEventParameters(int triggerThresh, int dxThresh, int innerDxThresh){
      m_triggerThreshold = triggerThresh;
      m_dxThreshold = dxThresh;
      m_innerDxThreshold = innerDxThresh;
    }


    /*
      The primary method. Given an raw input stream/list of (x,y), this attempts to extract the likely key presses
      using pattern recognition schemes. Don't get all hot and impressed with the term "pattern recognition", because
      most of the logic therein just comes from basic kinematics and statistics. In other words, be creative when trying
      to come up with better, faster methods for the event/key-press extraction process. Come up with whatever you can
      in order to anticipate key presses. The challenge is not just detecting events/key presses, but not over-detecting
      them either. A good testing practice to find better event-detection methods and values is to simply define those values
      and print them out, and sketch a graph for how and when they change. For instance, the average theta value (direction vector)
      for the last few points provides a good idea of current direction; thus, delta-theta, some rapid change in direction,
      could be used to help detect events. However, not all events entail a direction change, so it may be a sufficient but not a
      necessary characteristic for triggering an event. There's a lot of data values which you can extract from the raw input
      stream that work this way, the question is how to leverage them, and how perhaps to map them into something like an SVM
      or a neural network that can magically decide for itself when to trigger an event.

      The conservative and reliable measure is to simply use the product of the standard deviation of the last k points, where k is a parameter
      that may depend on the sensor rate (30 or 50 Hx, currently) or other factors. That is, read the input stream and calculate the
      product of the standard deviations of the last say, three (k=3) X and Y points:
        stDevProduct = stdev(point1.X,point2.X,point3.X) * stdev(point1.Y,point2.Y,point3.Y)
      Since this is a product, the value is very high when the eye is moving rapidly or afar to some new key, and the value decreases
      rapidly when the eye is focused on a key (signalling high input density). In my own testing I found this to be a value with very
      reliable characteristics, such that you can easily trigger an event when the stDevProduct falls below some threshold, signalling
      focus on a key. However, per the earlier comments, it is a trailing value. It is very good in the sense that it would be easy for the user
      to "learn" the trigger characteristics of this value, but there are leading values like dy/dx and delta-theta that could help anticipate
      events and trigger them with even greater precision and reliability and of course faster.

      But as far as signal processing goes, realize this is a bit of a goldilocks function. The parameters for triggering events
      need to be "just so", not too hot, not too cold. You just play with the parameters until they feel right, because they 
      will have to change with the sensor hardware. You can factor the parameters out as best you can, but their values still
      just have to be determined by physical testing.
    */
    public void Process(ArrayList<Point> inData, ArrayList<PointMu> meansList)
    {
      ArrayList<PointMu> intermediateData;
      int i, trigger, eventStart, eventEnd;
      double dx;

      System.out.println("Singularity builder processing "+Integer.toString(inData.size())+" raw data points");

      //TODO: sleep if no data
      trigger = 0;
      for(i = 0; i < inData.size() - 4; i++){
        //ignore points outside of the active region, including the <start/stop> region
        if(m_keyMapRef.InBounds(inData.get(i)) && m_keyMapRef.InBounds(inData.get(i+3))){
          dx = Point.DoubleDistance(inData.get(i),inData.get(i+3));

          /*
          //debug output, to view how data vals change
          System.out.println("dx="+dx.toString());
          if(dx > 0){
            System.out.println("1/dx="+(1/dx).toString());
          }
          */

          //TODO: advancing the index i below is done without InBounds() checks
          if(dx < m_dxThreshold){  //determine velocity: distance of points three ticks apart
            trigger++;           //receive n-triggers before fully triggering, to buffer noise; like using a timer, but event-based
            if(trigger >= m_triggerThreshold){  //trigger event and start collecting event data

              //capture the event
              eventStart = i;
              while(i < (inData.size() - 4) && dx < m_innerDxThreshold){  //event state. remain in this state until dX (inter-reading) exceeds some threshold
                dx = Point.DoubleDistance(inData.get(i),inData.get(i+3));
                i++;
              }
              eventEnd = i;   // exited Event state, so store right bound of the event cluster
              //TODO: below is a small optimization to skip some data points following an event, eg, perhaps by dx difference of 3 data points.
              //i += 2;

              //get the mean point w/in the event cluster and store it
              PointMu outPoint;
              outPoint = CalculateMean(eventStart,eventEnd,inData);
              System.out.println("Hit mean");
              outPoint.SetAlpha(m_keyMapRef.FindNearestAlphaKey(outPoint.GetPoint()));

              //NOTE A new cluster is appended only if it is a unique letter; this prevents repeated chars.
              if(intermediateData.isEmpty()){  //this is just an exception check, so we don't deref a -1 index in the next if-stmt, when the vec is empty
                intermediateData.add(outPoint);
              }
              //verify incoming alpha cluster is unique from previous alpha
              else if(outPoint.GetAlpha() != intermediateData.get(intermediateData.size()-1).GetAlpha()){
                intermediateData.add(outPoint);
              }

              //reset trigger for next event detection
              trigger = 0;
            } //exit the event-capture state, and continue streaming (reading inData and detecting the next event)
          }
        }
      }

      System.out.println("Clusters before merging...");
      PrintMeans(intermediateData);
      MergeMeans(intermediateData,meansList);
      //dbg
      //PrintOutData(outData);
    }

    /*
      Takes a list of points, a begin and end index for some estimate point cluster,
      and returns the mean point of that cluster.

      Notes: This potentially suffers from the "elbow" problem, but I never found it to be a problem.
      Most "clusters" (events) trigger at a sharp angle: the user is gazing from one letter to another,
      creating a sharp corner in the trace of their "gazes". This means that the mean value of that cluster
      (including both edges of the angle) will be inside the angle, instead of very near the point. But we're
      after the point of this corner, when/if it occurs. But notice that with more points in the cluster 
      (a more precise trigger threshold for instance), or fewer extrema in that cluster, then the problem vanishes.
      Again, I never found it to be a problem, ever.
    */
    public PointMu CalculateMean(int begin, int end, ArrayList<Point> coorList)
    {
      PointMu pointMean = new PointMu();
      int sumX, sumY, ct, i; // pixel math is all integer based
      
      //TODO: error checks on begin and end, esp. wrt coorList.size() and so on

      sumX = sumY = ct = 0;
      for(i = begin; i < end && i < coorList.size(); i++){
        if(m_keyMapRef.InBounds(coorList.get(i))){  //only accumulate valid points, not extrema outside the bounds of the key region of the ui
		      sumX += coorList.get(i).GetX();
		      sumY += coorList.get(i).GetY();
		      ct++;
        }
      }

      if(ct > 0){
        pointMean.GetPoint().SetXY(sumX / ct,sumY / ct);
      }
      else{
        System.out.println("ERROR ct==0 in CalculateMean ?? Should never occur");
        pointMean.GetPoint().SetXY(0,0);
      }

      //set the ticks (a confidence level for the cluster/event)
      pointMean.SetTicks(end - begin);

      return pointMean;
    }

    private void PrintMeans(ArrayList<PointMu> meansList)
    {
      int i = 1;
      for(PointMu mean : meansList){
        System.out.println(Integer.toString(i)+":  "+mean.GetPoint().toString()+" "+mean.GetAlpha());
        i++;
      }
    }

    /*
      A subsidiary responsibility of the SignalProcessor class is to clean the output. This
      removes redundant means from the output data. Means are "redundant" if they are the same
      character as the previous mean, or if they are within some very small distance of the previous
      mean point. This is intended to scrub over-detection cases, where the primary Process() function
      triggers too many events for activity in the same region, like if the user looks at 'A' and we receive 
      several triggers/events on 'A' or like on the border between 'A' and other keys.

      This may seem inefficient, like the Process() function should be responsible for not over-detecting. While the
      latter is the case (to a degree), its also the case that if you relax the Process() function and allow it to
      over-detect a little bit, then it can give very fast event-detection, but with less accuracy. But then we can
      just do some fast post-processing to remove the over-detection cases, which is what this function does. Its like
      allowing the Process() function to have a bit of an itchy-trigger finger so it always hits the mark, but then just
      correcting for it firing too many times on a single mark.
    */
    private void MergeMeans(ArrayList<PointMu> intermediateData, ArrayList<PointMu> mergedData)
    {
      int i;
      boolean lastMerge = false;

      if(!mergedData.isEmpty()){
        mergedData.clear();
      }

      for(i = 1; i < intermediateData.size(); i++){
        // Only append output clusters which exceeds some inter-key distance
        // Note that this discrimination continues until sufficient inter-cluster distance is achieved.
        if(MinSeparation(intermediateData.get(i-1),intermediateData.get(i))){
          mergedData.add(intermediateData.get(i-1));
        }
        //else, forward-accumulate the reflexive likelihood to preserve repeat char info
        else{
          System.out.println("Merged clusters "+Integer.toString(i-1)+"/"+Integer.toString(i));
          //identical alphas, so just merge the dupes, for instance, merge "AA" to "A"
          if(intermediateData.get(i-1).GetAlpha() == intermediateData.get(i).GetAlpha()){
            intermediateData.get(i-1).SetTicks(intermediateData.get(i-1).GetTicks() + intermediateData.get(i).GetTicks());
            mergedData.add(intermediateData.get(i-1));      
          }
          //else, point with more ticks (higher confidence) wins (outcome is same as prior 'if': these two blocks could be merged, but with less clarity
          else if(intermediateData.get(i-1).GetTicks() > intermediateData.get(i).GetTicks()){
            intermediateData.get(i-1).SetTicks(intermediateData.get(i-1).GetTicks() + intermediateData.get(i).GetTicks());
            mergedData.add(intermediateData.get(i-1));
          }
          else{
            intermediateData.get(i).SetTicks(intermediateData.get(i).GetTicks() + intermediateData.get(i-1).GetTicks());
            mergedData.add(intermediateData.get(i));
          }
          i++; //TODO: this advances the index to skip the next comparison. As a result, this method only merges two
               // consecutive 'merge-able' means, when in reality, there may be multiple ones. For that reason,
               // this should really be iteration (when minSep fails). That way successive errors are absorbed.

          //intermediateData.get(i).GetTicks() += intermediateData.get(i-1).GetTicks();
          if(i >= intermediateData.size()-1){
            lastMerge = true;
          }
        }
      }

      //variable detects if last two clusters were merged or not
      if(!lastMerge && !intermediateData.isEmpty()){
        mergedData.add(intermediateData.get(intermediateData.size()-1));
      }
    }

    /*
      Determines if two point means are "sufficiently" distinct to be called separate.
      Typical usage is verifying whether or not two consecutively detected point means are
      different enough (in terms of distance, char-id, or ticks) to say that they are separate
      input events, and not just over-detection. So this is a post-processing method.
    */
    public boolean MinSeparation(PointMu mu1, PointMu mu2)
    {
      if(mu1.GetAlpha() != mu2.GetAlpha()){  //TODO: this is redundant with a check in Process(). Oh well.
        if(Point.DoubleDistance(mu1.GetPoint(),mu2.GetPoint()) < (m_keyMapRef.GetMinInterKeyRadius() * 1.5)){
          if(mu1.GetTicks() <= 4 || mu2.GetTicks() <= 4){  //time separation is INF for now
            System.out.println("minkeyrad: "+Double.toString(m_keyMapRef.GetMinInterKeyRadius())+" dist: "+Double.toString(Point.DoubleDistance(mu1.GetPoint(),mu2.GetPoint())));
            System.out.println("mindist failed, ticks are ("+mu1.GetAlpha()+","+Integer.toString(mu1.GetTicks())+")  ("+mu2.GetAlpha()+","+Integer.toString(mu2.GetTicks())+")");
            return false;
    	    }
        }
      }

      return true;
    }

    //This function should only be used in testing, not release mode. It simply copies raw data to the output means, assuming the input
    //data is already a pre-processed set of point means (key presses) and not raw eye sensor data.
    public void DummyProcess(ArrayList<Point> rawData, ArrayList<PointMu> meansList)
    {
      if(!meansList.isEmpty()){
        meansList.clear();
      }
  
      for(Point pt : rawData){
        PointMu pmu = new PointMu(pt,0);
        meansList.add(pmu);
      }

      System.out.println("DummyProcess returning "+Integer.toString(meansList.size())+" output means");
    }
  }  //end SignalProcessor class

  //This is the primary "inference" class. The signal processor class just does its best to create a stream of likely key presses,
  //while this class takes that stream (as a string) and tries to infer to which words in the vocab model the stream is nearest.
  public class DirectInference
  {
    KeyMap m_keyMapRef;
    Set<String> m_wordSet;

    //DO NOT USE DEFAULT CTOR
    public DirectInference(){
      String filePath = "../resources/vocabModel.txt";
      System.out.println("WARNING default DirectInference ctor called with no input word-file param. Only use the constructor with a file param.");
      System.out.println("Default ctor guessing file path is >"+filePath+"< which will fail if path is invalid...");
      BuildWordSet(filePath);
    }
    //ONLY USE THIS CTOR
    public DirectInference(String vocabFileName, KeyMap kmap){
      m_keyMapRef = kmap;
      System.out.println("Building word model from >"+vocabFileName+"< for DirectInference class ctor...");
      BuildWordSet(vocabFileName);
    }

    /*  Expect word model as flat file word database, one (unique) word per line.
        Requirements for the word model (at least for fastmode) are no punctuation,
        and all upper case. This is because the fastmode is more conversaitonal than
        grammatical, selecting complete words rather than contractions or other. Its
        really more of a phonetic word model than a written word model. Forward all complaints
        to the American Spell Check society.
    */
    private void BuildWordSet(String vocabFile)
    {
      //m_wordSet = new Set<String>();
      BufferedReader br = new BufferedReader(new FileReader(vocabFile));
      String line;

      if(m_wordSet.size() > 0){
        m_wordSet.clear(); //clear the word model if it contains existing items
      }

      while((line = br.readLine()) != null){
        // process the line
        String upper = line.toUpperCase();
        m_wordSet.add(upper);
      }

      br.close();
      System.out.println("Building word model completed. wordSet.size()="+Integer.toString(m_wordSet.size()));
    }

    /*
      Primary method/driver. This takes in a vector of X/Y points (triggers or simulated key-presses)
      and tries to correlate these with words to find the nearest word.
    */
    public void Process(ArrayList<PointMu> pointMeans, ArrayList<SearchResult> results)
    {
      if(results.size() > 0){
        results.clear();
      }

      //The inference method. Other methods were tested (string distance methods, combined methods, etc), but no worries,
      //The vector distance method outperformed all of them, by enormous margins. The other versions can be found in the C++ version.
      VectorDistInference(pointMeans,results);

      //print the results, for debugging
      int i;
      for(i = 0; i < 75 && i < results.size(); i++){
        System.out.println(Integer.toString(i)+": "+results.get(i).toString());
      }
    }

    /*
      An exhaustive coordinate-vector comparison method. Given a sequence of XY points (stochastic triggers or
      simulated key presses), this searches over all words in the word model, evaluating each word's distance
      to the input sequence. Nearby words (a distance below some magic threshold) are pushed to the result list,
      list is sorted and returned, nearest word first.
    */
    private void VectorDistInference(ArrayList<PointMu> pointMeans, ArrayList<SearchResult> results)
    {
      int i;
      double dist, min;

      System.out.println("In DirectInference, process...");

      if(results.size() > 0){
        results.clear();
        //results.reserve(wordSet.size() * 4);
      }

      min = 99999;
      for(String word : m_wordSet){
      //for(String it = wordSet.begin(); it != wordSet.end(); ++it){
        dist = VectorDistance(pointMeans,word);
        //dist = VectorDistance(ArrayList<PointMu>,revArrayList<PointMu>,it);  //overload for fwd-bkwd versions
        if(dist < min){
          min = dist;
        }

        //TODO: this is a magic number, change it to parameter elsewhere to make it more obvious as an input param
        if(dist < 1500){  //push threshold: nearby words
		      //System.out.println("pushing >" << *it << "," << dist << "<");
		      //results.add(SearchResult{*it,dist});
          results.add(new SearchResult(dist,word));
        }
      }

      Collections.sort(results);
      //results.sort(ByDistance);

      /*
      SearchResultIt mit;
      for(i = 0, mit = results.begin(); mit != results.end() && i < 120; i++, ++mit){
        System.out.println(i << ": " << mit->first << "|" << mit->second);
      }
      */
    }

    /*
      Core utlility of the class.  Multiple distance metrics will undoubtedly need to be devised...
      This one should only be geometry-based; later language modeling class can handle unification
      
      Distance metric currently assumes that at least the first cluster aligns with the first letter's coordinates.
    */
    private double VectorDistance(ArrayList<PointMu> pointMeans, String candidate)
    {
      double dist = 99999.0; // infinite distance, buries this result if returned by default

      //Small optmization to only compare words within +/- k length of eachother.
      //It is important to think about this value before modifying it. You want to be generous,
      //so as to encompass many words in the scope of the search; but you also want to be constrained (use a small number)
      //so the search space is smaller, and thus faster. The problem is that the input sequence does not represent
      //repeated characters in many words, like 'S' and 'P' in MISSISSIPPI. So making the value too small may omit the target word,
      //if it has k repeated characters, and the chosen omission threshold here is < k, then that word will be effectively omitted.
      if(AbsDiff(pointMeans.size(),candidate.length()) <= 4){
        //calculate the distance value for this input sequence and some word in our word model
        dist = SumDistMetric_Aligned(pointMeans,candidate);
      }

      return dist;
    }

    /*
	    //aligned version of previous. this is a much harder problem to program.
	    //Currently just looks left/right two chars for a nearer neighbor key

      Intuitively, this is trying to gather the distance of all input points to the nearest, sequential ray
      trace given by the word's coordinates. There is probably a better approach to this than the hacky
      state-based method use here (detect-error, look-ahead, punish, discount, etc).

      This handles insertion error well. But not deletion error, such as if SignalProcessor outputs "QUCK" instead of "QUICK".
      Try to handle "QUCK" "QUICK" case. This may be handled the same as insertion error, by advancing the candidate instead of the
      cluster-point sequence.

      The error values for different types of error are calibrated like a small constraint programming problem. For instance,
      really junky, significant differences in the distance between two geometric sequences should be punished severely. However,
      if one word is a root of another, don't punish the difference in input too much, lest one actually is a root of the other.
      Likewise, give a very small punishment to strings which differ only in the number of repeats, so "DOG" has a higher rank than
      "DOGG" (since repeated chars are eaten). In this way, the intent is to go from coarse to fine "punishment" in terms of string distance,
      such that each successive measure only re-ranks items within some subgrouping.

      An easier way to understand the error arithmetic is in terms of base-ten numbers. Say I have two models that provide scores
      for some objects. The first model gives more significant information than the second, such that I always want to depend more
      on the first model, and rely on the second model only as a tie breaker (or when the first model has no information).
      The easy thing to do is to multiply the 1st model's score for 10, and multiply the scores of the second model by 1, and sum them.
      Thus the scoring is a sum of powers of 10: score1 * 10^1 + score2 * 10^0. Under this scheme, as long as range of each model's scores
      are less than the bucket size (so ranging from 0-9, in this base-ten scheme), then it will always be the case that the first model has
      highest precedence in terms of scoring, followed by the second model. This can be extended to an arbitrary number of models by simply
      appending "buckets" for new models (which are just the columns of some base ten number). So a three model system would be
      score3 * 10^2 + score2 * 10^1 + score1 * 10^0.

      "_Aligned" suffix of this function comes from the fact that this version does a modest edit-distance estimate to compensate for insertion
      and deletion errors in the input stream (extra or missing characters, such as "DOGG" or "DG" instead of "DOG"). True edit-distance
      metrics are dynamic programming methods that use quadratic space/time (size and complexity is string1.length * string2.length). So
      while this method doesn't do that, it approximates the quadratic edit-distance methods for insert/delete accounting in linear time
      by skipping chars and looking for string-realignment using basic assumptions, like checking if the next-two chars match in each
      string, etc. This works only because words are somewhat short (5-15 chars or so). If the input strings were of arbitrarily large
      length, then the approximation wouldn't work and you'd have to use the real dynamic programming methods (which are available in the C++ version).
      "Unaligned" versions were also tested, but this version performed the best, in linear time. Unaligned refers to ignoring insertion
      and deletion errors all together. You just scan the characters in each string pairwise for equality, so if there is an extra char
      in one string, then all the rest of the equality tests fail (Hamming distance). But since insert/deletion errors are infrequent, you can 
      cheat by just running Hamming distance in both directions, and summing the result: pairwise scan each string from the beginning of each string,
      then run it backward by starting from the end of each string. The error will act as a pivot. The assumption is that even with the error(s),
      running in both directions over the correct edit-word and the user input will give a value that is at least less than all (or nearly all)
      other words in the word database. There will still be some error, especially for shorter words (which contain less information), but you
      might decide to tolerate this for the sake of speed, and then just run language estimates (word n-gram) probabilities over the first
      twenty or so results in the output to find the more likely word, per a language model.
    */
    private double SumDistMetric_Aligned(ArrayList<PointMu> pointMeans, String candidate)
    {
      double sumDist, dist;
      int i, j, rpts, edts;

      //assumes a tight edit-error bound, such that alignment of points occurs within +/- one index 
      rpts = edts = 0;
      sumDist = dist = 0.0;
      for(i = 0, j = 0; i < pointMeans.size() && j < candidate.length(); i++, j++){
        //optimization: only compare distances if letters differ
        if(pointMeans.get(i).GetAlpha() != candidate.charAt(j)){
          //insertion error: essentially, this is a soft check whether *deleting* (skipping) the current letter yields realignment
          if((i+1) < pointMeans.size() && pointMeans.get(i+1).GetAlpha() == candidate.charAt(j)){
            if(j > 0){  //accumulate error, so this character difference receives at least some punishment
              sumDist += InsertionError(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j-1)), m_keyMapRef.GetPoint(candidate.charAt(j)));
            }
            else{ //the first is just an exception case, when we don't have two points to compare. so just compare char i to j
              sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
            }
            i++;  //skip the input error
            edts++;
          }
          //new, 1/14/14: attempts to handle cluster="QUCK" candidate="QUICK" case, where letters are missing from the cluster-points. 
          //In this case, do the opposite of the insertion error case, advancing candidate index
          else if((j+1) < candidate.length() && pointMeans.get(i).GetAlpha() == candidate.charAt(j+1)){
            //deletion error may be remedied the same as insertion error; however, observe that most deletions occur
            //when char i is detected, but i+1 is not, since its just too near to detect. Thus, the distance should be assessed wrt i,
            //instead of the midpoint (as for insertion error).
            if(i > 0 && pointMeans.size() > 1){
              sumDist += Point.DoubleDistance(pointMeans.get(i-1).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
              //sumDist += InsertionError(m_keyMapRef.GetPoint(candidate.charAt(j)), pointMeans.get(i).GetPoint, pointMeans.get(i-1).GetPoint());
            }
            //just an exception case, protecting the pointMeans bounds i and i-1
            else{
              sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
            }
            j++;
            edts++;
          }
          //else, assume we're just off the mark, and let distance accumulate
          //note we hit this case either if the next letters also differ, or if the current (differing) letters includes the last letter of candidate
          else{
            sumDist += Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
          }
        }

        //advances over repeated chars in the candidate. This is tracked with the rpts var, so we at least know about it
        if(j < (candidate.length()-1) && candidate.charAt(j) == candidate.charAt(j+1)){
          j++;
          rpts++;
        }
      }
      //end loop: either candidate or input-sequence index reached end. At most, one of these includes remaining characters.

      // ...so here we account for differences in string length
      //The natural metric is to continue summing distance for remaining characters in either string.
      //Only one or neither of these loops will execute, since either (or both) i or j is at end of its sequence.
      while(i < pointMeans.size()){
        sumDist +=  Point.DoubleDistance(pointMeans.get(i).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j-1)));
        i++;
      }
      while(j < candidate.length()){
        sumDist +=  Point.DoubleDistance(pointMeans.get(i-1).GetPoint(), m_keyMapRef.GetPoint(candidate.charAt(j)));
        while(j < (candidate.length()-1) && candidate.charAt(j) == candidate.charAt(j+1)){  //chew up repeated chars in candidate. We assume there aren't any in the input sequence, hence we don't do this in the previous loop
          j++;
          rpts++;
        }
        j++;
      }

      //finally, add some small punishment to repeats
      sumDist += ((double)rpts * m_keyMapRef.GetMinInterKeyRadius() * 0.15); //token punishment to separate FOXX from FOX, DOGG from DOG

      return sumDist;
    }

    /*
      Insertion errors occur when the user accidentally selects a key unintentionally. Observe that most of the time
      these errors will occur nearby the general character sequence. A typical characteristic is probably that such
      error occurs between ("on the way to") two points, but slightly off center. This function therefore smooths
      the distance assigned to this kind of error by defining it as the distance from the error-character to the midpoint 
      between the previous and next points in the candidate string. This makes sense, again, since the insertion errors
      will tend to occur near this midpoint, while very bad errors will tend to be much further away, giving them a
      bad error (distance) value, which is desirable. You kind of have to model this geometrically for it to make sense;
      it seems to obey the triangle inequality, so maybe this implies a metric that could be used elsewhere.
    */
    private double InsertionError(Point errorPt, Point pt1, Point pt2)
    {
      double correctionWeight;

      //midpoint correction likely over-corrects, punishing correct strings.
      //correctionWeight = MidPointCorrection(errorPt, pt1, pt2);
      //nearest point correction returns punishment for error as distance to nearest adjacent correct alpha, since most edit errors occur near a correct neighbor alpha
      correctionWeight = NearestPointCorrection(errorPt, pt1, pt2);

      return correctionWeight;
    }
    /*
      Returns error-correction approximation as the dist to the nearest of two neighbor points.
      That is, is E is an insertion-error in input sequence Beta, then return the dist to the nearest of
      its correct neighbors.  Examples are Beta="FOCX" for which E='C' with respect to the candidate string "FOX".
      Clearly for this case we don't want simply the midpoint, since it will punish the candidate score too much.
      The intuition here is that most insertion errors will occur adjacent to one of the neighbors in the candidate.
    */
    private double NearestPointCorrection(Point errorPt, Point pt1, Point pt2)
    {
      double dist1, dist2, ret;

      dist1 = Point.DoubleDistance(errorPt,pt1);
      dist2 = Point.DoubleDistance(errorPt,pt2);

      if(dist1 < dist2){
        ret = dist1;
      }
      else{
        ret = dist2;
      }

      return ret;
    }
  } // end DirectInference class

  //Constructs test input data from some input file containing a raw stream (X,Y) coordinates, one coordinate per line.
  //This function checks some of the file formatting, but also strongly assumes it is valid.
  //Output is stored in the pointSequence.
  public void BuildTestData(String testFilePath, ArrayList<Point> pointSequence)
  {
    BufferedReader br = new BufferedReader(new FileReader(testFilePath));
    String line;
    String[] tokens;

    //error-check: keymap must be populated with key-val entries in order to set coordinates for the existing entries
    if(pointSequence.isEmpty()){
      System.out.println("Clearing previous test input point list...");
      pointSequence.clear();
    }

    while((line = br.readLine()) != null){
      tokens = line.trim().split("\t");
      if(tokens.length != 2){
        System.out.println("ERROR incorrect number of tokens for this line >"+line+"< in test input file "+testFilePath);
      }
      else if(!isIntString(tokens[0]) || !isIntString(tokens[1])){
        System.out.println("ERROR non-integer tokens for this line >"+line+"< in test input file "+testFilePath);
      }
      else{
        try{
          int x = Integer.parseInt(tokens[0]);
          int y = Integer.parseInt(tokens[1]);
          Point pt = new Point(x,y);
          pointSequence.add(pt);
        }
        catch(NumberFormatException e){
          System.out.println("Number format exception >"+e.getCause().getMessage()+"< caught for test input file >"+testFilePath+"<  Coordinates incorrectly formatted");
        }
      }
    }

    br.close();
  }

  //Driver for consuming a file of raw input points, converting them to means, and finally running direct-inference method on those means.
  public void TestWordStream(String testFilePath)
  {
    ArrayList<Point> testPoints = new ArrayList<Point>();
    ArrayList<PointMu> testMeans = new ArrayList<PointMu>();
    ArrayList<SearchResult> results = new ArrayList<SearchResult>();

    BuildTestData(testFilePath,testPoints);
    //SignalProcessor.Process(testPoints,testMeans);
    signalProcessor.DummyProcess(testPoints,testMeans);
    directInference.Process(testMeans,results);

    System.out.println("Results:");
    for(SearchResult result : results){
      result.toString();
    }
  }


  /*
    Main test driver, given an input file containing a stream of (x,y) points generated from a sample run (a word)
    The test file dir, file names, and count are hard-coded. This is just for testing. But you can
    run this function without the hardware to test different input parameters to the signal processing, different inference
    logic, etc.
    TODO: send the tests with a read only mirror directory of the test inputs.

    This is a "performance test" only in terms of how it is currently being used. The source dir contains actual inputs from
    EyeTribe hardware, so you can consume these to perform any kind of tests you want. Don't get hyperactive about formal software testing,
    as the hardware itself may change, settings (eg, Hz), or different hardware may be used all together. I only focused on
    "things don't crash" and then testing to find the approximately-best parameters for signal processing and then direct inference.
  */
  public void PerformanceTest()
  {
    int i;
    String sourceDir = "../TestInput/Test1/";
    String testFilePrefix = "test";
    String testFileSuffix = ".txt";

    for(i = 1; i <= 12; i++){
      String fname = new String(sourceDir+testFilePrefix+Integer.toString(i)+testFileSuffix);
      System.out.println("Next test file: "+fname);
      TestWordStream(fname);
    }
    System.out.println("Performance test complete, no more test files.");
  }


  //some outermost/class-wide utilities
  //got tired of Java's poor documentation and wrote this instead of using IsLetter()
  public static boolean IsAlpha(char c){
    boolean ret = false;

    if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')){
      ret = true;
    }

    return ret;
  }
  //returns whether or not a string contains only integers or +/-. These functions aren't exactly industrial grade. "23+3232-2323+" returns true.
  public static boolean isIntString(String str){
    String intString = "1234567890-+";

    for(int i = 0; i < str.length(); i++){
      if(intString.indexOf(str.charAt(i)) == -1){
        return false;
      }
    }

    return true;
  }
  //returns the absolute value of the difference between two integers
  public static int AbsDiff(int m, int n){
    int ret;

    if(n > m){
      ret = n - m;
    }
    else{
      ret = m - n;
    }

    return ret;
  }

  KeyMap keyMap;
  SignalProcessor signalProcessor;
  DirectInference directInference;

  //dont use the default constructor, as it hardcodes the keymap file path
  public FastMode(){
    String keyFilePath = "../resources/keyMap.txt";
    System.out.println("Default FastMode constructor called. Ctor taking keymap file path parameter is perferred.");
    System.out.println("Attempting to build keymap with hardcoded path: "+keyFilePath);
    keyMap = new KeyMap(keyFilePath);
    signalProcessor = new SignalProcessor();
    directInference = new DirectInference();
  }
  public FastMode(String keyFilePath){

    if(PathExists(keyFilePath)){
      keyMap = new KeyMap(keyFilePath);
      signalProcessor = new SignalProcessor(keyMap,14,16,4);
      directInference = new DirectInference("../resources/languageModel/vocab.txt",keyMap);
    }
    else{
      System.out.println("ERROR key file path >"+keyFilePath+"< not found! FastMode construction failed");
    }
  }

  //java api doesn't provide a nice File.exists() function, so I made this wrapper. Note "File" here is an object;
  // a Java "File" is not an actual file on disk, so this isn't as ineffiicient as it looks, for a simple existence check.
  //This works for either dirs or files.
  public static boolean PathExists(String path){
    boolean ret = false;

    try{
      File f = new File(path);
      ret = f.exists();
    }
    catch(NullPointerException ex){
      //nothing to do, just return false
    }

    return ret;
  }

  public static void main(String[] args){

    System.out.println("Args:");  
    for(int i = 0; i < args.length; i++){
      System.out.println(Integer.toString(i)+": "+args[i]);     
    }

    //TODO: striaghten out object input parameters so this is easy to use for clientss
    if(args.length != 1){
      System.out.println("ERROR incorrect number of parameters. cmd line usage:  ./FastMode ../resources");
    }
    else{
      if(!PathExists(args[0])){
        System.out.println("ERROR parameter >"+args[0]+"< does not exist. cmd line usage:  ./FastMode ../resources");
      }
      else if(PathExists(args[0])){
        System.out.println("Building FastMode...");
        FastMode fastMode = new FastMode(args[0]);;
        fastMode.PerformanceTest(); 
      }
    }
  }
}  // end FastMode class





/*
TODO: delete me
public class Student implements Comparable<Student> {

    String name;
    int age;

    public Student(String name, int age) {
       this.name = name;
       this.age = age;
    }

    @Override
    public String toString() {
        return name + ":" + age;
    }

    @Override
    public int compareTo(Student o) {
        return Comparators.NAME.compare(this, o);
    }


    public static class Comparators {

        public static Comparator<Student> NAME = new Comparator<Student>() {
            @Override
            public int compare(Student o1, Student o2) {
                return o1.name.compareTo(o2.name);
            }
        };
        public static Comparator<Student> AGE = new Comparator<Student>() {
            @Override
            public int compare(Student o1, Student o2) {
                return o1.age - o2.age;
            }
        };
        public static Comparator<Student> NAMEANDAGE = new Comparator<Student>() {
            @Override
            public int compare(Student o1, Student o2) {
                int i = o1.name.compareTo(o2.name);
                if (i == 0) {
                    i = o1.age - o2.age;
                }
                return i;
            }
        };
    }
}
*/
