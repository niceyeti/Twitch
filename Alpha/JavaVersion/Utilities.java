import java.io.*;


//Holds a bunch of random, global utilities, called by multiple sub-classes
public class Utilities{
  private Utilities(){
    //nada, this is a static class
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
  public static boolean IsIntString(String str){
    String intString = "1234567890-+";

    for(int i = 0; i < str.length(); i++){
      if(intString.indexOf(str.charAt(i)) == -1){
        //System.out.println("Char not int: "+str.charAt(i));
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
}

