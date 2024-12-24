## Populate Rooms for UMAN ##
import random

def pop_rooms(rad_min, rad_max, policy_id, x_min, x_max, y_min, y_max, max_agents):
    goals = [["-102.0", "-50.0"],["-50.0", "-102.0"], ["50.0", "-102.0"], ["102.0", "-50.0"]]
    counter = 0
    f = open("LargeRoom20000.xml", "w")
    f.write("<Agents>\n")
    for j in range (y_max, y_min, -1):
        for i in range (x_min, x_max, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"1.4\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            goal_no = random.randrange(0,4)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break
    f.write("</Agents>")
    return

pop_rooms(0.215, 0.265, 0, -100, 100, -100, 100, 20000)
