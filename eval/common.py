# eval/ scripts helpers 

import csv
import datetime as dt
import json
import pathlib
import subprocess

import yaml

REPO_ROOT = pathlib.Path(__file__).resolve().parents[1]
RESULTS_DIR = REPO_ROOT / 'results'
RUNS_DIR = REPO_ROOT / 'eval' / 'runs'
GROUND_TRUTH = (REPO_ROOT / 'ros2_ws' / 'src' / 'inspector_bringup' / 'config'
                / 'warehouse_markers.yaml')


# return current commit so result can be traced back
def git_sha() -> str:
    try:
        out = subprocess.run(['git', 'rev-parse', '--short', 'HEAD'], cwd=REPO_ROOT,
                             capture_output=True, text=True, check=True)
        dirty = subprocess.run(['git', 'status', '--porcelain'], cwd=REPO_ROOT,
                               capture_output=True, text=True, check=True).stdout.strip()
        return out.stdout.strip() + ('-dirty' if dirty else '')
    except (OSError, subprocess.CalledProcessError):
        return 'unknown'


# results file metadata
def metadata(script: str, runs: list) -> dict:
    return {
        'script': rel(script),
        'git_sha': git_sha(),
        'generated_utc': dt.datetime.now(dt.UTC).isoformat(timespec='seconds'),
        'environment': 'simulation',
        'runs': [rel(r) for r in runs],
    }


# marker ground truth
def load_ground_truth(path: pathlib.Path = GROUND_TRUTH) -> dict:
    with open(path) as f:
        data = yaml.safe_load(f)
    return {m['id']: m for m in data['markers']}


# reads record_run.py's CSV as list of dicts
def read_csv(path: pathlib.Path) -> list:
    with open(path, newline='') as f:
        return list(csv.DictReader(f))


# write results to json and csv
def write_result(name: str, meta: dict, metrics: dict, rows: list | None = None) -> pathlib.Path:
    RESULTS_DIR.mkdir(exist_ok=True)
    out = RESULTS_DIR / f'{name}.json'
    with open(out, 'w') as f:
        json.dump({'meta': meta, 'metrics': metrics}, f, indent=2)
        f.write('\n')
    if rows:
        with open(RESULTS_DIR / f'{name}.csv', 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            writer.writeheader()
            writer.writerows(rows)
    return out


# returns n most recent eval/runs/ folders
def latest_runs(n: int = 1) -> list:
    runs = sorted((p for p in RUNS_DIR.glob('*') if p.is_dir()), key=lambda p: p.name)
    return runs[-n:]


# Make paths relative to repo root
def rel(path) -> str:
    return str(pathlib.Path(path).resolve().relative_to(REPO_ROOT))