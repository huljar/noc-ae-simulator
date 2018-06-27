#!/usr/bin/env python3
#
# This is the main analysis script that was used to extract statistics from the SQLite databases created
# by OMNeT++ and containing all the recorded values.
#
import os
import sys
import sqlite3 as sql
import numpy as np
import gnuplotpy as gp

# Global parameters
meshRows = 8
meshCols = 8
numCycles = 50000
endSimtimeRaw = 101000000 # Flits generated within the first 50000 cycles must have an event time value smaller than this

outputDir = 'plots/'

def dbConnect():
    # Database paths
    configName = sys.argv[1]
    pathSca = '../simulations/results/' + configName + '-0-sca.sqlite3'
    pathVec = '../simulations/results/' + configName + '-0-vec.sqlite3'

    # Database URIs with read only mode
    uriSca = 'file:' + pathSca + '?mode=ro'
    uriVec = 'file:' + pathVec + '?mode=ro'

    # Connect to DBs
    connSca = sql.connect(uriSca, uri=True)
    connVec = sql.connect(uriVec, uri=True)

    # Return connections
    return (connSca, connVec)

def dbClose(connSca, connVec):
    # Close DB connections
    connSca.close()
    connVec.close()

def getNumberOfInjectedFlits(cursorSca):
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].netStream', 'scaName': 'sendCount:count'}
    )
    flitsPerNode = cursorSca.fetchall()
    totalFlits = 0
    for node in flitsPerNode:
        totalFlits += node[0]
    return totalFlits

def getInjectionRate(cursorSca):
    return getNumberOfInjectedFlits(cursorSca) / (meshRows * meshCols * numCycles)

def getRouterQueueLengths(cursorSca):
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName and scalarValue > 0''',
            {'modName': 'Mesh2D.router[%].nodeInputQueue[_]', 'scaName': 'queueLength:max'}
    )
    lengthMax = max([row[0] for row in cursorSca.fetchall()])

    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName and scalarValue > 0''',
            {'modName': 'Mesh2D.router[%].nodeInputQueue[_]', 'scaName': 'queueLength:timeavg'}
    )
    lengthTimeavg = np.mean([row[0] for row in cursorSca.fetchall()])

    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName and scalarValue > 0''',
            {'modName': 'Mesh2D.router[%].localInputQueue', 'scaName': 'queueLength:max'}
    )
    localLengthMax = max([row[0] for row in cursorSca.fetchall()])

    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName and scalarValue > 0''',
            {'modName': 'Mesh2D.router[%].localInputQueue', 'scaName': 'queueLength:timeavg'}
    )
    localLengthTimeavg = np.mean([row[0] for row in cursorSca.fetchall()])

    return (lengthMax, lengthTimeavg, localLengthMax, localLengthTimeavg)

def plotRouterQueueLengths(cursorVec, routerNum = -1, portNum = -1):
    routerNames = []
    moduleNames = []

    if routerNum >= 0:
        routerNames = ['Mesh2D.router[' + str(routerNum) + ']']
    else:
        routerNames = ['Mesh2D.router[' + str(x) + ']' for x in range(meshRows * meshCols)]

    if portNum == 5:
        moduleNames = ['localInputQueue']
    elif portNum >= 0:
        moduleNames = ['nodeInputQueue[' + str(portNum) + ']']
    else:
        moduleNames = ['nodeInputQueue[' + str(x) + ']' for x in range(4)]
        moduleNames.append('localInputQueue')

    for router in routerNames:
        for module in moduleNames:        
            fullName = router + '.' + module

            cursorVec.execute(
                '''select d.value
                   from vector v inner join vectorData d on v.vectorId = d.vectorId
                   where v.moduleName = :name''',
                {'name': fullName}
            )

            # Fetch rows
            result = cursorVec.fetchall()

            # Get gnuplot parameters
            args = {
                'the_title': fullName, 
                'x_max': len(result),
                'y_max': 10,
                'filename': outputDir + fullName + '.png'
            }

            # Get data
            x = range(0, len(result))
            y = [x[0] for x in result]
            data = [x, y]

            # Run gnuplot
            gp.gnuplot('queuelength.gpi', args, data)

