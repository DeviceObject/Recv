

#define COOKIE_FILE		"Cookie.Dat"
#define KEY_FILE		"Key.Dat"
#define PORT_FILE		"Port.Dat"
#define VERSION_FILE	"Version.Dat"

#define VERSION_DEFAULT		"123456"
#define PORT_DEFAULT		"80"

typedef struct _SERVER_CONFIG
{
	CHAR *szVersion;
	CHAR *szCookie;
	CHAR *szKey;
	CHAR *szPort;
}SERVER_CONFIG,*PSERVER_CONFIG;


BOOLEAN ReadConfig(CHAR *szFileName,CHAR *szParam,int nLength);
BOOLEAN GetConfigString(CHAR *szFileName,CHAR **szParam);