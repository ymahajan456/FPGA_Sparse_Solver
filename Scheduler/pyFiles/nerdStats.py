import glob, os, sys, collections
from optparse import OptionParser
from optparse import Option, OptionValueError

import matplotlib.pyplot as plt

VERSION = "0.0.2"
USAGE = """Usage: python %prog [parameters]"""
# order is import
CppOperators = ['>>=', '<<=', '||', '|=', '^=', '>>', '>=', '==', '<=', '<<',
                '/=', '//', '/*', '->', '-=', '--', '+=', '++', '*=', '*/', 
                '&=', '&&', '%=', '!=', '~', '}', '|', '{', '^', '>', '=', '<',
                ';', '/', '.', '-', ',', '+', '*', ')', '(', '&', '%', '#', '!',
                '"', '\'']

opLists = {'.cpp': CppOperators, '.h' : CppOperators}

comments = {'.cpp' : '//', '.h' : '//'}
multiLineCommentStart = {'.cpp' : '/*', '.h' : '/*'}
multiLineCommentEnd = {'.cpp' : '*/', '.h' : '*/'}

def getStatsForFile(fileName, crazyCount, fileType, ignoreComments):
    wordCount = 0
    lineCount = 0
    charCount = 0

    opList = opLists[fileType]
    mlcStart = multiLineCommentStart[fileType]
    mlcEnd = multiLineCommentEnd[fileType]
    commentMark = comments[fileType]

    opCount = {op : 0 for op in opList}
    wordFreqs = collections.Counter()

    inMLC = False

    with open(fileName, 'r') as file:
        for line in file:
            if ignoreComments:
                if inMLC:
                    if(line.find(mlcEnd) >= 0):
                        inMLC = False
                        line = line.split(mlcEnd, 1)[1]
                    else:
                        continue
                elif line.find(mlcStart) >= 0:
                    line = line.split(mlcStart, 1)[0]
                    inMLC = True
                line = line.split(commentMark, 1)[0]

            charCount += len(line)
            for op in opList:
                while line.find(op) != -1:
                    line = line.replace(op, " ", 1)
                    opCount[op] += 1
            words = line.replace('  ', ' ')
            words = line.split()
            lineWords = collections.Counter(words)

            wordFreqs = wordFreqs + lineWords

            wordCount += len(words)
            lineCount += 1

            mostFreq = dict(collections.Counter(wordFreqs).most_common(crazyCount))

    return([lineCount, wordCount, charCount, opCount, mostFreq])

def appendFileStatsToLog(fileName, fileStats, logFile, dontPlot):
    fileNameSplit = fileName.rsplit('/', 1)
    logFile.write(fileNameSplit[1] + '\n')
    logFile.write('Lines: ' + str(fileStats[0]) + '\n')
    logFile.write('Words: ' + str(fileStats[1]) + '\n')
    logFile.write('Chars: ' + str(fileStats[2]) + '\n')
    for op in fileStats[3].keys():
        logFile.write(op + ': ' + str(fileStats[3][op]) + ' ')
    logFile.write('\n')

    words = list(fileStats[4].keys())
    freqs = list(fileStats[4].values())
    for op in words:
        logFile.write(op + ': ' + str(fileStats[4][op]) + ' ')

    if not dontPlot:
        plt.xticks(rotation=45)
        plt.subplots_adjust(bottom=0.15)
        plt.bar(range(len(freqs)),freqs,tick_label=words)#, color=(0.2, 0.4, 0.6, 0.6))
        plt.savefig(fileNameSplit[0] + '/nerdStats/'  + fileNameSplit[1] + '.png')
        plt.cla()

    logFile.write('\n')

def main():
    PROG = os.path.basename(os.path.splitext(__file__)[0])
    description = """Bas nam hi kafi hai"""

    parser = OptionParser(usage = 'Usage: python %prog [PARAMETERS]',
                            version = '%s %s' %(PROG, VERSION),
                            description = description)

    parser.add_option('-p', '--path', action = 'store', dest = "path",
                        type = 'string', default = '.', help = 'Path of the source folder')

    parser.add_option('-c', '--count', action = 'store', dest = 'crazyCount',
                        type = 'int', default = 10, help = 'Number of most frequent words')
    
    parser.add_option('--noComments', action = 'store_true', dest = 'noComments',
                        default = False, help = "Ignore comments while counting")
                        
    parser.add_option('--noShowOff', action = 'store_true', dest = 'dontPlot', 
                        default = False, help = 'Don\'t generate plots')
    
    # Extract Parameters
    (paras, args) = parser.parse_args()
    
    if(len(args)):
        print("Please check the parameters\n")
        sys.exit()

    folderPath = paras.path
    crazyCount = paras.crazyCount
    noComments = paras.noComments
    dontPlot = paras.dontPlot

    if not os.path.exists(folderPath + '/nerdStats'):
        os.makedirs(folderPath + '/nerdStats')

    
    fileTypes = ['.cpp', '.h']
    logFileName = folderPath + '/nerdStats/nerdStats.txt'
    logFile = open(logFileName, 'w')
    totalWords = 0
    totalLines = 0
    totalChars = 0
    for fileType in fileTypes:
        fileNames = glob.glob(folderPath + '/*' + fileType)
        # print(fileNames)
        for file in fileNames:
            fileStats = getStatsForFile(file, crazyCount, fileType, noComments)
            appendFileStatsToLog(file, fileStats, logFile, dontPlot)
            totalLines += fileStats[0]
            totalWords += fileStats[1]
            totalChars += fileStats[2]

    logFile.write("\n\n ...................  Total ..........................\n")
    logFile.write("Total #Lines: " + str(totalLines) + '\n')
    logFile.write("Total #Words: " + str(totalWords) + '\n')
    logFile.write("Total #Chars: " + str(totalChars) + '\n')
    logFile.close()


if __name__ == "__main__":
    main()