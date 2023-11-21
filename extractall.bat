@ECHO OFF

IF NOT EXIST pc (
	ECHO No "pc" folder found.
	GOTO err
)

IF NOT EXIST pc_raw MKDIR pc_raw

DEL /Q pc_raw\*

PUSHD pc

FOR /R %%I IN (*.tp) DO COPY /Y "%%I" ..\pc_raw>NUL

POPD

PUSHD pc_raw

FOR %%I IN (*.tp) DO python ../extracttp.py %%I

POPD

:err