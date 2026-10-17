from pathlib import Path
lines=[]
for base,prefix in [(Path('package/gui'),''),(Path('package/firmware'),'firmware/')]:
    for p in sorted(base.rglob('*')):
        if p.is_file():
            relative=(prefix+p.relative_to(base).as_posix()).replace('/','\\')
            lines.append('  Delete "$INSTDIR\\'+relative+'"')
    dirs=sorted((p for p in base.rglob('*') if p.is_dir()),key=lambda p:len(p.parts),reverse=True)
    for p in dirs:
        relative=(prefix+p.relative_to(base).as_posix()).replace('/','\\')
        lines.append('  RMDir "$INSTDIR\\'+relative+'"')
    if prefix:lines.append('  RMDir "$INSTDIR\\firmware"')
lines += ['  Delete "$INSTDIR\\Desinstaller.exe"','  RMDir "$INSTDIR"']
Path('uninstall-files.nsh').write_text('\n'.join(lines)+'\n')
