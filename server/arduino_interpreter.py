import serial
import time

CHECK_DATA_IN_SEC = 10  # Time in seconds that this script pings the Arduino for information.

DATA_FILE = 'serial_to_data.dat'  # File location
ARDUINO_PORT = '/dev/cu.usbmodem1411'  # This must match the port

# Make connection with Arduino
ardu_ser = serial.Serial(ARDUINO_PORT, 9600, timeout=10)

# Variables to keep track of time
timeNow = 0
timeNextCheck = 0

receive_new_data = False

# Returns the string of the current time.
def get_time_now_str():
    return str(time.asctime(time.localtime(time.time())))


# The actual script
# Repeat loop
while 1:
    timeNow = time.time()  # Update time

    #if not receive_new_data and timeNow > timeNextCheck:
    #    ardu_ser.write("1".encode())  # Ping the Arduino
    #    receive_new_data = True  # Turn on the lock to catch information
    #    print get_time_now_str() + ": Pinging Arduino for data, waiting information"

    # If it is time to ping the arduino for data.
    if timeNow > timeNextCheck:
        #print get_time_now_str() + ": Pinging Arduino for data..."
        data = ardu_ser.readline()  # TODO This can randomly freezes up sometimes and will not move until another serial msg is returned.

        #if "free" in data:
        #    print get_time_now_str() + "Unfreeze received"
        #    continue

        data_line = data.split("/n") # In case the data given has multiple lines combined

        for raw_data in data_line:
            # Make sure the data given isn't null.
            if raw_data is None:
                print get_time_now_str() + ": No data received from Arduino."
                continue  # Continue on the for loop of data.

            print get_time_now_str() + ": Data received! \"" + raw_data + "\""

            try:
                datafile = open(DATA_FILE, 'a')  # 'a' means append.
                datafile.write(get_time_now_str() + "@" + data + "\n")  # Attach a time.
                datafile.close()  # Remember to close it so other scripts have access to it.

                print get_time_now_str() + ": Data saved."

            except IOError:
                print get_time_now_str() + ": Error - Cannot write data to file, throwing out data."

        # Set up for next check.
        receive_new_data = False
        timeNextCheck = time.time() + CHECK_DATA_IN_SEC

    # Take a rest
    time.sleep(1)
