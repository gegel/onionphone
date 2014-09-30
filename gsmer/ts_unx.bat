#
#       UNIX installation verification file
#       for GSM Enhanced Full Rate Speech Codec
#

coder spch_unx.inp spchtst.cod
ed_iface spchtst.cod spchtst.dec
decoder spchtst.dec spchtst.out

cmp spchtst.cod spch_unx.cod
cmp spchtst.dec spch_unx.dec
cmp spchtst.out spch_unx.out
