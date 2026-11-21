# Event log in the backend and the timeline in the dashboard
uint8 INFO = 0
uint8 WARN = 1
uint8 ERROR = 2

std_msgs/Header header
uint8 severity
string source     # node name
string code       # machine readable tag
string message    # human readable text
