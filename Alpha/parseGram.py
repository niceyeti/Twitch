import math 

ifile = open("./unigrams_RO.txt","r")
ofile = open("./unigrams.txt","w")

gramDict = {}
bigramNormals = {}
trigramNormals = {}
quadgramNormals = {}

normal = 0.0
lines = ifile.readlines()
for line in lines:
  gram = line.split("\t")[0].lower()
  pgram = float(line.split("\t")[1])
  #print "len=",len(gram)

  if len(gram) == 2:
    if bigramNormals.get(gram[0],-1) == -1:
      bigramNormals[gram[0]] = pgram
    else:
      bigramNormals[gram[0]] += pgram

  if len(gram) == 3:
    if trigramNormals.get(gram[0]+gram[1],-1) == -1:
      trigramNormals[gram[0]+gram[1]] = pgram
    else:
      trigramNormals[gram[0]+gram[1]] += pgram

  if len(gram) == 4:
    if quadgramNormals.get(gram[0]+gram[1]+gram[2],-1) == -1:
      quadgramNormals[gram[0]+gram[1]+gram[2]] = pgram
    else:
      quadgramNormals[gram[0]+gram[1]+gram[2]] += pgram



  gramDict[gram] = float(pgram)
  
  normal += float(pgram)

print "normalizing..."
for key in gramDict.keys():

  if len(key) == 1:
    gramDict[key] = (-1.0) * math.log(gramDict[key] / normal, 2.0)  #init table to log probabilities 
    output = key + "\t" + str(gramDict[key]) + "\n"
    ofile.write(output)

  #if 2-gram model, get the normal over the previous characer: P(a|b) = p(a^b)/p(b) = nAB / nB
  if len(key) == 2:
    gramDict[key] = (-1.0) * math.log(gramDict[key] / bigramNormals[key[0]], 2.0)  #init table to log probabilities 
    output = key + "\t" + str(gramDict[key]) + "\n"
    ofile.write(output)
  
  if len(key) == 3:
    gramDict[key] = (-1.0) * math.log(gramDict[key] / trigramNormals[key[0]+key[1]], 2.0)  #init table to log probabilities 
    output = key + "\t" + str(gramDict[key]) + "\n"
    ofile.write(output)

  if len(key) == 4:
    gramDict[key] = (-1.0) * math.log(gramDict[key] / quadgramNormals[key[0]+key[1]+key[2]], 2.0)  #init table to log probabilities 
    output = key + "\t" + str(gramDict[key]) + "\n"
    ofile.write(output)


print gramDict

ofile.close()
ifile.close()


















