import math
from BFS_python import *

bot_radius = 1

#FILL IN GRID WITH 0s AND 1s DEPENDING ON RADIUS OF GRID
def create_circle():
  for i in range(0, width):
      temp_array = []
      for j in range(0, length):
          temp_array.append(1)
      grid.append(temp_array)
  #print_grid()

#PRINT THE GRID
def print_grid():
    for i in range(len(grid)):
        temp_array = grid[i]
        for j in range(len(temp_array)):
            print(" " + str(temp_array[j]), end = ' ')
        print('\n')

#FILL IN OBSTACLES
def fill_obstacles():
    for i in range(len(obstacles)):
        temp_obstacles = obstacles[i]
        x = temp_obstacles[0]
        y = temp_obstacles[1]
        l = temp_obstacles[2]
        w = temp_obstacles[3]

        #print("x: " + str(x) + ", y: " + str(y) + ", w: " + str(w) + ", l: " + str(l))

        #IF W = 1 AND L = 1
        if (w == 1 and l == 1):
            grid_x = x
            #grid_y = 2*radius - y
            grid_y = y
            #print("grid x: " + str(grid_x) + ", grid y: " + str(grid_y))

            if (x >= 0 and y >= 0):
                if (grid[grid_x][grid_y] == 1):
                    grid[grid_x][grid_y] = 0

        #ANYTHING ELSE
        elif (w > 0 and l > 0):
            #print("x: " + str(x) + ", " + str(x+w))
            #print("y: " + str(y) + ", " + str(y+l))
            for j in range(x, x+w):
                for k in range(y, y+l):
                    #print("j: " + str(j) + ", k: " + str(k))
                    grid_x = j
                    grid_y = k
                    if (j >= 0 and k >= 0):
                        if grid_x < width and grid_y < length and grid_x >= 0 and grid_y >= 0:
                            if (grid[grid_x][grid_y] == 1):
                                grid[grid_x][grid_y] = 0


def check_collisions(path):
    for location in path:
        x = location[0]
        y = location[1]
        for i in range(x, x+robot_width):
            for j in range(y, y+robot_length):
                if (grid[i][j] == 0): #collision
                    return True
    return False

def closest_V_target(V, target_x, target_y, grid, length, width):
    best = None
    for location in V:
        x = location[0]
        y = location[1]
        #data = path(x,y,target_x,target_y, grid, [], length, width)
        data = path(x,y,target_x,target_y,grid,[],length,width)
        if best == None:
            best = (location, data[2])
        elif data[2] < best[1]:
            best = (location, data[2])
    return best[0]

#circle parameters

length = 10
width = 10
grid = []

# We are assuming the locations given are the robot's bottom left corner
robot_length = 2
robot_width = 1
create_circle()

#obstacles: (middle_x, middle_y, length, width)
obstacle1 = [3, 3, 3, 1]
obstacle2 = [8, 2, 2, 2]
obstacle3 = [4, 3, 2, 2]
obstacle4 = [24, 12, 2, 2]
obstacles = []
obstacles.append(obstacle1)
obstacles.append(obstacle2)
#obstacles.append(obstacle3)
obstacles.append(obstacle4)
fill_obstacles()

print_grid()
data = path(5,1,7,6,grid,[],length,width)
dist = data[2]
directions = data[3]
path_taken = data[4]
print(dist)
print(directions)
print(path_taken)
#print(check_collisions(path))
V = [[1,1], [2,5], [3,2]]
answer = closest_V_target(V,4,7,grid,length,width)
print(answer)
