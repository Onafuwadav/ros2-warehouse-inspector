// mission_node.cpp feeds it events and carries out the Command it returns
// Since nothing here touches ROS missions can be carried out in microseconds

//   IDLE -- start() --> NAVIGATING -- nav ok --> INSPECTING -- decided --> NAVIGATING -- next/done
//                         |
//                         +-- nav failed --> record NAV_FAILED -- next/done
//   at any state -- cancel() --> ABORTED

#ifndef INSPECTOR_MISSION__MISSION_LOGIC_HPP_
#define INSPECTOR_MISSION__MISSION_LOGIC_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace inspector_mission
{

enum class State
{
  IDLE,
  NAVIGATING,
  INSPECTING,
  DONE,
  ABORTED,
};

std::string toString(State state);

// inspection pose
struct Waypoint
{
  std::string shelf;
  std::string side;
  int marker_id = -1;
  double x = 0.0;
  double y = 0.0;
  double yaw = 0.0;
};

std::vector<Waypoint> loadWaypoints(const std::string & path);

// InspectionResult constants
enum class ResultStatus : std::uint8_t
{
  OK = 0,
  MISSING = 1,
  BLURRY = 2,
  WRONG_ID = 3,
  NAV_FAILED = 4,
};

struct Result
{
  Waypoint waypoint;
  int observed_marker_id = -1;
  ResultStatus status = ResultStatus::MISSING;
  double best_focus = 0.0;
  double dwell_time_s = 0.0;
};

struct MissionConfig
{
  double inspect_timeout_s = 5.0;
  int min_detections = 3;
  double focus_threshold = 100.0;
};

struct Command
{
  std::optional<Result> result;   // publish InspectionResult
  std::optional<Waypoint> goal;   // send to Nav2 as NavigateToPose
  bool finished = false;          // mission over - send action result
};

class MissionLogic
{
public:
  explicit MissionLogic(MissionConfig config = MissionConfig{});


  // now_s is ROS time in s
  Command start(const std::vector<Waypoint> & route, double now_s);
  Command cancel();

  Command onNavSucceeded(double now_s);
  Command onNavFailed(double now_s);
  Command onMarkersSeen(const std::vector<int> & ids, double now_s);
  Command onFocusScore(double score, double now_s);
  Command onTick(double now_s);

  State state() const {return state_;}
  std::size_t index() const {return index_;}
  std::size_t size() const {return route_.size();}
  const std::vector<Result> & results() const {return results_;}
  std::string getCurrentShelf() const;
  const MissionConfig & config() const {return config_;}
  void setFocusThreshold(double threshold) {config_.focus_threshold = threshold;}

private:
  MissionConfig config_;
  State state_ = State::IDLE;
  std::vector<Waypoint> route_;
  std::size_t index_ = 0;
  std::vector<Result> results_;

  Command advance(const Result & r);
  Command next();
  Result makeResult(ResultStatus status, double now_s) const;
  void resetCounters();

  double arrived_at_s_ = 0.0;
  int consecutive_hits_ = 0;
  int last_wrong_id_ = -1;
  double best_focus_ = 0.0;
};

}  // namespace inspector_mission

#endif  // INSPECTOR_MISSION__MISSION_LOGIC_HPP_
