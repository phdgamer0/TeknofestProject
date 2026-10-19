#!/usr/bin/env python3
import concurrent.futures
import os
import subprocess
import sys
import time

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
BUILD_DIR = os.path.join(PROJECT_ROOT, "yazdan-backend/build")
OBJ_DIR = os.path.join(BUILD_DIR, "obj")
SCENARIOS_SRC_DIR = os.path.join(PROJECT_ROOT, "yazdan-backend/tests/scenarios")
SCENARIOS_BIN_DIR = os.path.join(BUILD_DIR, "scenarios")

SRC_FILES = [
    "ardunio.cpp",
    "comms.cpp",
    "logger.cpp",
    "logic.cpp",
    "physichs.cpp",
]

def compile_object(src_name):
    src_path = os.path.join(PROJECT_ROOT, "yazdan-backend/src", src_name)
    obj_path = os.path.join(OBJ_DIR, src_name.replace(".cpp", ".o"))
    cmd = ["g++", "-c", src_path, "-o", obj_path, "-O3", "-Wall", "-Wextra", "-std=c++20"]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error compiling {src_name}:\n{res.stderr}", file=sys.stderr)
        sys.exit(1)
    return obj_path

def compile_scenario(idx):
    id_str = f"{idx:02d}"
    src_file = os.path.join(SCENARIOS_SRC_DIR, f"scenario_{id_str}.cpp")
    bin_file = os.path.join(SCENARIOS_BIN_DIR, f"scenario_{id_str}")
    obj_files = [os.path.join(OBJ_DIR, s.replace(".cpp", ".o")) for s in SRC_FILES]
    cmd = ["g++", src_file] + obj_files + ["-o", bin_file, "-O3", "-Wall", "-Wextra", "-std=c++20"]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Error compiling scenario {id_str}:\n{res.stderr}", file=sys.stderr)
        return False
    return True

def main():
    os.makedirs(BUILD_DIR, exist_ok=True)
    os.makedirs(OBJ_DIR, exist_ok=True)
    os.makedirs(SCENARIOS_BIN_DIR, exist_ok=True)

    # Clean logs
    log_path = os.path.join(PROJECT_ROOT, "yazdan-backend/logs.txt")
    with open(log_path, "w") as f:
        pass

    # Auto-discover all scenario files
    scenario_files = sorted([f for f in os.listdir(SCENARIOS_SRC_DIR) if f.startswith("scenario_") and f.endswith(".cpp")])
    scenario_indices = [int(f.replace("scenario_", "").replace(".cpp", "")) for f in scenario_files if f.replace("scenario_", "").replace(".cpp", "").isdigit()]
    total_scenarios = len(scenario_indices)

    print("=" * 80)
    print(f"   BUILDING & RUNNING {total_scenarios} INDIVIDUAL STANDALONE SCENARIO PROCESSES")
    print("=" * 80)

    # 1. Compile backend objects in parallel
    print("Compiling core backend translation units...")
    with concurrent.futures.ThreadPoolExecutor() as executor:
        list(executor.map(compile_object, SRC_FILES))

    # 2. Compile all scenarios in parallel
    print(f"Compiling {total_scenarios} standalone scenario binaries in parallel across {os.cpu_count()} CPU cores...")
    with concurrent.futures.ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        results = list(executor.map(compile_scenario, scenario_indices))
    if not all(results):
        print("Compilation failed for one or more scenarios.", file=sys.stderr)
        sys.exit(1)

    print("\n" + "=" * 80)
    print("   EXECUTING INDEPENDENT PER-PROCESS SCENARIOS (ZERO SHARED STATE)")
    print("=" * 80)

    passed_count = 0
    total_wall_ms = 0.0
    total_cpu_ms = 0.0
    max_peak_rss_mb = 0.0

    for idx in scenario_indices:
        id_str = f"{idx:02d}"
        bin_file = os.path.join(SCENARIOS_BIN_DIR, f"scenario_{id_str}")
        res = subprocess.run([bin_file], capture_output=True, text=True, cwd=PROJECT_ROOT)
        output = res.stdout.strip()
        print(output)
        if res.returncode == 0:
            passed_count += 1
            # Parse timing & rss from output
            try:
                for line in output.splitlines():
                    if "Wall:" in line and "CPU:" in line:
                        parts = line.split("|")
                        wall_str = parts[0].split("Wall:")[1].replace("ms", "").strip()
                        cpu_str = parts[1].split("CPU:")[1].replace("ms", "").strip()
                        total_wall_ms += float(wall_str)
                        total_cpu_ms += float(cpu_str)
                        if "Peak:" in parts[2]:
                            peak_str = parts[2].split("Peak:")[1].replace("MB)", "").strip()
                            peak_val = float(peak_str)
                            if peak_val > max_peak_rss_mb:
                                max_peak_rss_mb = peak_val
            except Exception:
                pass
        else:
            print(f"FAILED (Exit Code {res.returncode})", file=sys.stderr)
            if res.stderr:
                print(res.stderr, file=sys.stderr)
            sys.exit(1)

    print("=" * 80)
    print(f"   ALL {passed_count}/{total_scenarios} INDEPENDENT PROCESS SCENARIOS PASSED WITH ZERO FAILURES!")
    print(f"   TOTAL RUNTIME: Wall: {total_wall_ms:.3f} ms | CPU: {total_cpu_ms:.3f} ms | MAX PEAK RSS: {max_peak_rss_mb:.1f} MB")
    print("=" * 80)

if __name__ == "__main__":
    main()
