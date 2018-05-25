import sqlite3
import schedule
import time
import traceback

DATA_FILE = 'serial_to_data.dat'  # File location
DATABASE = '/Users/angus/PycharmProjects/plantr/plant.db'

print "== Raw to Database Script =="

# These values are here in case we don't receive a full piece of data from the Arduino, we can use the last data given.
temp_blob = ""
humid_blob = ""
ph_blob = ""


def curr_time():
    return str(time.asctime(time.localtime(time.time())))


def connect_db():
    return sqlite3.connect(DATABASE, timeout=1)


def proc():
    print curr_time() + ": Running database conversion."

    try:
        datafile = open(DATA_FILE, 'r')

        db = connect_db()  # Start connection to databse.
        cursor = db.cursor()

        print curr_time() + ": Connecting to database..."

        # For every line in the temp file
        for d in datafile.readlines():
            data_array = d.split("@")  # Split the time with the rest of the data.
            if len(data_array) != 2:  # Make sure the data format is correct
                print curr_time() + ": Error - Data is in an incorrect format, throwing out data."

            else:
                time_blob = data_array[0]
                data_blob = data_array[1].replace("\n", "")

                if data_blob is "":
                    print curr_time() + ": Empty data received."
                else:
                    data_blob_split = data_blob.split(",")

                    for data_sep in data_blob_split:
                        if "temp" in data_sep:
                            temp_blob = data_sep.split(":")[1]
                        elif "ph" in data_sep:
                            ph_blob = data_sep.split(":")[1]
                        elif "humid" in data_sep:
                            humid_blob = data_sep.split(":")[1]

                    combine_data = "temp:" + temp_blob + ",ph:" + ph_blob + ",humid:" + humid_blob

                    try:
                        cursor.execute('insert into entries(time, data) values(?, ?)', (str(time_blob), str(combine_data)))
                        db.commit()
                        print curr_time() + ": Data added. \"" + str(cursor.lastrowid) + " # " + time_blob + ", " + combine_data + "\""

                    except sqlite3.OperationalError:
                        #traceback.print_stack()
                        print curr_time() + ": Error - Data cannot be added to database, throwing out data. \"" + combine_data + "\""

        if db is not None:  # Finally close the database connection.
            db.close()  # close connection for webserver to access
            open(DATA_FILE, 'w')  # clear data file

    except IOError:
        print curr_time() + ": Error - Cannot write data to file, throwing out data."

    print curr_time() + ": Done."


proc()
schedule.every(30).seconds.do(proc)


while 1:
    schedule.run_pending()
    time.sleep(0.1)
