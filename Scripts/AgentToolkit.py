## Agent Toolkit ##

import math
import random
import numpy as np

def rotation_matrix(theta):
    theta_rad = np.radians(theta)
    return np.array([
        [np.cos(theta_rad), -np.sin(theta_rad)],
        [np.sin(theta_rad),  np.cos(theta_rad)]
    ])

def populate_rect(x, y, width, height, num_rows, goals, num_agents, use_random, offset, theta, file_name):
    agents_per_row = math.ceil(num_agents / num_rows)
    col_dist = width / agents_per_row
    row_dist = height / num_rows

    count = 0

    R = rotation_matrix(theta)
    
    f = open(file_name, "a")
    
    for j in np.arange(y - offset[1], y - offset[1] - height, -row_dist):
        if count >= num_agents:
            break
        for i in np.arange(x - offset[0], x - offset[0] + width, col_dist):
            random_rad = random.uniform(0.24-0.025, 0.24+0.025)
            if (use_random):
                random_x = random.uniform(i - col_dist/2 + random_rad, i + col_dist/2 - random_rad)
                random_y = random.uniform(j - row_dist/2 + random_rad, j + row_dist/2 - random_rad)
            else:
                random_x = i
                random_y = j

            point = np.array([random_x - (x - offset[0]), random_y - (y - offset[1])])
            rotated_point = R @ point
            rotated_point[0] = rotated_point[0] + (x - offset[0])
            rotated_point[1] = rotated_point[1] + (y - offset[1])
            
            min_dist = 100000000
            goal = [0, 0]
            for g in goals:
                temp_dist = math.sqrt(pow((g[0] - rotated_point[0]), 2) + pow((g[1] - rotated_point[1]), 2))
                if temp_dist < min_dist:
                    min_dist = temp_dist
                    goal = g
                    
            f.write("<Agent rad=\"" + str(random_rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(rotated_point[0]) + "\" y=\"" + str(rotated_point[1]) + "\"/>\n")
            f.write("<goal x=\"" + str(goal[0]) + "\" y=\"" + str(goal[1]) + "\"/>\n")
            f.write("<Policy id=\"" + str(0) + "\"/>\n")
            f.write("</Agent>\n")
            count += 1
    return
            
        
