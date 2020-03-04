import queue

def path(x0, y0, x1, y1, grid, visited, length, width):
    grid_x0 = x0
    grid_y0 = y0
    grid_x1 = x1
    grid_y1 = y1

    #handle visited
    for i in range(width):
        temp = []
        for j in range(length):
            temp.append(0)
        visited.append(temp)

    for i in range(width):
        temp = []
        for j in range(length):
            if (grid[i][j] == 0): #mark blocked cells as visited
                visited[i][j] = 1
            else:
                visited[i][j] = 0
    #print_visited()

    #BFS
    q = []
    temp_loc = (grid_x0, grid_y0, 0, [], [])
    q.append(temp_loc)
    visited[grid_x0][grid_y0] = 1
    while (q != []):
        temp = q.pop(0)
        x = temp[0]
        y = temp[1]
        d = temp[2]
        path_dir = temp[3]
        path_loc = temp[4]
        #print("x=" + str(x) + ", y=" + str(y) + ", d=" + str(d))

        #destination found
        if(x == grid_x1 and y == grid_y1):
            return temp

        #moving left
        if (x-1 >= 0 and visited[x-1][y] == 0):
            way = 'L'
            thing0=x-1
            thing1=y
            thing2=d+1
            thing3 = temp[3].copy()
            thing3.append(way)
            thing4 = temp[4].copy()
            thing4.append([thing0, thing1])
            temp0 = (thing0,thing1,thing2,thing3,thing4)
            q.append(temp0)
            visited[x-1][y] = 1

        #moving right
        if (x+1 < width and visited[x+1][y] == 0):
            way = 'R'
            thing0=x+1
            thing1=y
            thing2=d+1
            thing3 = temp[3].copy()
            thing3.append(way)
            thing4 = temp[4].copy()
            thing4.append([thing0, thing1])
            temp1 = (thing0,thing1,thing2,thing3,thing4)
            q.append(temp1)
            visited[x+1][y] = 1

        #moving up
        if (y+1 < length and visited[x][y+1] == 0):
            way = 'U'
            thing0=x
            thing1=y+1
            thing2=d+1
            thing3 = temp[3].copy()
            thing3.append(way)
            thing4 = temp[4].copy()
            thing4.append([thing0, thing1])
            temp2 = (thing0,thing1,thing2,thing3,thing4)
            q.append(temp2)
            visited[x][y+1] = 1

        #moving down
        if (y-1 >= 0 and visited[x][y-1] == 0):
            way = 'D'
            thing0=x
            thing1=y-1
            thing2=d+1
            thing3 = temp[3].copy()
            thing3.append(way)
            thing4 = temp[4].copy()
            thing4.append([thing0, thing1])
            temp3 = (thing0,thing1,thing2,thing3,thing4)
            q.append(temp3)
            visited[x][y-1] = 1
    #endwhile

    return -1   # no path found
'''
def print_visited():
    for i in range(4):
        for j in range(4):
            print(visited[i][j], end=' ')
        print('\n')

temp1 = [0, 1, 0, 1]
temp2 = [1, 0, 1, 1]
temp3 = [0, 1, 1, 1]
temp4 = [1, 1, 1, 1]
grid = []
grid.append(temp1)
grid.append(temp2)
grid.append(temp3)
grid.append(temp4)


visited = []
print(path(0, 3, 3, 0))
'''
