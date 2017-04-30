import sys

'''
Splits a input file created by generateBirds.py into n ranks to be used by the
simulator.c code
'''

def main():
    # USAGE:
    # python splitInputFile.py <input_file> <number of ranks>
    try:
        input_file = sys.argv[1]
        ranks = int(sys.argv[2])
    except:
        print "python splitInputFile.py <input_file> <number of ranks>"
        return

    all_birds = []
    with open(input_file) as f:
        all_birds = f.readlines()
    num_birds = len(all_birds)
    birds_per_rank = num_birds / ranks

    offset = 0
    for rank in range(ranks):
        with open("rank_" + str(rank) + ".birds", "w") as f:
            for i in range(offset, offset+birds_per_rank):
                f.write(all_birds[i])
        offset += birds_per_rank

if __name__ == '__main__':
    main()
