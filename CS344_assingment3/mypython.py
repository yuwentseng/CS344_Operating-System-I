
#Name: Yu-Wen, Tseng
#CS344 python asignment

import random, string #Library for string and getting the random number

Filename = [ "HomerSimpson", "MargeSimpson", "BartSimpson"] # create 3 files in the same directory

#Set the function to make random alphabet
def Randomalphabet():
    alphabet = "" #create  a empty list
    randomalpha = alphabet.join(random.choice(string.ascii_letters)for x in range(10)).lower() #make randomly characters from the lowercase alphabet
    print(randomalpha)
    return randomalpha

#write text in the each 3 files
for i in range(3):
    file = open(Filename[i],'w+')
    file.write(Randomalphabet())

#Randomly run the 1~42 use the random.randint()
number1 = random.randint(1,42)
number2 = random.randint(1,42)
print(number1)
print(number2)
print(number1*number2) #Get product of the two random numbers

