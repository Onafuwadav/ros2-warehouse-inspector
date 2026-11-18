# Outcome of inspecting shelf marker from inspector_mission and collected in InspectShelves action

# Status values
uint8 OK = 0          # expected marker seen, image in focus
uint8 MISSING = 1     # expected marker not seen within the timeout
uint8 BLURRY = 2      # marker seen butbfocus score below threshold
uint8 WRONG_ID = 3    # a different marker id was seen where this one should be
uint8 NAV_FAILED = 4  # could not reach the inspection pose

std_msgs/Header header

string shelf             
int32 expected_marker_id
int32 observed_marker_id # -1 if nothing was seen
uint8 status

# Best focus score seen while at the waypoint.
float64 focus_score

# Robot pose when result decided
geometry_msgs/Pose robot_pose

# Distance between where marker was measured and where ground truth says it is
float64 marker_position_error_m

# Seconds spent at this waypoint from arrival to decision
float64 dwell_time_s
