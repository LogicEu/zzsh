#ifndef ZZ_SHELL_H
#define ZZ_SHELL_H

/* zzcmd */
int zzls(int argc, char** argv);
int zzecho(int argc, char** argv);
int zzpwd(int argc, char** argv);
int zzcp(int argc, char** argv);
int zzmv(int argc, char** argv);
int zzcd(int argc, char** argv);
int zzrm(int argc, char** argv);
int zzmkdir(int argc, char** argv);
int zztouch(int argc, char** argv);
int zzcat(int argc, char** argv);
int zzrmdir(int argc, char** argv);

/* zzsh */
int zzsh(int argc, char** argv);

#endif /* ZZ_SHELL_H */

