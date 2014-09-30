/*=============================================================================
C
C			SUNGRAPH ENDBLOCK 
C
C============================================================================*/

status = end_block(ifile_ofile_fid);
if (status < 0)
  write_error(status,"endblock ifile_ofile_fid");

status = end_block(channel_fid);
if (status < 0)
  write_error(status,"endblock channel_fid");

status = end_block(stream_error_fid);
if (status < 0)
  write_error(status,"endblock stream_error_fid");

status = end_block(lsp1_fid);
if (status < 0)
  write_error(status,"endblock lsp1_fid");

status = end_block(lsp2_fid);
if (status < 0)
  write_error(status,"endblock lsp2_fid");

status = end_block(cb_fid);
if (status < 0)
  write_error(status,"endblock cb_fid");

status = end_block(pitch_fid);
if (status < 0)
  write_error(status,"endblock pitch_fid");

status = end_block(error_fid);
if (status < 0)
  write_error(status,"endblock error_fid");

status = end_block(rc_fid);
if (status < 0)
  write_error(status,"endblock rc_fid");

status = end_block(constrain_fid);
if (status < 0)
  write_error(status,"endblock constrain_fid");
