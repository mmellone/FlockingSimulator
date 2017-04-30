import sys
from random import randint
from math import sqrt
from numpy.random import normal as randCoords
from random import vonmisesvariate as randDir

def main():
    # USAGE:
    # generateBirds.py <number of centers> <size> <number of birds> <diameter of centers (size of std dev)> <output file name>
    try:
        numCenters = int(sys.argv[1])
        size = int(sys.argv[2])
        numBirds = int(sys.argv[3])
        diameter = size*0.05
        outputFileName = sys.argv[4]
    except:
        print "USAGE: generateBirds.py <number of centers> <size> <number of birds> <output file name>"
        return
    centers = []
    minDistBetweenCenters = int(size * 0.2)

    for c in range(numCenters):
        nextCenter = (randint(0, size), randint(0, size), randint(0, size))
        while distToClosestCenter(nextCenter, centers, size) < minDistBetweenCenters or closeToBoundary(nextCenter, size):
            nextCenter = (randint(0, size), randint(0, size), randint(0, size))
        centers.append(nextCenter)

    fout = open(outputFileName, "w")
    for c in centers:
        for b in range(numBirds/numCenters):
            # Create position coordinates according to a gaussian distribution around center c
            coords = map(int, tuple(randCoords(c, diameter)))
            map(lambda x: x % size, coords)
            direction = randDir(0,0)
            birdInfo = str(coords[0])+","+str(coords[1])+","+str(coords[2])

            # Create a random directional vector
            for i in range(3):
                birdInfo += "," + str(randint(-5, 5))
            birdInfo += "\n"
            
            # Print info to file
            fout.write(birdInfo)
    fout.close()


def distToClosestCenter(nextCenter, centers, size):
    minDist = size
    for c in centers:
        if distance(nextCenter, c) < minDist:
            minDist = distance(nextCenter, c)
    return minDist

def closeToBoundary(nextCenter, size):
    for i in range(2):
        if nextCenter[i] < size*0.2 or nextCenter[i] > size*0.8:
            return True
    return False

def distance(p0, p1):
    return sqrt( (p0[0]-p1[0])**2 + (p0[1]-p1[1])**2 )

if __name__ == '__main__':
    main()
