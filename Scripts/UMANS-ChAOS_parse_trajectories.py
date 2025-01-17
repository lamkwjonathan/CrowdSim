### UMANS-ChAOS Parse Trajectories ###

import os

def clean(directory):
    for filename in os.scandir(directory):
        if filename.is_file():
            f_list = []
            f = open(filename, "r")
            for line in f:
                parsed_line = line.split(",")
                f_list.append(parsed_line[0] + "," + parsed_line[1] + "," + parsed_line[2] + '\n')
            f.close()
            f = open(filename, 'w')
            for item in f_list:
                f.write(item)
            f.close()
            

clean(r"C:\Users\imjon\Documents\School\FYP\CrowdSim\ChAOS\Scenarios\UTownGreenEvacuationPerfTest")
