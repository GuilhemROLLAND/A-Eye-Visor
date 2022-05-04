fIn = open("./sauve/image_preprocessed.json", "r")
fOut = open("./sauve/image_preprocessed_format.json", "w")

idx = 0
fOut.write("[\n")
fOut.write("[\n")
fOut.write("[\n")
fOut.write("[\n")

for line in fIn:
    idx += 1
    fOut.write(line)
    if(idx % 3 == 0):
        fOut.write("],\n")
        if(idx % 224 == 0):
            fOut.write("],\n")
            if(idx % 224 == 0):
                fOut.write("],\n")
                fOut.write("[\n")
            fOut.write("[\n")
        fOut.write("[\n")
fOut.write("],\n")
fOut.write("],\n")
fOut.write("],\n")
fOut.write("]\n")


fIn.close()
fOut.close()
