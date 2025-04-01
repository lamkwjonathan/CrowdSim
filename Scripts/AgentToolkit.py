## Agent Toolkit ##

import math
import random
import numpy as np

def populate_rect(x, y, width, height, num_rows, goals, num_agents, file_name):
    agents_per_row = math.ceil(num_agents / num_rows)
    col_dist = width / agents_per_row
    row_dist = height / num_rows

    count = 0
    
    f = open(file_name, "a")
    
    for j in np.arange(y, y - height, -row_dist):
        if count >= num_agents:
            break
        for i in np.arange(x, x + width, col_dist):
            random_rad = random.uniform(0.24-0.025, 0.24+0.025)
            min_dist = 100000000
            goal = [0, 0]
            for g in goals:
                temp_dist = math.sqrt(pow((g[0] - i), 2) + pow((g[1] - j), 2))
                if temp_dist < min_dist:
                    min_dist = temp_dist
                    goal = g
                    
            f.write("<Agent rad=\"" + str(random_rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i) + "\" y=\"" + str(j) + "\"/>\n")
            f.write("<goal x=\"" + str(goal[0]) + "\" y=\"" + str(goal[1]) + "\"/>\n")
            f.write("<Policy id=\"" + str(0) + "\"/>\n")
            f.write("</Agent>\n")
            count += 1
    return
            
        
