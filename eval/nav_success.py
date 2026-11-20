# Start from root
#   python3 eval/nav_success.py --runs 5

# Compute nav success, time to goal and dwell per lap writing results to results/nav_success.json

import argparse
from statistics import mean

from common import latest_runs, metadata, read_csv, write_result

NAV_FAILED = 4

def lap_stats(rows: list) -> dict:
 
    rows = sorted(rows, key=lambda r: float(r['stamp']))
    legs, dwells = [], []
    reached = 0

    # pair current and precious waypoints together 
    for prev, cur in zip([None] + rows[:-1], rows):

        # filter failures
        if int(cur['status']) == NAV_FAILED:
            continue

        reached += 1
        dwells.append(float(cur['dwell_s']))

        if prev is not None:
            legs.append(float(cur['stamp']) - float(prev['stamp'])
                        - float(cur['dwell_s']))
            
    return {
        'attempted': len(rows),
        'reached': reached,
        'duration_s': float(rows[-1]['stamp']) - float(rows[0]['stamp']),
        'legs': legs,
        'dwells': dwells,
    }

# formats raw numbers into clean metrics
def summarise(name: str, attempted, reached, duration, legs, dwells) -> dict:

    return {
        'run': name,
        'attempted': attempted,
        'success_rate': round(100 * reached / attempted, 1) if attempted else 0.0,
        'time_s': round(duration, 1),
        'mean_time_to_goal_s': round(mean(legs), 1) if legs else 0.0,
        'max_time_to_goal_s': round(max(legs), 1) if legs else 0.0,
        'mean_dwell_s': round(mean(dwells), 1) if dwells else 0.0,
    }

# combine runs for total summary
def compute(rows_by_run: dict) -> dict:

    stats = {name: lap_stats(rows) for name, rows in sorted(rows_by_run.items()) if rows}
    laps = [summarise(n, s['attempted'], s['reached'], s['duration_s'],
                      s['legs'], s['dwells']) for n, s in stats.items()]
    all_s = stats.values()
    total = summarise(
        'total',
        sum(s['attempted'] for s in all_s),
        sum(s['reached'] for s in all_s),
        sum(s['duration_s'] for s in all_s),
        [leg for s in all_s for leg in s['legs']],
        [d for s in all_s for d in s['dwells']],
    )
    return {'laps': laps, 'total': total}

def main():
    parser = argparse.ArgumentParser(description="Navigation reliability and mission timing")
    parser.add_argument('--runs', type=int, default=1)
    args = parser.parse_args()
    runs = latest_runs(args.runs)
    if not runs:
        raise SystemExit('No runs in eval/runs/')
    rows_by_run = {r.name: read_csv(r / 'inspection_results.csv') for r in runs}
    
    # saves to results/nav_success.json
    out = write_result('nav_success', metadata(__file__, runs), compute(rows_by_run))
    print(f'wrote {out}')


if __name__ == '__main__':
    main()
