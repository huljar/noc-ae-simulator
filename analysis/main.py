#!/usr/bin/env python3
import os
import sqlite3 as sql
import numpy as np
import gnuplotpy as gp

# Global parameters
meshRows = 8
meshCols = 8

outputDir = 'plots/'

def dbConnect():
    # Database paths
    pathSca = '../simulations/results/General-0-sca.sqlite3'
    pathVec = '../simulations/results/General-0-vec.sqlite3'

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

def getFlitEndToEndLatency(cursorVec, sourceId, targetId):
    sourceIds = []
    targetIds = []

    idVecName = 'flitsGenerated:vector(flitId)'

    if sourceId >= 0:
        sourceIds = [sourceId]
    else:
        sourceIds = [x for x in range(meshRows * meshCols)]

    if targetId >= 0:
        targetIds = [targetId]
    else:
        targetIds = [x for x in range(meshRows * meshCols)]

    for source in sourceIds:
        for target in targetIds:
            # Build source/target module names
            sourceModName = 'Mesh2D.app[' + str(source) + '].producer'
            targetModName = 'Mesh2D.app[' + str(target) + '].consumer'

            # Get target raw address
#            targetRaw = # TODO: look up how python division/modulo rounds

#            cursorVec.execute(
#                '''select s.simtimeRaw, t.simtimeRaw
#                   from 

if __name__ == '__main__':
    # Ensure output directory exists
    if not os.path.exists(outputDir):
        os.makedirs(outputDir)

    # Connect to DBs
    (connSca, connVec) = dbConnect()

    cursorSca = connSca.cursor()
    cursorVec = connVec.cursor()

    # Plot queue lengths of routers
    plotRouterQueueLengths(cursorVec, portNum = 5)
    
    # Close DB connections
    dbClose(connSca, connVec)