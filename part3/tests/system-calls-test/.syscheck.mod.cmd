savedcmd_syscheck.mod := printf '%s\n'   syscheck.o | awk '!x[$$0]++ { print("./"$$0) }' > syscheck.mod
