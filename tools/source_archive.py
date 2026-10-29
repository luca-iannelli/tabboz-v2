"""Create the corresponding source archive without build caches or acquired tools."""
from pathlib import Path
import zipfile

root=Path(__file__).resolve().parents[1]
target=root/'dist/TabbozSimulator-2.0.0-source.zip'
target.parent.mkdir(exist_ok=True)
excluded={'.git','.tools','.superpowers','build','dist','__pycache__'}
with zipfile.ZipFile(target,'w',zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
    for path in sorted(root.rglob('*')):
        relative=path.relative_to(root)
        if any(part in excluded for part in relative.parts):continue
        if path.is_file() and not path.name.startswith('build-'):
            archive.write(path,'TabbozSimulator-2.0.0/'+relative.as_posix())
print(target)
