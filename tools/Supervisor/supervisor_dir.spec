# -*- mode: python ; coding: utf-8 -*-
from PyInstaller.utils.hooks import collect_data_files
from PyInstaller.utils.hooks import collect_all

datas = [('asset', 'asset')]
binaries = []
hiddenimports = ['serial']
datas += collect_data_files('nicegui')
#tmp_ret = collect_all('nicegui')
#datas += tmp_ret[0]; binaries += tmp_ret[1]; hiddenimports += tmp_ret[2]
datas += collect_data_files('nicegui')
hiddenimports += [
    'nicegui',
    'fastapi',
    'uvicorn',
    'socketio',
    'engineio',
]
#tmp_ret = collect_all('pymodbus')
#datas += tmp_ret[0]; binaries += tmp_ret[1]; hiddenimports += tmp_ret[2]
hiddenimports += [
    'pymodbus.client',
    'pymodbus.framer.rtu',
]

a = Analysis(
    ['supervisor.py'],
    pathex=[],
    binaries=binaries,
    datas=datas,
    hiddenimports=hiddenimports,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['notebook', 'pandas', 'scipy', 'matplotlib',
    'jupyter',
    'IPython',
    'numpy',
    'pytest',
    'tkinter',
    'PyQt5',
    'PyQt6',
    'PySide2',
    'PySide6'],
    noarchive=False,
    optimize=2,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    [],
    exclude_binaries=True,
    name='supervisor',
    debug=False,
    bootloader_ignore_signals=False,
    strip=True,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=['icon.ico'],
)

coll = COLLECT(
    exe,
    a.binaries,
    a.datas,
    strip=True,
    upx=True,
    name='supervisor',
)