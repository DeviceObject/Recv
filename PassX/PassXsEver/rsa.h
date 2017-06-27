
const char szPublicKey[] = "-----BEGIN PUBLIC KEY-----\n" \
	"MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC4lJrFb4049YXcaHzwrqm/wiF6\n" \
	"ywE8+p9uMpTqGQ8ZcWhqaEdVTabvsJ3e4IXeTeWtyzp8LsoqrjrJMBX57+Qjz/1S\n" \
	"pZ5sc6eXvp5vRlQ9IW87Zv0itrBLo6W08RxcSvs3fs/qUpab6CZHSNp2Fqmv0Hea\n" \
	"Gu2pCCakTzgLwsFocQIDAQAB\n" \
	"-----END PUBLIC KEY-----\n";

const char szPrivateKey[] = "-----BEGIN RSA PRIVATE KEY-----\n" \
	"MIICXAIBAAKBgQC4lJrFb4049YXcaHzwrqm/wiF6ywE8+p9uMpTqGQ8ZcWhqaEdV\n" \
	"TabvsJ3e4IXeTeWtyzp8LsoqrjrJMBX57+Qjz/1SpZ5sc6eXvp5vRlQ9IW87Zv0i\n" \
	"trBLo6W08RxcSvs3fs/qUpab6CZHSNp2Fqmv0HeaGu2pCCakTzgLwsFocQIDAQAB\n" \
	"AoGBAIwIccMH9mQYIQ4uV1thp3bLmQrHqJDSstGvnjpb+JCc8VKI+lVFLDj8DUlh\n" \
	"nS4ievV3EU/VT5tBw2ePC50q2jDnMDgjds5gUx7fHfVmxJcqd98dgP9DpDpMmmI9\n" \
	"pPV74jt7EzE7hVPZjNMEBL75Y6Qii5sCw5GLjWrJvfrXnp4BAkEA4c4zrv3HHimi\n" \
	"y00+Z88DnAtCwwuRJXRJk+kITW7C0K/j7zXJQxiBoHs5F6uxtqYuW+V7wS7irOvb\n" \
	"iN8OfBaJuQJBANFDLyxtj1e1Z1NOx1B8F0vkrxZe58Fo1YeziTcaiGEWRq6gHuzb\n" \
	"0I3vnUi+totqSgWlSOivJDF3trzUIDfU0HkCQHXEGWh2qtasWF83lgiGCxfjN5qJ\n" \
	"+dVS5NzjeJUJ40j7syEbKDB6I4ETQx95SGI+IYeEFBsY3Nfa2tBzpRZKmuECQCeL\n" \
	"u8F4nusjU/hxXcc+/CQSfmgK3V82kYvIiPjJ4dX/ILJlfhKi48G+84lIyTSFjKGO\n" \
	"f73BQ2S5y+XZbJFAfXkCQFIOG+rXl8fmiSelifW/Bvhet0HXe5dGodsZ9S5+7kRc\n" \
	"YjhcScvhG2Yz72zezFTm8iLo/189iH3ItJ4/gLP3gnk=\n" \
	"-----END RSA PRIVATE KEY-----\n";


ULONG_PTR MyRSAEncrypt(char *szSrcDat,char **szOutEncrypted);
ULONG_PTR MyRSADecrypt(char *szEncrypted,char **szSrcDat);
ULONG_PTR MyFileEncrypt(WCHAR *wFileName);