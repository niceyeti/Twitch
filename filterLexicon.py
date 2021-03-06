'''
COCA lexicon is full of garbage words like roman numerals and other junk. This attempts to eliminate 
some of that garbage. A lot of this has to be done manually, especially due to the abundance of names
and stuff like DOOG, DOGG, and so on, instead of just DOG.
'''

def RomanNumeral(string):
  for c in string:
    if c not in "XCIV.":
      return False
  return True

#verify string of length >= 3 is not just some acronym or repeated character
def NotRepeater(string):
  i = len(string)
  if i < 2:
    return True
  else:
    i = 0
    while i < (len(string) - 1):
      if string[i] != string[i+1]:
        return True
      i += 1
  return False

#no puncuated words, only whole words
def HasPunct(string):
  for c in string:
    if c in " 1234567890&*()[]|.?!-+_:','\"\\/;$!":
      return True
  return False

def ValidOneChar(string):
  if len(string.strip()) != 1:
    return True
  elif string.strip() not in ["a","i"]:
    return False
  return True


#verify two character string is a valid word, not a random combination, of which there are plenty
def ValidTwoChar(string):
  if len(string) != 2:
    return True
  else:
    #two char string, so check if it is in the valid set of two-char words
    if string in ["an","is","we","ac","dr","tv","cd","be","by","mr","ms","ok","jr","pc","so","me","to","it","he","ay","ya","as","at","do","ef","ex","go","he","if","in","ma","my","no","ow","of","oh","oi","on","oo","or","os","oy","pa","to","ew","mm","me","uh","up","ur","us","ya","ax"]:
      return True
  return False

#for removing strings that are all consonants
def HasVowel(word):
  for c in word:
    if c in "aeiouy":
      return True
  return False

#for removing all-vowel strings
def HasConsonant(word):
  for c in word:
    if c not in "aeiou":
      return True
  return False

def IsValid(word):
  if not HasPunct(word):
    if ValidTwoChar(word):
      if ValidOneChar(word):
        if NotRepeater(word):
          if not RomanNumeral(word):
            #verify longer strings have a mixture of vowels and consonants
            if len(word) < 3 or (HasConsonant(word) and HasVowel(word)):
              return True
  return False


import math


ifile = open("./w2_.txt","r")
lines = ifile.readlines()
ifile.close()
output = set()
shortWords = {}

#short words will be filtered by probability
for line in lines:
  toks = line.lower().strip().split("\t")
  if len(toks) >= 3:
    if IsValid(toks[1]):
      output.add(toks[1])
    else:
      print "invalid: "+toks[1]
    if IsValid(toks[2]):
      output.add(toks[2])
    else:
      print "invalid: "+toks[2]

    if len(toks[1]) < 4:
      if shortWords.get(toks[1],-1) > -1:
        shortWords[toks[1]] += float(toks[0])
      else:
        shortWords[toks[1]] = float(toks[0])
      #shortWords.append(toks[1])
    """
    if len(toks[2]) <= 5:
      if shortWords.get(toks[2],-1) > -1:
        shortWords[toks[2]] += float(toks[0])
      else:
        shortWords[toks[2]] = float(toks[0])
      #shortWords.append(toks[2])
    """






ofile = open("./vocabModel.txt","w+")
for word in output:
  ofile.write(word+"\n")
ofile.close()


#short word analysis
total = 0.0
for word in shortWords.keys():
  total += shortWords[word]
#set each word to a base-two log-probability
for word in shortWords.keys():
  shortWords[word] = -1.0 * math.log(shortWords[word] / total,2.0)

ofile = open("./validShort.txt","w+")
ofile2 = open("./invalidShort.txt","w+")
ofile3 = open("./checkThese.txt","w+")
for word in shortWords.keys():
  if IsValid(word):
    if shortWords[word] < 22.00:  #probability threshold; manualy verify the culled words
      #print word
      ofile.write(word+"\t"+str(shortWords[word])+"\n")
    else:
      ofile3.write(word+"\t"+str(shortWords[word])+"\n")
  else:
    ofile2.write(word+"\n")
ofile.close()
ofile2.close()
ofile3.close()


ifile = open("./invalidShort.txt","r")
invalidWords = ifile.readlines()
lines = open("./checkThese.txt","r").readlines()
#extend invalid list of words
for line in lines:
  tokens = line.strip().split("\t")
  if len(tokens) >= 2:
    invalidWords.append(tokens[0])

ofile = open("./validShort.txt","r")
lines = ofile.readlines()
final = open("./vocab.txt","w+")

outlines = []
for line in lines:
  tokens = line.split("\t")
  if (len(tokens) >= 2) and (tokens[0] not in invalidWords):
    outlines.append(tokens[0])

for line in outlines:
  final.write(line+"\n")

ifile.close()
ofile.close()
final.close()







