#!/usr/bin/env python3
import sqlite3

def dbConnect():
    # Database paths
    pathSca = '../simulations/results/General-0-sca.sqlite3'
    pathVec = '../simulations/results/General-0-vec.sqlite3'

    # Database URIs with read only mode
    uriSca = 'file:' + pathSca + '?mode=ro'
    uriVec = 'file:' + pathVec + '?mode=ro'

    # Connect to DBs and get a cursor
    connSca = sqlite3.connect(uriSca, uri=True)
    connVec = sqlite3.connect(uriVec, uri=True)

    # Return connections
    return (connSca, connVec)

def dbClose(connSca, connVec):
    # Close DB connections
    connSca.close()
    connVec.close()

if __name__ == '__main__':
    # Connect to DBs
    (connSca, connVec) = dbConnect()

    cursorSca = connSca.cursor()
    cursorVec = connVec.cursor()

    # Open results file for gnuplot

    cursorVec.execute('''select v.moduleName, v.vectorName, d.value
                         from vector v inner join vectorData d on v.vectorId = d.vectorId
                         where v.vectorId = 1''')

    print(cursorVec.fetchone())

    dbClose(connSca, connVec)
