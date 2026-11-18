#include "inspector_mission/mission_logic.hpp"

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace inspector_mission
{

std::string toString(State state)
{
  switch (state) {
    case State::IDLE: return "IDLE";
    case State::NAVIGATING: return "NAVIGATING";
    case State::INSPECTING: return "INSPECTING";
    case State::DONE: return "DONE";
    case State::ABORTED: return "ABORTED";
  }
  return "UNKNOWN";
}

std::vector<Waypoint> loadWaypoints(const std::string & path)
{
  YAML::Node root;
  try {
    root = YAML::LoadFile(path);
  } catch (const YAML::Exception & e) {
    throw std::runtime_error("cannot read waypoints file '" + path + "': " + e.what());
  }
  const auto list = root["waypoints"];
  if (!list || !list.IsSequence()) {
    throw std::runtime_error("'" + path + "' has no 'waypoints:' list");
  }
  std::vector<Waypoint> out;
  for (const auto & node : list) {
    Waypoint w;
    w.shelf = node["shelf"].as<std::string>();
    w.side = node["side"] ? node["side"].as<std::string>() : "";
    w.marker_id = node["marker_id"].as<int>();
    w.x = node["x"].as<double>();
    w.y = node["y"].as<double>();
    w.yaw = node["yaw"].as<double>();
    out.push_back(w);
  }
  return out;
}

MissionLogic::MissionLogic(MissionConfig config)
: config_(config)
{
}

Command MissionLogic::next()
{
  Command command;
  if (index_ < route_.size()) {
    state_ = State::NAVIGATING;
    command.goal = route_[index_];
  } else {
    state_ = State::DONE;
    command.finished = true;
  }
  return command;
}

Command MissionLogic::start(const std::vector<Waypoint> & route, [[maybe_unused]] double now_s)
{
  route_ = route;
  index_ = 0;
  results_.clear();
  return next();
}

Command MissionLogic::advance(const Result & r)
{
  results_.push_back(r);
  ++index_;
  Command command = next();
  command.result = r;
  return command;
}

Command MissionLogic::cancel()
{
  state_ = State::ABORTED;
  Command command;
  command.finished = true;
  return command;
}

void MissionLogic::resetCounters()
{
  consecutive_hits_ = 0;
  last_wrong_id_ = -1;
  best_focus_ = 0.0;
}

Command MissionLogic::onNavSucceeded(double now_s)
{
  if (state_ != State::NAVIGATING) {return Command{};}
  state_ = State::INSPECTING;
  resetCounters();
  arrived_at_s_ = now_s;
  return Command{};
}

Result MissionLogic::makeResult(ResultStatus status, double now_s) const
{
  Result result;
  if (status != ResultStatus::NAV_FAILED) {
    result.dwell_time_s = now_s - arrived_at_s_;
    result.best_focus = best_focus_;
  }
  if (status == ResultStatus::OK || status == ResultStatus::BLURRY) {
    result.observed_marker_id = route_[index_].marker_id;
  } else if (status == ResultStatus::WRONG_ID) {
    result.observed_marker_id = last_wrong_id_;
  }

  result.status = status;
  result.waypoint = route_[index_];
  return result;
}

Command MissionLogic::onNavFailed(double now_s)
{
  if (state_ != State::NAVIGATING) {return Command{};}
  return advance(makeResult(ResultStatus::NAV_FAILED, now_s));
}

Command MissionLogic::onMarkersSeen(const std::vector<int> & ids, double now_s)
{
  if (state_ != State::INSPECTING) {return Command{};}
  const int expected_id = route_[index_].marker_id;
  const bool seen = (std::find(ids.begin(), ids.end(), expected_id) != ids.end());
  if (seen) {
    ++consecutive_hits_;
    last_wrong_id_ = -1;
  } else {
    consecutive_hits_ = 0;
    if (!ids.empty()) {last_wrong_id_ = ids.front();}
  }
  if (consecutive_hits_ >= config_.min_detections) {
    const auto status = best_focus_ >= config_.focus_threshold ?
      ResultStatus::OK : ResultStatus::BLURRY;
    return advance(makeResult(status, now_s));
  }
  return Command{};
}

Command MissionLogic::onFocusScore(double score, [[maybe_unused]] double now_s)
{
  if (state_ != State::INSPECTING) {return Command{};}
  best_focus_ = std::max(best_focus_, score);
  return Command{};
}

Command MissionLogic::onTick(double now_s)
{
  if (state_ != State::INSPECTING) {return Command{};}
  if (now_s - arrived_at_s_ > config_.inspect_timeout_s) {
    ResultStatus status;
    if (last_wrong_id_ >= 0) {
      status = ResultStatus::WRONG_ID;
    } else {
      status = ResultStatus::MISSING;
    }
    return advance(makeResult(status, now_s));
  }
  return Command{};
}

}  // namespace inspector_mission
