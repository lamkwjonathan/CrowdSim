## Populate Rooms for UMAN ##
import random

def pop_rooms(rad_min, rad_max, spd_min, spd_max, policy_id, x_min, x_max, y_min, y_max, x_min_2, x_max_2, y_min_2, y_max_2, max_agents):
    goals = [["40.5", "69.2"],["80.9", "66.3"], ["61.8", "11.5"], ["-58.6", "26.9"]]
    counter = 0
    f = open("UTownGreen3000.xml", "w")
    f.write("<Agents>\n")
    for j in range (y_max, y_min, -1):
        for i in range (x_min, x_max, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            goal_no = random.randrange(0,len(goals))
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break
        
    for j in range (y_max_2, y_min_2, -1):
        for i in range (x_min_2, x_max_2, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            goal_no = random.randrange(0,len(goals))
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break
    f.write("</Agents>")
    return

pop_rooms(0.215, 0.265, 1.2, 1.6, 0, -30, 30, -10, 30, -40, -10, -35, -15, 3000)
