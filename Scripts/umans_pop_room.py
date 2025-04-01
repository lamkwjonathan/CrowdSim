## Populate Rooms for UMAN ##

import random

def pop_rooms(rad, policy_id, x_min, x_max, y_min, y_max):
    f = open("Room400_v3.xml", "w")
    f.write("<Agents>\n")
    for i in range (x_min, x_max, 1):
        for j in range (y_min, y_max, 1):
            random_rad = random.uniform(rad-0.025, rad+0.025)
            f.write("<Agent rad=\"" + str(random_rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j+0.5) + "\"/>\n")
            f.write("<goal x=\"12.0\" y=\"0.0\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
    f.write("</Agents>")
    return

pop_rooms(0.24, 0, -10, 10, -10, 10)
