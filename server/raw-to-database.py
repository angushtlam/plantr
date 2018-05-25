import sqlite3
import schedule
import time

from contextlib import closing

DATA_FILE = 'serialToData.dat'  # File location

# Configuration of the sql database
DATABASE = 'plant.db'
DEBUG = True
SECRET_KEY = 'l33TsEcuR3XDdddddd!!@1@1212!'
USERNAME = 'admin'
PASSWORD = 'password'

print "Script loaded"

def curr_time():
    return str(time.asctime(time.localtime(time.time())))

def connect_db():
    return sqlite3.connect(DATABASE)

def init_db():
    with closing(connect_db()) as db:
        sqlformat = open('schema.sql', mode='r')
        db.cursor().executescript(sqlformat.read())
        db.commit()

def proc():
    print curr_time() + "Running database conversion."

    try:
        datafile = open(DATA_FILE, 'r')

        db = connect_db()  # Start connection
        print curr_time() + "Loading..."

        # For every line in the file
        for d in datafile.readlines():
            dline = d.replace("/n", "")
            data_array = d.split("@")
            if len(data_array) != 2:  # Make sure the data format is correct
                print curr_time() + ": Error - Data is in an incorrect format, throwing out data."
            else:
                timeblob = data_array[0]
                datablob = data_array[1]

                try:
                    db.execute('insert into entries (time, data) values (?, ?)', [timeblob, datablob])
                    db.commit()
                    print curr_time() + ": Data added successfully."

                except sqlite3.OperationalError:
                    print curr_time() + ": Error - Data cannot be added to database, throwing out data."

        if db is not None:  # Finally close the database connection.
            db.close() # close connection for webserver to access
            open(DATA_FILE, 'w') # clear data file

    except IOError:
        print curr_time() + ": Error - Cannot write data to file, throwing out data."

    print "Done."

schedule.every(30).seconds.do(proc)

while 1:
    schedule.run_pending()
    time.sleep(0.1)