def getNumberOfGeneratedFlits(cursorVec):
    cursorVec.execute(
            '''select vectorCount
               from vector
               where vector.vectorName = :vecName''',
            {'vecName': 'flitsProduced:vector(flitId)'}
    )
    flitsPerPe = cursorVec.fetchall()
    totalFlits = 0
    for pe in flitsPerPe:
        totalFlits += pe[0]
    return totalFlits

def getNumberOfGeneratedArqs(cursorVec):
    cursorVec.execute(
            '''select vectorCount
               from vector
               where vector.vectorName = :vecName''',
            {'vecName': 'arqsGenerated:vector(flitId)'}
    )
    arqsPerNi = cursorVec.fetchall()
    totalArqs = 0
    for ni in arqsPerNi:
        totalArqs += ni[0]
    return totalArqs

def getQueueTimesEncAuthModules(cursorSca):
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].encryptQueue', 'scaName': 'timeInQueue:max'}
    )
    encQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].decryptQueue', 'scaName': 'timeInQueue:max'}
    )
    decQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].authenticateQueue', 'scaName': 'timeInQueue:max'}
    )
    authQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].verifyQueue', 'scaName': 'timeInQueue:max'}
    )
    verQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])

    encMax = max(encQueueMax, decQueueMax)
    authMax = max(authQueueMax, verQueueMax)

    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].encryptQueue', 'scaName': 'timeInQueue:avg'}
    )
    encQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].decryptQueue', 'scaName': 'timeInQueue:avg'}
    )
    decQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].authenticateQueue', 'scaName': 'timeInQueue:avg'}
    )
    authQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].verifyQueue', 'scaName': 'timeInQueue:avg'}
    )
    verQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])

    encAvg = np.mean([encQueueAvg, decQueueAvg])
    authAvg = np.mean([authQueueAvg, verQueueAvg])

    return (encMax, authMax, encAvg, authAvg)

def getQueueTimesEncAuthModulesFullGen(cursorSca):
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].encryptQueue', 'scaName': 'timeInQueue:max'}
    )
    encQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].authenticateQueue', 'scaName': 'timeInQueue:max'}
    )
    authQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].decryptVerifyQueue', 'scaName': 'timeInQueue:max'}
    )
    decVerQueueMax = max([row[0] for row in cursorSca.fetchall() if row[0] is not None])

    encMax = max(encQueueMax, decVerQueueMax)
    authMax = max(authQueueMax, decVerQueueMax)

    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].encryptQueue', 'scaName': 'timeInQueue:avg'}
    )
    encQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].authenticateQueue', 'scaName': 'timeInQueue:avg'}
    )
    authQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])
    cursorSca.execute(
            '''select scalarValue
               from scalar
               where moduleName like :modName and scalarName = :scaName''',
            {'modName': 'Mesh2D.ni[%].decryptVerifyQueue', 'scaName': 'timeInQueue:avg'}
    )
    decVerQueueAvg = np.mean([row[0] for row in cursorSca.fetchall() if row[0] is not None])

    encAvg = np.mean([encQueueAvg, decVerQueueAvg])
    authAvg = np.mean([authQueueAvg, decVerQueueAvg])

    return (encMax, authMax, encAvg, authAvg)

def getResidualErrorProbabilityAndEndToEndLatency(cursorVec): # Both in same function to save time
    # Get flits that were generated within the first recorded 50000 cycles
    cursorVec.execute(
            '''select d.value as id
               from vectorData d inner join vector v on d.vectorId = v.vectorId
               where v.vectorName = :vecName and d.simtimeRaw < :simtimeLimit''',
            {'vecName': 'flitsProduced:vector(flitId)', 'simtimeLimit': endSimtimeRaw}
    )
    flitsProduced = cursorVec.fetchall()

    # Get flits that were generated within the first recorded 50000 cycles and arrived at the destination PE at some point
    # Also get their production (aka creation) and consumption (aka arrival at PE) times
    cursorVec.execute(
            '''select s.id, s.prodTime, d.consTime
               from (select d.value as id, d.simtimeRaw as prodTime
                     from vectorData d inner join vector v on d.vectorId = v.vectorId
                     where v.vectorName = :vecNameProd and d.simtimeRaw < :simtimeLimit) s
               inner join (select d.value as id, d.simtimeRaw as consTime
                           from vectorData d inner join vector v on d.vectorId = v.vectorId
                           where v.vectorName = :vecNameCons) d
               on s.id = d.id order by s.id asc''',
            {'vecNameProd': 'flitsProduced:vector(flitId)', 'simtimeLimit': endSimtimeRaw, 'vecNameCons': 'flitsConsumed:vector(flitId)'}
    )
    flitsTransmitted = cursorVec.fetchall()

    # Calculate latencies in cycles (divide by 2000 because raw time is in picoseconds and one cycle is 2 nanoseconds)
    latencies = [(row[2] - row[1]) / 2000 for row in flitsTransmitted]
    return (len(flitsProduced), len(flitsTransmitted), np.mean(latencies))

