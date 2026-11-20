import json
import pathlib
import re


def table(rows, cols):
    head = "| " + " | ".join(cols) + " |\n|" + "---|" * len(cols) + "\n"
    return head + "".join("| " + " | ".join(str(r[c]) for c in cols) + " |\n" for r in rows)

def replace(readme, name, body):
    pat = re.compile(rf"(<!-- results:{name}:start -->).*?(<!-- results:{name}:end -->)", re.DOTALL)
    return pat.sub(lambda m: f"{m[1]}\n{body}{m[2]}", readme)

readme_path = pathlib.Path("README.md")
readme = readme_path.read_text()
nav = json.loads(pathlib.Path("results/nav_success.json").read_text())
readme = replace(readme, "nav", table(nav["runs"], ["lap", "reached", "attempted", "duration_s"]))
readme_path.write_text(readme)