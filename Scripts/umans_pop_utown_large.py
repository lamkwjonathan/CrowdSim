## Populate Rooms for UMAN ##
import random

def pop_room():
    rad_min = 0.215
    rad_max = 0.265
    spd_min = 1.2
    spd_max = 1.6
    policy_id = 0
    max_agents = 100000
    threshold = 0.7
    threshold_2 = 0.6

    goals = [["40.5", "69.2"],["80.9", "66.3"], ["61.8", "11.5"], ["-58.6", "26.9"]]
    counter = 0
    
    f = open("UTownGreen100000.xml", "w")
    f.write("<Agents>\n")

    ## Central Block (4250)
    for j in range (40, -10, -1):
        for i in range (-45, 40, 1):
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

    ## Bottom Block 1 (1500)
    for j in range (-10, -30, -1):
        for i in range (-50, 25, 1):
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
    
    ## Bottom Block 2 (1200)
    for j in range (-30, -50, -1):
        for i in range (-55, 5, 1):
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

    ## Bottom Block 3 (1100)
    for j in range (-50, -70, -1):
        for i in range (-60, -5, 1):
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

    ## Bottom Block 4 (500)
    for j in range (-70, -80, -1):
        for i in range (-65, -15, 1):
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

    ## Top Block 1 (850)
    for j in range (50, 40, -1):
        for i in range (-10, 75, 1):
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
        
    ## Top Block 2 (550)
    for j in range (60, 50, -1):
        for i in range (25, 80, 1):
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

    ## Top Block 3 (750)
    for j in range (40, 10, -1):
        for i in range (40, 65, 1):
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

    ### OUTSIDE ###

    ## Left Block 1 (10000)
    for j in range (50, -50, -1):
        for i in range (-200, -100, 1):
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

    ## Left Block 2 (10000)
    for j in range (-50, -150, -1):
        for i in range (-210, -110, 1):
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

    ## Left Left Block 1 (10000)
    for j in range (50, -50, -1):
        for i in range (-300, -200, 1):
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

    ## Left Left Block 2 (10000)
    for j in range (-50, -150, -1):
        for i in range (-310, -210, 1):
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

    ## Right Block 1 (10000)
    for j in range (-20, -120, -1):
        for i in range (80, 180, 1):
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

    ## Right Block 2 (10000)
    for j in range (-120, -220, -1):
        for i in range (80, 180, 1):
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

    ## Right Right Block 1 (10000)
    for j in range (-20, -120, -1):
        for i in range (180, 280, 1):
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

    ## Right Right Block 2 (10000)
    for j in range (-120, -220, -1):
        for i in range (180, 280, 1):
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

    ## Bottom Bottom Block (10000)
    for j in range (-150, -250, -1):
        for i in range (-50, 50, 1):
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
    print(counter)
    return

pop_room()
