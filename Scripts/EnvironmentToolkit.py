import geojson
import pygame
import math
from pygame.locals import *
import xml.etree.ElementTree as ET

def create_xml_from_geojson(geojson_filename, obstacles_filename, border_size):
    with open(geojson_filename) as file:
       data = geojson.load(file)
    
    f = open(obstacles_filename, 'w')
    f.write("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
    
    x_min = 1000000000
    x_max = 0
    y_min = 1000000000
    y_max = 0
    for feature in data["features"]:
        geometry = feature["geometry"]
        for coord in geometry["coordinates"][0][0]:
            if coord[0] < x_min:
                x_min = coord[0]
            if coord[0] > x_max:
                x_max = coord[0]
            if coord[1] < y_min:
                y_min = coord[1]
            if coord[1] > y_max:
                y_max = coord[1]

    f.write("<Obstacles>\n")
    f.write("<Offset x=\"" + str(x_min - border_size) + "\" y=\"" + str(y_min - border_size) + "\"/>\n")
    f.write("<Dimension width=\"" + str(math.ceil(x_max - x_min + 1 + 2*border_size)) + "\" height=\"" + str(math.ceil(y_max - y_min + 1 + 2*border_size)) + "\"/>\n")
    
    for feature in data["features"]:
        geometry = feature["geometry"]
        f.write("<Obstacle>\n")
        for coord in geometry["coordinates"][0][0]:
            f.write("<Point x=\"" + str(coord[0]) + "\" y=\"" + str(coord[1]) + "\"/>\n")
        f.write("</Obstacle>\n")

    f.write("</Obstacles>")

def create_png_from_geojson(geojson_filename, obstacles_filename, border_size):
    with open(geojson_filename) as file:
       data = geojson.load(file)

    #Get canvas boundaries
    x_min = 1000000000
    x_max = 0
    y_min = 1000000000
    y_max = 0
    for feature in data["features"]:
        geometry = feature["geometry"]
        for coord in geometry["coordinates"][0][0]:
            if coord[0] < x_min:
                x_min = coord[0]
            if coord[0] > x_max:
                x_max = coord[0]
            if coord[1] < y_min:
                y_min = coord[1]
            if coord[1] > y_max:
                y_max = coord[1]

    width = math.ceil(x_max - x_min + 1 + 2*border_size)
    height = math.ceil(y_max - y_min + 1 + 2*border_size)
    pygame.init()

    canvas = pygame.display.set_mode((width,height))
    color_black = (0,0,0)
    color_white = (255,255,255)
    
    canvasOn = True

    while canvasOn:
        canvas.fill(color_white)
        for event in pygame.event.get():
            
            if event.type == QUIT:
                canvasOn = False
                
        for feature in data["features"]:
            geometry = feature["geometry"]
            points = []
            for coord in geometry["coordinates"][0][0]:
                x_coord = coord[0] - x_min + border_size
                y_coord = (height - 1) - (coord[1] - y_min + border_size) 
                points.append([x_coord, y_coord])
            pygame.draw.polygon(canvas, color_black, points)

        pygame.display.update()

    pygame.image.save(canvas, obstacles_filename)
    pygame.display.quit()
    pygame.quit()
    print("x_min = " + str(x_min))
    print("y_min = " + str(y_min))

def create_mask_png_from_geojson(obstacles_geojson_filename, border_size, mask_geojson_filename, mask_output_filename):
    with open(obstacles_geojson_filename) as file:
       data = geojson.load(file)

    #Get canvas boundaries
    x_min = 1000000000
    x_max = 0
    y_min = 1000000000
    y_max = 0
    for feature in data["features"]:
        geometry = feature["geometry"]
        for coord in geometry["coordinates"][0][0]:
            if coord[0] < x_min:
                x_min = coord[0]
            if coord[0] > x_max:
                x_max = coord[0]
            if coord[1] < y_min:
                y_min = coord[1]
            if coord[1] > y_max:
                y_max = coord[1]

    width = math.ceil(x_max - x_min + 1 + 2*border_size)
    height = math.ceil(y_max - y_min + 1 + 2*border_size)

    with open(mask_geojson_filename) as file:
        data = geojson.load(file)
    
    pygame.init()

    canvas = pygame.display.set_mode((width,height))
    color_black = (0,0,0)
    color_white = (255,255,255)
    
    canvasOn = True

    while canvasOn:
        canvas.fill(color_white)
        for event in pygame.event.get():
            
            if event.type == QUIT:
                canvasOn = False
                
        for feature in data["features"]:
            geometry = feature["geometry"]
            points = []
            for coord in geometry["coordinates"][0][0]:
                x_coord = coord[0] - x_min + border_size
                y_coord = (height - 1) - (coord[1] - y_min + border_size) 
                points.append([x_coord, y_coord])
            pygame.draw.polygon(canvas, color_black, points)

        pygame.display.update()

    pygame.image.save(canvas, mask_output_filename)
    pygame.display.quit()
    pygame.quit()

def create_png_from_xml(xml_filename, png_filename):
    tree = ET.parse(xml_filename)
    root = tree.getroot()
    
    x_min = float(root.find("Offset").get("x"))
    y_min = float(root.find("Offset").get("y"))
    width = int(root.find("Dimension").get("width"))
    height = int(root.find("Dimension").get("height"))

    pygame.init()

    canvas = pygame.display.set_mode((width,height))
    color_black = (0,0,0)
    color_white = (255,255,255)
    
    canvasOn = True

    while canvasOn:
        canvas.fill(color_white)
        for event in pygame.event.get():
            
            if event.type == QUIT:
                canvasOn = False
                
        for obstacle in root.findall("Obstacle"):
            points = []
            for point in obstacle.findall("Point"):
                x_coord = float(point.get("x")) - x_min
                y_coord = (height - 1) - (float(point.get("y")) - y_min)
                points.append([x_coord, y_coord])
            pygame.draw.polygon(canvas, color_black, points)

        pygame.display.update()

    pygame.image.save(canvas, png_filename)
    pygame.display.quit()
    pygame.quit()

def get_endpoints_coordinates(endpoint_geocoord_list, offset_x, offset_y):
    points = []
    for coord in endpoint_geocoord_list:
        x = coord[1] - offset_x
        y = coord[0] - offset_y
        points.append([x, y])
    print(points)
    return points
    