def getRouterFlitCounts(cursorVec):
    # Get number of flits sent, forwarded, and received by the routers
    cursorVec.execute(
            '''select moduleName, sum(vectorCount)
               from vector
               where moduleName like :modName and vectorName like :vecName
               group by moduleName''',
            {'modName': 'Mesh2D.router[%].switch', 'vecName': 'flits%:vector(flitId)'}
    )
    routerList = cursorVec.fetchall()

    # Sort by router index
    routerList.sort(key=lambda r: int(r[0][14:-8]))

    # Return only the counts
    return [row[1] for row in routerList]

if __name__ == '__main__':
    # Ensure output directory exists
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    # Connect to DBs
    (connSca, connVec) = dbConnect()

    cursorSca = connSca.cursor()
    cursorVec = connVec.cursor()

    # Print total injection (= acceptance) rate
    injectionRate = getInjectionRate(cursorSca)
    print('The total network injection rate is', injectionRate)

    # Print information rate
    numSourceFlits = getNumberOfGeneratedFlits(cursorVec)
    numInjectedFlits = getNumberOfInjectedFlits(cursorSca)
    print('The information rate is', numSourceFlits / numInjectedFlits)

    # Print residual error probability and end-to-end latency
    (flitsProduced, flitsTransmitted, latencies) = getResidualErrorProbabilityAndEndToEndLatency(cursorVec)
    print('The residual error probability is ' + str((flitsProduced - flitsTransmitted) / flitsProduced) +
          ' (' + str(flitsProduced - flitsTransmitted) + ' of ' + str(flitsProduced) + ')')
    print('The average end-to-end latency is ' + str(latencies))

    # Print router heat map matrix
    #counts = getRouterFlitCounts(cursorVec)
    #print('\nRouter heat map:')
    #for i in reversed(range(8)): # print rows in reverse order for gnuplot
    #    for j in range(8):
    #        print("{0:8d}".format(counts[8*i+j]), end='')
    #    print('')
    #print('')

    # Plot queue lengths of routers
    #plotRouterQueueLengths(cursorVec, portNum = 5)

    # Print average number of ARQs per flit created at PE
    #numFlits = getNumberOfGeneratedFlits(cursorVec)
    #numArqs = getNumberOfGeneratedArqs(cursorVec)
    #print('Number of flits generated at the processing elements: ' + str(numFlits))
    #print('Number of ARQs generated: ' + str(numArqs))
    #print('... that means we have ' + str(numArqs / numFlits) + ' ARQs per source flit.\n')

    # Print max/avg queue times for enc/auth queues
    #(encMax, authMax, encAvg, authAvg) = getQueueTimesEncAuthModules(cursorSca)
    #(encMax, authMax, encAvg, authAvg) = getQueueTimesEncAuthModulesFullGen(cursorSca)
    #print('Encryption unit wait time: max ' + str(encMax) + ', avg ' + str(encAvg))
    #print('Authentication unit wait time: max ' + str(authMax) + ', avg ' + str(authAvg) + '\n')

    # Print max/avg queue lengths for router ←→ router input queues
    #(lengthMax, lengthTimeavg, localLengthMax, localLengthTimeavg) = getRouterQueueLengths(cursorSca)
    #print('Maximum/timeaverage router←→router queue length:', lengthMax, '/', lengthTimeavg)
    #print('Maximum/timeaverage local input queue length:', localLengthMax, '/', localLengthTimeavg)

    # Close DB connections
    dbClose(connSca, connVec)
