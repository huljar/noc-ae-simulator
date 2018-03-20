#!/usr/bin/env python3
import sqlite3

connSca = sqlite3.connect('../simulations/results/General-0-sca.sqlite3')
connVec = sqlite3.connect('../simulations/results/General-0-vec.sqlite3')
cursorSca = connSca.cursor()
cursorVec = connVec.cursor()

cursorVec.execute('''select v.moduleName, v.vectorName, d.value
                   from vector v inner join vectorData d on v.vectorId = d.vectorId
                   where v.vectorId = 1''')

print(cursorVec.fetchone())

connSca.close()
connVec.close()
