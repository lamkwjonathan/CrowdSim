## Test Scenario for UMAN ##

import random

def pop_crossing_area(rad, policy_id):
    f = open("Crossing.xml", "w")
    f.write("<Agents>\n")
    for i in range (-20, -0, 1):
        for j in range (-10, 10, 1):
            random_rad = random.uniform(rad-0.025, rad+0.025)
            f.write("<Agent rad=\"" + str(random_rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j+0.5) + "\"/>\n")
            f.write("<goal x=\"5.0\" y=\"0.0\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("<color r=\"255\" g=\"0\" b=\"0\" />\n")
            f.write("</Agent>\n")

    for i in range (0, 20, 1):
        for j in range (-10, 10, 1):
            random_rad = random.uniform(rad-0.025, rad+0.025)
            f.write("<Agent rad=\"" + str(random_rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j+0.5) + "\"/>\n")
            f.write("<goal x=\"-5.0\" y=\"0.0\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("<color r=\"0\" g=\"0\" b=\"255\" />\n")
            f.write("</Agent>\n")
    f.write("</Agents>")
    return

pop_crossing_area(0.24, 0)
