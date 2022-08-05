#define	__MONTH0	( __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? '0' : '0') \
					: __DATE__ [2] == 'b' ? 2 \
					: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? '0' : '0') \
					: __DATE__ [2] == 'y' ? '0' \
					: __DATE__ [2] == 'l' ? '0' \
					: __DATE__ [2] == 'g' ? '0' \
					: __DATE__ [2] == 'p' ? '0' \
					: __DATE__ [2] == 't' ? '1' \
					: __DATE__ [2] == 'v' ? '1' \
					: '1')

#define	__MONTH1	( __DATE__ [2] == 'n' ? (__DATE__ [1] == 'a' ? '1' : '6') \
					: __DATE__ [2] == 'b' ? 2 \
					: __DATE__ [2] == 'r' ? (__DATE__ [0] == 'M' ? '3' : '4') \
					: __DATE__ [2] == 'y' ? '5' \
					: __DATE__ [2] == 'l' ? '7' \
					: __DATE__ [2] == 'g' ? '8' \
					: __DATE__ [2] == 'p' ? '9' \
					: __DATE__ [2] == 't' ? '0' \
					: __DATE__ [2] == 'v' ? '1' \
					: '2')

// ISO 8601 YYYY-MM-DDThh:mm:ss
#define ISO_8601_DATE	(const char[]){ __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10], \
										'-', __MONTH0, __MONTH1, \
										'-', __DATE__[4] == ' ' ? '0' : __DATE__[4], \
											 __DATE__[5], \
										'T', __TIME__[0], __TIME__[1], \
										':', __TIME__[3], __TIME__[4], \
										':', __TIME__[6], __TIME__[7], \
										'\0' }
