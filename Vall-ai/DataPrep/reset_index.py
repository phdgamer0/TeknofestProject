from pathlib import Path

DATASETS_FOLDER = Path("Datasets")
LAST_INDEX_FILE = DATASETS_FOLDER / "last_index.txt"
RESET_VALUE = 0

DATASETS_FOLDER.mkdir(parents=True, exist_ok=True)
LAST_INDEX_FILE.write_text(str(RESET_VALUE), encoding="utf-8")

print(f"[DONE] {LAST_INDEX_FILE} has been reset to {RESET_VALUE}")