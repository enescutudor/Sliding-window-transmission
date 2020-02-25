# Sliding-window-transmission
A data-link algorithm for transmitting a file over a link emulator.

# How to use
For testing purposes, you can use the run-experiment.sh script, which has a few options to customize the conditions in which the file will be transmitted, such as: speed, delay, corruption rate, the file to be sent, reordering rate and loss rate. If you set a higher value for the corruption rate and choose a bigger file, chances are the communication will fail.
If you intend to use the client without the script, send will require 3 arguments: the filename, speed and delay.
If you intend to manually run the link-emulator, it requires the following arguments: speed=$SPEED delay=$DELAY loss=$LOSS corrupt=$CORRUPT reorder=$REORDER.
