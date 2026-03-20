'''
TODO
change output folder name to last index
'''

from pathlib import Path
import sys
import cv2  # type: ignore


NTH_FRAME = 12
OUTPUT_FOLDER_NAME = "testVidFrames"  ########## CHANGE
IMAGE_EXT = ".jpg"

# last index use case
USE_LAST_INDEX_FILE = True

# last index alternative
START_INDEX = 0


if len(sys.argv) < 2:
    print("Usage: python3 file.py <video_file_path>")
    sys.exit(1)

VIDEO_PATH = sys.argv[1]

# output path
video_file = Path(VIDEO_PATH)

if not video_file.exists():
    raise FileNotFoundError(f"Video file does not exist: {VIDEO_PATH}")

base_dir = video_file.parent

datasets_dir = base_dir / "Datasets"
output_dir = datasets_dir / OUTPUT_FOLDER_NAME
last_index_file = datasets_dir / "last_index.txt"

datasets_dir.mkdir(parents=True, exist_ok=True)
output_dir.mkdir(parents=True, exist_ok=True)


def load_last_index(txt_path: Path, fallback: int = 0) -> int:
    if txt_path.exists():
        try:
            content = txt_path.read_text(encoding="utf-8").strip()
            if content == "":
                return fallback
            return int(content)
        except Exception:
            print(f"[WARN] Could not read {txt_path.name}, using fallback {fallback}")
            return fallback
    return fallback


def save_last_index(txt_path: Path, value: int):
    txt_path.write_text(str(value), encoding="utf-8")


if USE_LAST_INDEX_FILE:
    save_idx = load_last_index(last_index_file, fallback=START_INDEX)
else:
    save_idx = START_INDEX


# load vid
cap = cv2.VideoCapture(str(video_file))

if not cap.isOpened():
    raise FileNotFoundError(f"Could not open video: {VIDEO_PATH}")

total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
fps = cap.get(cv2.CAP_PROP_FPS)

print(f"[INFO] Video: {video_file.name}")
print(f"[INFO] Total frames: {total_frames}")
print(f"[INFO] FPS (reported): {fps}")
print(f"[INFO] Saving every {NTH_FRAME}th frame")
print(f"[INFO] Output folder: {output_dir}")
print(f"[INFO] Starting saved image index: {save_idx}\n")


# fame extraction
frame_idx = 0
saved_count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break

    if frame_idx % NTH_FRAME == 0:
        out_name = f"frame_{save_idx:06d}{IMAGE_EXT}"
        out_path = output_dir / out_name

        success = cv2.imwrite(str(out_path), frame)
        if success:
            print(f"[SAVED] Video frame number {frame_idx} -> {out_name}")
            save_idx += 1
            saved_count += 1
        else:
            print(f"[WARN] Failed to save frame {frame_idx}")

    frame_idx += 1

cap.release()

# update last index
if USE_LAST_INDEX_FILE:
    save_last_index(last_index_file, save_idx)

print("\n[DONE]")
print(f"[INFO] Processed frames: {frame_idx}")
print(f"[INFO] Saved frames: {saved_count}")
print(f"[INFO] Next start index saved in {last_index_file.name}: {save_idx}")
print(f"[INFO] Output folder: {output_dir}")