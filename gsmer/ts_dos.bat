REM
REM     MS-DOS installation verification file
REM     for GSM Enhanced Full Rate Speech Codec
REM

coder spch_dos.inp spchtst.cod
ed_iface spchtst.cod spchtst.dec
decoder spchtst.dec spchtst.out

fc /b spchtst.cod spch_dos.cod
fc /b spchtst.dec spch_dos.dec
fc /b spchtst.out spch_dos.out
