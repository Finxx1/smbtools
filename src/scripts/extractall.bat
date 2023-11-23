@ECHO OFF

IF NOT EXIST pc (
	ECHO No "pc" folder found.
	GOTO err
)

RMDIR /S /Q pc_raw

MKDIR pc_raw

PUSHD pc

FOR /R %%I IN (*.tp) DO COPY /Y "%%I" ..\pc_raw>NUL
FOR /R %%I IN (*.itx) DO COPY /Y "%%I" ..\pc_raw>NUL

POPD

PUSHD pc_raw

FOR %%I IN (*.tp) DO (
python ../extracttp.py %%I
PUSHD "_%%~nI.tp"
python ../../extractitx.py ..\%%~nI_coords.itx 0.png"
POPD
)

POPD

:err