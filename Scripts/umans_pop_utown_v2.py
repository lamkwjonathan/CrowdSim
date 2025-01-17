## Populate Rooms for UMAN ##
import random

def pop_room():
    rad_min = 0.215
    rad_max = 0.265
    spd_min = 1.2
    spd_max = 1.6
    policy_id = 0
    max_agents = 3000
    threshold = 0.75
    threshold_2 = 0.7

    goals = [["40.5", "69.2"],["80.9", "66.3"], ["61.8", "11.5"], ["-58.6", "26.9"]]
    counter = 0
    
    f = open("UTownGreen3000.xml", "w")
    f.write("<Agents>\n")

    ## Top Block
    for j in range (30, -10, -1):
        for i in range (-30, -13, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = 3
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    ### buffer
    for j in range (30, -10, -1):
        for i in range (-13, -7, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            divider = random.randrange(0,2)
            if divider == 0:
                goal_no = 3
            else:
                goal_no = random.randrange(0,3)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    for j in range (30, 7, -1):
        for i in range (-7, 30, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = random.randrange(0,2)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    for j in range (7, 3, -1):
        for i in range (-7, 30, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            divider = random.randrange(0,2)
            if divider == 0:
                goal_no = 2
            else:
                goal_no = random.randrange(0,2)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    for j in range (3, -10, -1):
        for i in range (-7, 30, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = 2
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    ## Bottom Block
    for j in range (-15, -35, -1):
        for i in range (-40, -32, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold_2:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = 3
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    ### buffer
    for j in range (-15, -35, -1):
        for i in range (-32, -28, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            divider = random.randrange(0,2)
            if divider == 0:
                goal_no = 3
            else:
                goal_no = random.randrange(0,3)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    for j in range (-15, -25, -1):
        for i in range (-28, -10, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold_2:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = random.randrange(0,2)
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break

    for j in range (-25, -35, -1):
        for i in range (-28, -10, 1):
            if counter >= max_agents:
                break
            rad = (rad_max - rad_min)*random.random() + rad_min
            spd = (spd_max - spd_min)*random.random() + spd_min
            f.write("<Agent rad=\"" + str(rad) + "\" pref_speed=\"" + str(spd) + "\" max_speed=\"1.8\" remove_at_goal=\"true\">\n")
            f.write("<pos x=\"" + str(i+0.5) + "\" y=\"" + str(j-0.5) + "\"/>\n")
            deviance = random.random()
            if deviance > threshold_2:
                goal_no = random.randrange(0,len(goals))
            else:
                goal_no = 2
            f.write("<goal x=\"" + goals[goal_no][0] + "\" y=\"" + goals[goal_no][1] + "\"/>\n")
            f.write("<Policy id=\"" + str(policy_id) + "\"/>\n")
            f.write("</Agent>\n")
            counter += 1
        if counter >= max_agents:
            break
        
    f.write("</Agents>")
    return

pop_room()
