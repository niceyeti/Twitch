import math

ifile = open("unigrams.txt","r")
ofile = open("unigrams_OUT.txt","w+")
lines = ifile.readlines()
model = {}

for line in lines:
  tokens = line.strip().split(" ")
  if len(tokens) == 2:
    model[tokens[0]] = float(tokens[1])

s = 0.0
for key in model.keys():
  s += model[key]

i = 0
for key in model.keys():
  model[key] = -1.0 * math.log(model[key] / s, 2.0)
  ofile.write(key+"\t"+str(model[key])+"\n")
  i += 1

print "wrote: "+str(i)
ifile.close()
ofile.close()



