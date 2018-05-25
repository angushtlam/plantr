import serial
import time

CHECK_DATA_IN_SEC = 12  # Time in seconds that this script pings the Arduino for information.
DATA_FILE = 'serialToData.dat'  # File location
ARDUINO_PORT = '/dev/ttyACM1'  # This must match the port

# Make connection with Arduino
ardu = serial.Serial(ARDUINO_PORT, 9600)

# Lock the loop if the script is still getting info from the Arduino
lock = False

# Variables to keep track of time
timeNow = time.time()
timeNextCheck = time.time()

def curr_time():
    return str(time.asctime(time.localtime(time.time())))

# Repeat loop
while 1:
    timeNow = time.time()  # Update time

    if lock:
        print curr_time() + ": Locked, waiting for info."

        data = ardu.readline()  # TODO This randomly freezes up sometimes and will not move until another serial msg.

        if "unfrz" in data:
            print curr_time() + ": Unfreeze received."
            lock = False
            continue

        if data is None:
            print curr_time() + ": No data received from Arduino"
            lock = False;

        print curr_time() + ": Data received! >" + data
        try:
            datafile = open(DATA_FILE, 'a')
            datafile.write(curr_time() + "@ " + data) # Attach a time.
            datafile.close()  # Remember to close it so other scripts have access to it.

            timeNextCheck += CHECK_DATA_IN_SEC  # Add time.
            lock = False;
            print curr_time() + ": Data saved."

        except IOError:
            print curr_time() + ": Error - Cannot write data to file, throwing out data."

    if timeNextCheck - timeNow < 0:
        if not lock:
            ardu.write("p")  # Ping the Arduino
            lock = True  # Turn on the lock to catch information
            print curr_time() + ": Pinging Arduino for data, waiting information"

    # Take a rest
    time.sleep(1)
