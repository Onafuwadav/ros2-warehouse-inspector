import json
import pathlib
import re


def table(rows, cols):
    head = "| " + " | ".join(h for _, h in cols) + " |\n|" + "---|" * len(cols) + "\n"
    body = "".join("| " + " | ".join(str(r[k]) for k, _ in cols) + " |\n" for r in rows)
    return head + body


NAV_COLS = [
    ("run", "Lap"),
    ("attempted", "Waypoints attempted"),
    ("success_rate", "Success rate (%)"),
    ("time_s", "Lap time (s)"),
    ("mean_time_to_goal_s", "Mean time to goal (s)"),
    ("max_time_to_goal_s", "Max time to goal (s)"),
    ("mean_dwell_s", "Mean dwell time per shelf (s)"),
]

def replace(readme, name, body):
    pat = re.compile(rf"(<!-- results:{name}:start -->).*?(<!-- results:{name}:end -->)", re.DOTALL)
    new, count = pat.subn(lambda m: f"{m[1]}\n{body}{m[2]}", readme)
    if count == 0:
        raise SystemExit(f"README has no results:{name} markers")
    return new

ROOT = pathlib.Path(__file__).resolve().parents[1]   # works from any folder

readme_path = ROOT / "README.md"
readme = readme_path.read_text()

nav = json.loads((ROOT / "results/nav_success.json").read_text())

t = nav["metrics"]["total"]
total = dict(t, run="**All laps**", time_s=t["mean_lap_s"])

rows = nav["metrics"]["laps"] + [total]

body = table(rows, NAV_COLS) + f"\n_From `{nav['meta']['script']}` at commit `{nav['meta']['git_sha']}`_\n"

readme = replace(readme, "nav", body)
readme_path.write_text(readme)